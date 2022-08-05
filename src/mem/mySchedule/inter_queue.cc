
#include "inter_queue.hh"
#include "mem/mem_interface.hh"

//#include "debug/interQ.hh"

namespace gem5
{


namespace memory{



///interQueue
InterQueue::InterQueue(const InterQueueParams& p) : //mctrl(p.mc),

SimObject(p){

}

///stage scheduling////////////////////////////////////////

bool
STAGE_SCHED_Queue::readQueueFull(unsigned int pkt_count, uint8_t subQueueId){
        
        assert( (readSide.stage1AMTBUCKET > subQueueId) && (subQueueId >= 0) );
        
        return !readSide.stage1Meta[subQueueId].canPush(pkt_count);



}

bool
STAGE_SCHED_Queue::writeQueueFull(unsigned int pkt_count, uint8_t subQueueId){
        
        assert( (writeSide.stage1AMTBUCKET > subQueueId) && (subQueueId >= 0) );
        
        return !writeSide.stage1Meta[subQueueId].canPush(pkt_count);


}

void
STAGE_SCHED_Queue::pushToQueues(MemPacket* mpkt, bool isRead){
        // must copy this to mempacket because main pkt may change
        mpkt->cpuId          = mpkt->pkt->req->cpuId >= 0 ? mpkt->pkt->req->cpuId : 0;
        mpkt->queueAddedTime = curTick();
        mpkt->fromNetwork    = mpkt->pkt->req->fromNetwork;
        ////////////////////
        if (isRead){
                readSide.pushToQueues( mpkt );
                if (!readSide.nextStage2Event.scheduled()){
                        schedule(readSide.nextStage2Event, curTick());
                }
        }else{
                writeSide.pushToQueues( mpkt );
                if (!writeSide.nextStage2Event.scheduled()){
                        schedule(writeSide.nextStage2Event, curTick());
                }
        }
}

std::pair<MemPacket*, bool>
STAGE_SCHED_Queue::chooseToDram(bool is_read){
        return is_read ? readSide.chooseToDram() : writeSide.chooseToDram();
}

bool
STAGE_SCHED_Queue::serveByWriteQueue(Addr addr, unsigned size){
        return writeSide.serveByWriteQueue(addr, size);
}


std::vector<MemPacketQueue*>
STAGE_SCHED_Queue::getQueueToSelect(bool read){
        std::vector<MemPacketQueue*> ret;
        for (MemPacketQueue& mq : (read ? readSide.stage3Data : writeSide.stage3Data) ){
                ret.push_back(&mq);
        }
        return ret;
}
//////////stageMetaData declaration////////////////////

//stage1
void
STAGE_SCHED_Queue::stageMetaData::pushToQueues(MemPacket* mpkt){
                assert(mpkt != nullptr);

                QUEUEID bucketId = mpkt->cpuId;
                stageGSize[bucketId]++;
                stage1PktCount++;
                stage1Meta[bucketId].push(mpkt);
                
                
}

void
STAGE_SCHED_Queue::stageMetaData::updateAllBatchStatus(){
        for (auto& bucket : stage1Meta){
                bucket.updateBatchStatus();
        }
}
//stage2
STAGE_SCHED_Queue::stageMetaData::STAGE2_PICK
STAGE_SCHED_Queue::stageMetaData::genlotto(){
        int lot = rand() % TOTALLOTO;
        return (lot <= SJFLOTTO) ? STAGE2_PICK::sjf : STAGE2_PICK::rr;
}

void
STAGE_SCHED_Queue::stageMetaData::processStage2Event(){
        assert(stage1PktCount >= 1); // check that their are packet to pick and drain

        updateAllBatchStatus();
        
        if ((stage2CurState == STAGE2_STATE::pick) && (stage1PktCount > 0) ){ // their are to reason to pick if number of packet  = 0
                STAGE2_PICK pickalgo = genlotto();
                if (pickalgo == STAGE2_PICK::rr){
                        owner.algo_stats.selectedByRR++;
                        QUEUEID nextBucket = (lastRRBucket + 1) % stage1AMTBUCKET;
                        while(nextBucket != lastRRBucket){ 
                                if (stage1Meta[nextBucket].canPop()){ break;}
                                nextBucket = (nextBucket + 1) % stage1AMTBUCKET; 
                        }
                        selBucket    = nextBucket;
                        lastRRBucket = nextBucket;
                }else if ( pickalgo == STAGE2_PICK::sjf ){
                        QUEUEID nextBucket;
                        uint64_t nextSize = UINT64_MAX;
                        owner.algo_stats.selectedBySJF++;
                        for (QUEUEID iterBucket = 0; iterBucket < stage1AMTBUCKET; iterBucket++){
                                if (stage1Meta[iterBucket].canPop()){
                                        // MemPacket* frontMpkt = stage1Data[iterBucket].front();
                                        // uint64_t iterBatchSize = stage1Meta[iterBucket].bs[frontMpkt->batchId];
                                        // if (iterBatchSize < nextSize){
                                        //         nextBucket = iterBucket;
                                        //         nextSize   = iterBatchSize;
                                        // }

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

                

                stage2CurState = stage1Meta[selBucket].canPop() ? STAGE2_STATE::drain : STAGE2_STATE::pick;

        }
        
        
        if (stage2CurState == STAGE2_STATE::drain){
                assert(stage1Meta[selBucket].canPop());
                MemPacket* mpkt = stage1Meta[selBucket].front();
                uint8_t bankNum = mpkt->bank;
                
                if (!stage3full(bankNum)){
                        mpkt = stage1Meta[selBucket].pop();
                        stage3Data[bankNum].push_back(mpkt);
                        //update status
                        stage1PktCount--;
                        stage3PktCount++;
                        stage2CurState = 
                                (       (!stage1Meta[selBucket].empty()) && 
                                        (mpkt->batchId == stage1Meta[selBucket].front()->batchId)
                                ) 
                                ? STAGE2_STATE::drain 
                                : STAGE2_STATE::pick; 

                        if ( !(owner.mctrl)->requestEventScheduled() ){
                                owner.mctrl->restartScheduler(curTick());
                        }

                }

        }

        //start stage2 again if it needed
        if ( (stage1PktCount != 0) && (!nextStage2Event.scheduled())){
                owner.schedule(nextStage2Event, curTick() + stage2TFDL);
        }
        //todo start stage3 if from main ctrl



};

//stage3
bool
STAGE_SCHED_Queue::stageMetaData::stage3full(uint8_t bankNum, uint64_t reqEntry){
        assert(bankNum < stage3Data.size());
        return (stage3Data[bankNum].size() + reqEntry) > stage3Size[bankNum];
}

std::pair<MemPacket*, bool>
STAGE_SCHED_Queue::stageMetaData::chooseToDram(){

        QUEUEID nextBucket =  selBank;
        do{
                nextBucket = (nextBucket +  1) % stage3AmtBank;

                if (!stage3Data[nextBucket].empty()){
                        MemPacket*  mempkt = stage3Data[nextBucket].front();
                         
                        if (owner.mctrl->packetReady(mempkt)){
                                stage3Data[nextBucket].pop_front();
                                stage3PktCount--;
                                stageGSize[mempkt->cpuId]--;
                                assert(stageGSize[mempkt->cpuId] >= 0);
                                assert(stage3PktCount >= 0);
                                //stat
                                owner.algo_stats.batchHit++;
                                ///////////
                                return {mempkt, true};
                        }
                }
        }
        while(nextBucket != selBank);

        owner.algo_stats.batchMiss++;
        return {nullptr, false};

}

//constructor
STAGE_SCHED_Queue::stageMetaData::stageMetaData(
                        STAGE_SCHED_Queue& own,
                        uint32_t  tt_lotto,
                        uint32_t  sjf_lotto,
                        uint64_t  stage1_sizePerSrc,
                        uint8_t   stage1_amtSrc,
                        Tick      stage1_FORMATION_THRED,
                        Tick      stage2_TFDELAY,
                        uint64_t  stage3_sizePerBank,
                        uint8_t   stage3_amtBank

):
owner(own),
TOTALLOTO(tt_lotto),
SJFLOTTO(sjf_lotto),
stage1AMTBUCKET(stage1_amtSrc),
stage1_FORMATION_THRED(stage1_FORMATION_THRED),
stage2TFDL(stage2_TFDELAY),
stage3AmtBank(stage3_amtBank),
nextStage2Event( [this]{ processStage2Event(); }, "stage2 event" )
{
        //stageG
        stageGSize.resize(stage1AMTBUCKET);
        for (auto& x : stageGSize){ x = 0; }
        //stage 1 initialization
        stage1PktCount = 0;
        stage1Meta.resize(stage1AMTBUCKET);
        for (auto& x : stage1Meta){ 
                x.FORMATION_THRED = stage1_FORMATION_THRED; 
                x.maxSize         = stage1_sizePerSrc;
                x.owner           = this;
        }
        //stage 2 initialization
        lastRRBucket = 0;
        selBucket    = 0;
        stage2CurState = STAGE2_STATE::pick;
        //stage 3 initialization
        selBank    = 0;
        stage3PktCount = 0;
        stage3Size.resize(stage3AmtBank);
        for (auto& x : stage3Size){ x = stage3_sizePerBank; }
        stage3Data.resize(stage3AmtBank);
}

//////////writeStageMetaData declaration////////////////////
bool 
STAGE_SCHED_Queue::writeStageMetaData::serveByWriteQueue(Addr addr, 
                                                        unsigned size,
                                                        MemPacketQueue& srcToFind){
        for(MemPacket* mpkt: srcToFind){
                if ( (mpkt->addr <= addr) + 
                     ((addr +size) <= (mpkt->addr + mpkt->size)) 
                   )
                {
                        return true;
                }
        }

}


bool 
STAGE_SCHED_Queue::writeStageMetaData::serveByWriteQueue(Addr addr, unsigned size){
        for (auto& stage1_buck : stage1Meta){
                if (serveByWriteQueue(addr, size, stage1_buck.dayta)){
                        owner.algo_stats.serveByWriteQ++;
                        return true;
                }
        }
        for (auto& stage3_q : stage3Data){
                if (serveByWriteQueue(addr, size, stage3_q)){
                        owner.algo_stats.serveByWriteQ++;
                        return true;
                }
        }
        return false;
}

STAGE_SCHED_Queue::writeStageMetaData::writeStageMetaData( STAGE_SCHED_Queue& own,
                                                           uint32_t  tt_lotto,
                                                           uint32_t  sjf_lotto,
                                                           uint64_t  stage1_sizePerSrc,
                                                           uint8_t   amtSrc,
                                                           Tick      stage1_FORMATION_THRED,
                                                           Tick      stage2_TFDELAY,
                                                           uint64_t  stage3_sizePerBank,
                                                           uint8_t   amtBank
                        ): stageMetaData(       own,
                                                tt_lotto,
                                                sjf_lotto,
                                                stage1_sizePerSrc,
                                                amtSrc,
                                                stage1_FORMATION_THRED,
                                                stage2_TFDELAY,
                                                stage3_sizePerBank,
                                                 amtBank            
                                        )
                        
{}

///////////// BucketMeta////////////////////////////////////////

bool
STAGE_SCHED_Queue::stageMetaData::BucketMeta::canPush(uint64_t  neededEntry){
        return (curSize + neededEntry) <= maxSize;
}

bool 
STAGE_SCHED_Queue::stageMetaData::BucketMeta::canPop(){
        return (curSize > 0) ? batchMap[front()->batchId].isBatchReady : false;
}

void
STAGE_SCHED_Queue::stageMetaData::BucketMeta::push(MemPacket* mpkt){
        assert( !( (lastBatchId == -1) ^ (curSize == 0) ));
        updateBatchStatus();
        if (lastBatchId != -1){
                if (batchMap[lastBatchId].isBatchReady ||
                    (!isRowHit(mpkt, dayta.back()))
                   )
                {
                        // update previous batch-> not row hit or batch redy due to thredshold
                        batchMap[lastBatchId].isBatchReady   = true; // case come from row !hit 
                        // create new batch and update global variable
                        lastBatchId = lastBatchId+1;
                        batchMap[lastBatchId].firstAddedTime = curTick();
                        batchMap[lastBatchId].isBatchReady   = false;
                        batchMap[lastBatchId].batchSize      = 1;
                        //update stat
                        owner->owner.algo_stats.startNewBatch[mpkt->cpuId]++;
                }else{ // batch not ready and row hit
                        assert(batchMap[lastBatchId].isBatchReady == false);
                        batchMap[lastBatchId].batchSize++;
                        //update stat
                        owner->owner.algo_stats.exploitBatch[mpkt->cpuId]++;

                }
        }else{
                        // create new batch and update global variable
                        lastBatchId = lastBatchId+1;
                        batchMap[lastBatchId].firstAddedTime = curTick();
                        batchMap[lastBatchId].isBatchReady   = false;
                        batchMap[lastBatchId].batchSize      = 1;
                        //update stat
                        owner->owner.algo_stats.startNewBatch[mpkt->cpuId]++;
        }
        // assign to packet and put packet to the bucket
        mpkt->batchId = lastBatchId;
        dayta.push_back(mpkt);
        curSize++;
        assert(curSize == dayta.size());

}

MemPacket*
STAGE_SCHED_Queue::stageMetaData::BucketMeta::pop(){
        assert( ( !dayta.empty() ) && (curSize > 0) && (lastBatchId != -1) );
        MemPacket* mpkt = dayta.front();
        dayta.pop_front();

        assert(batchMap[mpkt->batchId].batchSize > 0);
        panic_if( !batchMap[mpkt->batchId].isBatchReady, "stage2 intend to pop but batch is not ready" );

        // update cur size
        curSize--;
        // update batch map and last batch id
        if (curSize == 0){
                clear();
        }else if (batchMap[mpkt->batchId].batchSize == 1){
                // update batch map and global variable
                batchMap.erase( batchMap.find(mpkt->batchId));
        }else{
                batchMap[mpkt->batchId].batchSize--;
        }
        return mpkt;

}

MemPacket*
STAGE_SCHED_Queue::stageMetaData::BucketMeta::front(){
        assert(!dayta.empty());
        return dayta.front();
}


void 
STAGE_SCHED_Queue::stageMetaData::BucketMeta::updateBatchStatus(){
        //this is used to make batch ready when last batch reach the thredshold
        if (curSize == 0){
                assert((lastBatchId == -1) && (dayta.size() == 0));
        }else{
                assert(lastBatchId >= 0);
                if ( (curTick() - batchMap[lastBatchId].firstAddedTime) >= FORMATION_THRED){
                        batchMap[lastBatchId].isBatchReady = true;
                }
        }
}

bool 
STAGE_SCHED_Queue::stageMetaData::BucketMeta::isRowHit(MemPacket* a, MemPacket* b){
        return (a->rank == b->rank) && (a->bank == b->bank) && (a->row == b->row);
}

void 
STAGE_SCHED_Queue::stageMetaData::BucketMeta::clear(){
        lastBatchId = -1;
        curSize     =  0;
        dayta.clear();
        batchMap.clear();
}

//////////////////////////////////////////////////

STAGE_SCHED_Queue::STAGE_SCHED_Queue(const STAGE_SCHED_QueueParams &p):

InterQueue(p),
algo_stats(*this),
readSide( *this, 
          p.tt_lotto,
          p.sjf_lotto,
          p.stage1_sizePerSrc, 
          p.stage1_amtSrc, 
          p.stage1_FORMATION_THRED,
          p.stage2_TFDELAY, 
          p.stage3_sizePerBank, 
          p.stage3_amtBank
        ),
writeSide( *this, 
          p.tt_lotto,
          p.sjf_lotto,
          p.stage1_sizePerSrc, 
          p.stage1_amtSrc, 
          p.stage1_FORMATION_THRED,
          p.stage2_TFDELAY, 
          p.stage3_sizePerBank, 
          p.stage3_amtBank
),
amtSrc(p.stage1_amtSrc)
{
        
}


STAGE_SCHED_Queue::STAGE_SCHED_Stats::STAGE_SCHED_Stats(STAGE_SCHED_Queue& ITQ):
statistics::Group(& ITQ),
STAGEQueueOwner(ITQ),
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
        batchMiss,
        statistics::units::Count::get(),
        "number of time that dram is not ready in all selected batch when scheduler need to select"
        ),
ADD_STAT(
        batchHit,
        statistics::units::Count::get(),
        "number of time that dram is not ready in all selected batch when scheduler need to select"
        ),
ADD_STAT(exploitBatch,
         statistics::units::Count::get(),
         "amount of packet that can tie within the last of fifo stage"),
ADD_STAT(startNewBatch,
         statistics::units::Count::get(),
         "amount of packet that must assign new batch number"),
 ADD_STAT(serveByWriteQ,
         statistics::units::Count::get(),
         "amount of memory packet that is serve by write queue")
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
}

void 
STAGE_SCHED_Queue::STAGE_SCHED_Stats::regStats(){
        using namespace statistics;
        exploitBatch .init    (STAGEQueueOwner.amtSrc);
        startNewBatch.init    (STAGEQueueOwner.amtSrc);
        // maxSizeWriteQueue.init(STAGEQueueOwner.maxWriteStageSize+10).flags(nozero);
        // maxSizeReadQueue .init(STAGEQueueOwner.maxReadStageSize +10).flags(nozero);


}



}

} 

