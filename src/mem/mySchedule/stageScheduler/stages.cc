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
Stages::STAGE2_PICK
Stages::genlotto(){
        int lot = rand() % TOTALLOTO;
        return (lot <= SJFLOTTO) ? STAGE2_PICK::sjf : STAGE2_PICK::rr;
}

void
Stages::processStage2Event(){
        assert(stage1PktCount >= 1); // check that their are packet to pick and drain

        updateAllBatchStatus();
        
        if ((stage2CurState == STAGE2_STATE::pick) && (stage1PktCount > 0) ){ // their are to reason to pick if number of packet  = 0
                STAGE2_PICK pickalgo = genlotto();
                if (pickalgo == STAGE2_PICK::rr){
                        //TODO build self stat
                        //owner.algo_stats.selectedByRR++;
                        QUEUEID nextBucket = (lastRRBucket + 1) % stage1AMTBUCKET;
                        while(nextBucket != lastRRBucket){ 
                                if (stage1Data[nextBucket].canPop()){ break;}
                                nextBucket = (nextBucket + 1) % stage1AMTBUCKET; 
                        }
                        selBucket    = nextBucket;
                        lastRRBucket = nextBucket;
                }else if ( pickalgo == STAGE2_PICK::sjf ){
                        QUEUEID nextBucket = 0;
                        uint64_t nextSize = UINT64_MAX;
                        //TODO build self stat
                        //owner.algo_stats.selectedBySJF++;
                        for (QUEUEID iterBucket = 0; iterBucket < stage1AMTBUCKET; iterBucket++){
                                if (stage1Data[iterBucket].canPop()){
                                        if ( stageGSize[iterBucket] <= nextSize ){
                                                nextBucket = iterBucket;
                                                nextSize   = stageGSize[iterBucket];
                                        }
                                }
                        }
                        selBucket = nextBucket;
                }else{
                        panic("un recognize stage2 pick policy");
                }

                if (stage1Data[selBucket].canPop()){
                        stage2CurState =  STAGE2_STATE::drain;
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
        
        
        if (stage2CurState == STAGE2_STATE::drain){
                assert(stage1Data[selBucket].canPop());
                MemPacket* mpkt    = stage1Data[selBucket].front();
                uint8_t    bankNum = mpkt->bank;
                
                if (!stage3full(bankNum)){
                        mpkt = stage1Data[selBucket].pop();
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
bool
Stages::stage3full(uint8_t bankNum, uint64_t reqEntry){
        assert(bankNum < stage3Data.size());
        return (stage3Data[bankNum].size() + reqEntry) > stage3Size[bankNum];
}

std::pair<MemPacket*, bool>
Stages::chooseToDram(){

        QUEUEID nextBucket =  selBank;
        do{
                nextBucket = (nextBucket +  1) % stage3AmtBank;

                if (!stage3Data[nextBucket].empty()){
                        MemPacket*  mempkt = stage3Data[nextBucket].front();
                         
                        if (owner->mctrl->packetReady(mempkt)){
                                stage3Data[nextBucket].pop_front();
                                stage3PktCount--;
                                stageGSize[mempkt->cpuId]--;
                                selBank = nextBucket;
                                assert(stageGSize[mempkt->cpuId] >= 0);
                                assert(stage3PktCount >= 0);
                                //stat
                                //TODO build self stat
                                //owner.algo_stats.batchHit++;
                                ///////////
                                return {mempkt, true};
                        }
                }
        }
        while(nextBucket != selBank);

        //owner.algo_stats.batchMiss++;
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
TOTALLOTO(p.st2_tt_lotto),
SJFLOTTO(p.st2_sjf_lotto),
stage2TFDL(p.st2_tf_dl),
nextStage2Event( [this]{ processStage2Event(); }, name() ),
stage3AmtBank(p.st3_amt_bank),
SimObject(p)
{
        //stageG
        stageGSize.resize(stage1AMTBUCKET);
        for (auto& x : stageGSize){ x = 0; }
        //stage 1 initialization
        stage1PktCount = 0;
        for (int i = 0; i < stage1AMTBUCKET; i++){ 
                stage1Data.push_back(Bucket(
                                                p.st1_size_per_src,
                                                Bucket::pushPolicy::OLDSMS,
                                                stage1_FORMATION_THRED,
                                                this
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

}