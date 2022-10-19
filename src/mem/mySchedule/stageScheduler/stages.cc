#include "stages.hh"
#include "mem/mem_ctrl.hh"
#include "mem/mySchedule/stageScheduler/inter_stage.hh"

namespace gem5::memory{

//////////stageMetaData declaration////////////////////

//stageG

bool
Stages::empty(){
        return (stage1PktCount == 0) && (stage3PktCount == 0);
}

bool
Stages::shouldBypass( unsigned int pkt_count,
                      uint8_t      subQueueId,  
                      uint64_t     mpkc_val
                    ){

        stage_stats.MPKC_val.sample(mpkc_val);

        if ( (stage3PktCount >  stage3BypassLim) || (mpkc_val >= stage3BypassMPKC_thred)  ){
                
                if (stage3PktCount >  stage3BypassLim){
                        stage_stats.BypassExceedLim++;
                }
                
                if (mpkc_val > stage3BypassMPKC_thred){
                        stage_stats.BypassExceedMPKC++;
                }
                
                return false;
        }

        int bn = 0;
        for (auto& q : stage3Data){
                if ( (q.size() + pkt_count) > stage3Size[bn++]){
                        stage_stats.BypassStage3Lim++;
                        return false;
                }
        }

        return true;

}

//stage1

bool
Stages::canPush(QUEUEID qid, uint64_t neededEntry){
        assert(qid < stage1AMTBUCKET);
        return stage1Data[qid].canPush(neededEntry);
}

void
Stages::pushToQueues(MemPacket* mpkt){
                assert(mpkt != nullptr);
                // we do not need to update batch status dueto self called by bucket
                //we assume that cpuId is valid because caller sanitize it
                stage_stats.amountPkt++;
                QUEUEID bucketId = mpkt->cpuId;
                stageGSize[bucketId]++;
                stage1PktCount++;
                stage1Data[bucketId].push(mpkt);
                //schedule stage2 if it needed
                if (!nextStage2Event.scheduled()){
                        schedule(nextStage2Event, curTick());
                }
}

void
Stages::updateAllBatchStatus(){
        for (auto& bucket : stage1Data){
                bucket.updateBatchStatus();
        }
}
//stage2
enums::SMS_STAGE2_PICK
Stages::genlotto(){

        uint32_t lot = rand() % TOTALLOTTO;
        for (enums::SMS_STAGE2_PICK pol  = enums::SMS_STAGE2_PICK::SMS_rr; 
                                    pol != enums::SMS_STAGE2_PICK::Num_SMS_STAGE2_PICK; 
                                    pol  = enums::SMS_STAGE2_PICK(pol+1)){
                if (lot < LOTTO[pol]){
                        return pol;
                }
                lot -= LOTTO[pol];
        }
        panic("pick policy eror");
}

void
Stages::processStage2Event(){
        assert(stage1PktCount >= 1); // check that their are packet to pick and drain

        updateAllBatchStatus();
        //select bucket(stage1) to inject to stage3
        if ((stage2CurState == STAGE2_STATE::pick) && (stage1PktCount > 0) ){ // their are to reason to pick if number of packet  = 0
                enums::SMS_STAGE2_PICK pickalgo = genlotto();
                if (pickalgo == enums::SMS_STAGE2_PICK::SMS_rr){
                        stage_stats.selectedByRR++;
                        QUEUEID nextBucket = (lastRRBucket + 1) % stage1AMTBUCKET;
                        while(nextBucket != lastRRBucket){ 
                                if (stage1Data[nextBucket].canPop()){ break;}
                                nextBucket = (nextBucket + 1) % stage1AMTBUCKET; 
                        }
                        selBucket    = nextBucket;
                        lastRRBucket = nextBucket;
                }else if ( pickalgo == enums::SMS_STAGE2_PICK::SMS_sjf ){
                        QUEUEID nextBucket = 0;
                        uint64_t nextSize = UINT64_MAX;
                        stage_stats.selectedBySJF++;
                        for (QUEUEID iterBucket = 0; iterBucket < stage1AMTBUCKET; iterBucket++){
                                if (stage1Data[iterBucket].canPop()){
                                        if ( stageGSize[iterBucket] <= nextSize ){
                                                nextBucket = iterBucket;
                                                nextSize   = stageGSize[iterBucket];
                                        }
                                }
                        }
                        selBucket = nextBucket;
                }else if ( pickalgo == enums::SMS_STAGE2_PICK::SMS_s1mf){
                        stage_stats.selectedByS1MF++;
                        QUEUEID nextBucket = 0;
                        uint64_t nextSize  = 0;
                        for (QUEUEID iterBucket = 0; iterBucket < stage1AMTBUCKET; iterBucket++){
                                if (stage1Data[iterBucket].canPop()){
                                        if ( stage1Data[iterBucket].size() > nextSize ){
                                                nextBucket = iterBucket;
                                                nextSize   = stage1Data[iterBucket].size();
                                        }
                                }
                        }
                        selBucket = nextBucket;
                }else{
                        panic("un recognize stage2 pick policy");
                }

                if (stage1Data[selBucket].canPop()){
                        stage2CurState =  STAGE2_STATE::drain;
                        stage_stats.batchedSize.sample(stage1Data[selBucket].get_batchSize(
                                                                                                stage1Data[selBucket].front()->batchId
                                                                                           ));
                        DPRINTF_SMS(SMS, "---------------------------------------------------\n");
                        DPRINTF_SMS(SMS, "////////////////////////picking up\n");
                        DPRINTF_SMS(SMS, "stage1 section\n");
                        //printStage(stage1Meta);
                        DPRINTF_SMS(SMS, "stage3 section\n");
                        //printStage(stage3Data);
                        //TODO build self stat
                        //BATCHID preLogBid = stage1Meta[selBucket].front()->batchId;
                        //owner.algo_stats.batchedSize.sample(stage1Meta[selBucket].batchMap[preLogBid].batchSize);
                }else{
                        stage2CurState =  STAGE2_STATE::pick;
                }

        }
        
        //push to stage 3 section
        if (stage2CurState == STAGE2_STATE::drain){
                assert(stage1Data[selBucket].canPop());
                // front did not mean retrieve head of queue only but depended policy
                MemPacket* mpkt    = stage1Data[selBucket].front();
                uint8_t    bankNum = mpkt->bankId;
                
                if (!stage3full(bankNum)){
                        mpkt = stage1Data[selBucket].pop();
                        assert(mpkt != nullptr);
                        stage3Data[bankNum].push_back(mpkt);
                        //update status
                        stage1PktCount--;
                        stage3PktCount++;

                        if ( !stage1Data[selBucket].empty() && (mpkt->batchId == stage1Data[selBucket].front()->batchId) ){
                                stage2CurState = STAGE2_STATE::drain;
                        }else{
                                stage2CurState = STAGE2_STATE::pick;

                                DPRINTF_SMS(SMS, "//////////////////finishDrainSection\n");
                                DPRINTF_SMS(SMS, "stage1 section\n");
                                //printStage(stage1Data);
                                DPRINTF_SMS(SMS, "stage3 section\n");
                                //printStage(stage3Data);
                                DPRINTF_SMS(SMS, "------------------------------------------------\n");
                        }
                        

                        if ( !(owner->mctrl)->requestEventScheduled() ){
                                owner->mctrl->restartScheduler(curTick());
                        }

                }

        }

        //start stage2 again if it needed
        if ( (stage1PktCount != 0) && (!nextStage2Event.scheduled())){
                schedule(nextStage2Event, curTick() + stage2TFDL);
        }
        
};

//stage3

void
Stages::pushToQueuesBypass(MemPacket* mpkt){
        assert(mpkt);
        assert(stage3Data[mpkt->bankId].size() < stage3Size[mpkt->bank]);
        stage_stats.amountBypass++;
        stage3PktCount++;
        stageGSize[mpkt->cpuId]++;
        stage3Data[mpkt->bankId].push_back(mpkt);
        //start process next request event
        if ( !(owner->mctrl)->requestEventScheduled() ){
                                owner->mctrl->restartScheduler(curTick());
        }
}

bool
Stages::stage3full(uint8_t bankNum, uint64_t reqEntry){
        assert(bankNum < stage3Data.size());
        return (stage3Data[bankNum].size() + reqEntry) > stage3Size[bankNum];
}

std::pair<MemPacket*, bool>
Stages::chooseToDram(){

        int dbg_amt = 0;
        for (auto& stg3_q : stage3Data){
                if (stg3_q.size()){
                        dbg_amt++;
                }
        }

        QUEUEID nextBank =  selBank;
        do{
                nextBank = (nextBank +  1) % stage3AmtBank;

                if (!stage3Data[nextBank].empty()){
                        MemPacket*  mempkt = stage3Data[nextBank].front();
                        assert(mempkt != nullptr);
                        if (owner->mctrl->packetReady(mempkt)){
                                stage3Data[nextBank].pop_front();
                                stage3PktCount--;
                                stageGSize[mempkt->cpuId]--;
                                selBank = nextBank;
                                assert(stageGSize[mempkt->cpuId] >= 0);
                                assert(stage3PktCount >= 0);
                                //stat
                                //TODO build self stat
                                //owner.algo_stats.batchHit++;
                                ///////////
                                stage_stats.dramChooseHit++;
                                stage_stats.selStage3_hitDBG.sample(dbg_amt);
                                return {mempkt, true};
                        }
                }
        }
        while(nextBank != selBank);

        //owner.algo_stats.batchMiss++;
        stage_stats.dramChoosemiss++;
        stage_stats.selStage3_missDBG.sample(dbg_amt);
        return {nullptr, false};

}

void
Stages::printStage(std::vector<MemPacketQueue>& stageRef){
        // DPRINTF_SMS(SMS,"--------------\n");
        // for (int i = 0; i < stageRef.size(); i++){
        //         int count = 0;
        //         std::string pre_print = "";
        //         for (int j = 0; j < stageRef[i].size(); j++){
        //                 count++;
        //                 if ( ( (j+1) < stageRef[i].size() ) &&  
        //                      (stageRef[i].at(j+1)->batchId == stageRef[i].at(j)->batchId) )
        //                 {
        //                         //pass
        //                 }else{
        //                         //DPRINTF_SMS(SMS,"bid:%d:%d", stageRef[i].at(j)->batchId,count);
        //                         pre_print += "bid: " + std::to_string(stageRef[i].at(j)->batchId) + 
        //                                      " c: " + std::to_string(count) + " ";
        //                         count = 0;
        //                 }
        //         }
        // DPRINTF_SMS(SMS,"q:%d  --->%s\n", i, pre_print);
        // }
}


void
Stages::printStage(std::vector<Bucket>& stageRef){
        // DPRINTF_SMS(SMS,"--------------\n");
        // for (int i = 0; i < stageRef.size(); i++){
        //         int count = 0;
        //         std::string pre_print = "";
        //         for (int j = 0; j < stageRef[i].dayta.size(); j++){
        //                 count++;
        //                 if ( ( (j+1) < stageRef[i].dayta.size() ) &&  
        //                      (stageRef[i].dayta.at(j+1)->batchId == stageRef[i].dayta.at(j)->batchId) )
        //                 {
        //                         //pass
        //                 }else{
        //                         //DPRINTF_SMS(SMS,"bid:%d:%d", stageRef[i].dayta.at(j)->batchId,count);
        //                         pre_print += "bid: " + std::to_string(stageRef[i].dayta.at(j)->batchId) + 
        //                                      " c: " + std::to_string(count) + " ";
        //                         count = 0;
        //                 }
        
        //         }
        //         DPRINTF_SMS(SMS,"q:%d  --->%s\n", i, pre_print);
        // }
}

//constructor
Stages::Stages(
        const StagesParams& p
              ):
//stage1
stage1AMTBUCKET(p.st1_amt_src),
stage1_FORMATION_THRED(p.st1_formation_thred),
//stage2
TOTALLOTTO(p.st2_tt_lotto),
LOTTO(p.st2_vec_lotto),
stage2TFDL(p.st2_tf_dl),
nextStage2Event( [this]{ processStage2Event(); }, name() ),
//stage3
stage3BypassMPKC_thred(p.st3_BypassMPKC_thred),
stage3BypassLim(p.st3_BypassLim),
stage3AmtBank(p.st3_amt_bank),
//upper stage
SimObject(p),
stage_stats(*this)
{
        //stageG
        stageGSize.resize(stage1AMTBUCKET);
        for (auto& x : stageGSize){ x = 0; }
        //stage 1 initialization
        stage1PktCount = 0;
        for (int i = 0; i < stage1AMTBUCKET; i++){ 
                stage1Data.push_back(Bucket(
                                                p.st1_size_per_src,
                                                p.st1_pushPol,
                                                p.st1_popPol ,
                                                stage1_FORMATION_THRED,
                                                this,
                                                (QUEUEID)i
                                           )       
                                    );
        }
        //stage 2 initialization
        lastRRBucket   = 0;
        selBucket      = 0;
        stage2CurState = STAGE2_STATE::pick;
        //stage 3 initialization
        selBank        = 0;
        stage3PktCount = 0;
        stage3Size.resize(stage3AmtBank);
        for (auto& x : stage3Size){ x = p.st3_size_per_bank; }
        stage3Data.resize(stage3AmtBank);
}


//////////////////////////////////////////////////////////////////////////////////////////////
        Stages::Stages_Stats::Stages_Stats(Stages& ITQ):
        statistics::Group(& ITQ),
        stages_owner(ITQ),
        ADD_STAT(selectedByRR,
                 statistics::units::Count::get(),
                 "amount of times that scheduler select by using rubin"
                ),
        ADD_STAT(
                selectedBySJF,
                statistics::units::Count::get(),
                "amount of times that scheduler select by using short job first"
                ),
        ADD_STAT(
                selectedByS1MF,
                statistics::units::Count::get(),
                "amount of times that scheduler select by using stage1 max first"
                ),
        ADD_STAT(
                dramChooseHit,
                statistics::units::Count::get(),
                "number of times that dram is ready in all selected batch when scheduler need to select"
                ),
        ADD_STAT(
                dramChoosemiss,
                statistics::units::Count::get(),
                "number of time that dram is not ready in all selected batch when scheduler need to select"
                ),
        ADD_STAT(amountPkt,
                 statistics::units::Count::get(),
                 "amount of packet that get throught  stage1"),
        ADD_STAT(amountBypass,
                 statistics::units::Count::get(),
                 "amount of packet that get bypass to stage3"),
        ADD_STAT(serveByWriteQ,
                 statistics::units::Count::get(),
                 "amount of memory packet that is serve by write queue"),        
        ADD_STAT(batchExpire,
                 statistics::units::Count::get(),
                 "number of expire batch"),
        ADD_STAT(exceedStage1,
                 statistics::units::Count::get(),
                 "for write queue, count if the req over stage1 exceed rate"),
        ADD_STAT(exploitBatch,
                 statistics::units::Count::get(),
                 "amount of packet that can tie within the last of fifo stage"),
        ADD_STAT(startNewBatch,
                 statistics::units::Count::get(),
                 "amount of packet that must assign new batch number"),
        ADD_STAT(batchedSize,
                statistics::units::Count::get(),
                "batch size in stage1 before picked to stage3"),
        ADD_STAT(selStage3_hitDBG,
                statistics::units::Count::get(),
                "incase stage3 selection is hit so sample stage3"),
        ADD_STAT(selStage3_missDBG,
                statistics::units::Count::get(),
                "incase stage3 selection is hit so sample stage3"),
        ADD_STAT(MPKC_val,
                statistics::units::Count::get(),
                "MPKC value")
        // ADD_STAT(diffPushTime,
        //         statistics::units::Count::get(),
        //         "differrent time between 2 packet arrive")
        //,
        // ADD_STAT(
        //         maxSizeWriteQueue,
        //         statistics::units::Count::get(),
        //         "max size of mempacket that fill in each write queue"),
        // ADD_STAT(
        //         maxSizeReadQueue,
        //         statistics::units::Count::get(),
        //         "max size of mempacket that fill in each read queue")
        {
                using namespace statistics;

                // batchExpire  .init(stages_owner.stage1AMTBUCKET);
                // exceedStage1 .init(stages_owner.stage1AMTBUCKET);
                // exploitBatch .init(stages_owner.stage1AMTBUCKET);
                // startNewBatch.init(stages_owner.stage1AMTBUCKET);
                batchedSize.init(1024).flags(nozero);
                MPKC_val.init(50);
                //diffPushTime
                //.init(1);
                // TODO for now we deactivate it

        }

        void 
        Stages::Stages_Stats::regStats(){
                using namespace statistics;
                // exploitBatch .init    (STAGEQueueOwner.amtSrc);
                // startNewBatch.init    (STAGEQueueOwner.amtSrc);
                // maxSizeWriteQueue.init(STAGEQueueOwner.maxWriteStageSize+10).flags(nozero);
                // maxSizeReadQueue .init(STAGEQueueOwner.maxReadStageSize +10).flags(nozero);
                batchExpire  .init(stages_owner.stage1AMTBUCKET);
                exceedStage1 .init(stages_owner.stage1AMTBUCKET);
                exploitBatch .init(stages_owner.stage1AMTBUCKET);
                startNewBatch.init(stages_owner.stage1AMTBUCKET);
                // debugger
                selStage3_hitDBG.init(stages_owner.stage3AmtBank);
                selStage3_missDBG.init(stages_owner.stage3AmtBank);


}

}