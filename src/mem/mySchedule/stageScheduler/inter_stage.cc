#include"inter_stage.hh"
#include "mem/mySchedule/stageScheduler/stages.hh"
#include "mem/mySchedule/stageScheduler/write_stages.hh"

namespace gem5::memory{


///stage scheduling////////////////////////////////////////

bool
InterStage::readQueueFull(unsigned int pkt_count, uint8_t subQueueId){
        
        assert( (amtSrc > subQueueId) && (subQueueId >= 0) );
        
        return !readSide->canPush(subQueueId, pkt_count);

}

bool
InterStage::writeQueueFull(unsigned int pkt_count, uint8_t subQueueId){
        
        assert( (amtSrc > subQueueId) && (subQueueId >= 0) );
        
        return !writeSide->canPush(subQueueId, pkt_count);

}

void
InterStage::pushToQueues(MemPacket* mpkt, bool isRead){
        // must copy this to mempacket because main pkt may change
        mpkt->cpuId          = mpkt->pkt->req->cpuId >= 0 ? mpkt->pkt->req->cpuId : 0;
        mpkt->fromNetwork    = mpkt->pkt->req->fromNetwork;
        ////////////////////
        if (isRead){
                readSide->pushToQueues( mpkt );
        }else{
                writeSide->pushToQueues( mpkt );
        }
}

std::pair<MemPacket*, bool>
InterStage::chooseToDram(bool is_read){
        return is_read ? readSide->chooseToDram() : writeSide->chooseToDram();
}

bool
InterStage::serveByWriteQueue(Addr addr, unsigned size){
        return writeSide->serveByWriteQueue(addr, size);
}


std::vector<MemPacketQueue*>
InterStage::getQueueToSelect(bool read){
        std::vector<MemPacketQueue*> ret;
        for (MemPacketQueue& mq : (read ? readSide->stage3Data : writeSide->stage3Data) ){
                ret.push_back(&mq);
        }
        return ret;
}


qos::MemCtrl::BusState 
InterStage::turnpolicy(qos::MemCtrl::BusState current_state){

        if (writeStageExceed()){
                return qos::MemCtrl::BusState::WRITE;
        }else if (isReadEmpty() && !isWriteEmpty()){
                return qos::MemCtrl::BusState::WRITE;
        }else{
                return qos::MemCtrl::BusState::READ;
        }              
}
bool
InterStage::isWriteEmpty(){
        return writeSide->empty();
}
bool
InterStage::isReadEmpty(){
        return readSide->empty();
}
bool
InterStage::writeStageExceed(){
        return writeSide->exceed();
}

InterStage::InterStage(const InterStageParams &p):
InterQueue(p),
readSide  ( p.readStages ),
writeSide ( p.writeStages),
amtSrc    ( p.amt_src    )
//algo_stats(*this)
{

        assert(readSide  != nullptr);
        assert(writeSide != nullptr);
        readSide ->owner = this;
        writeSide->owner = this;

}

////////////////////////////////////////////////////////////////////////////////////////////////
        // STAGE_SCHED_Queue::STAGE_SCHED_Stats::STAGE_SCHED_Stats(STAGE_SCHED_Queue& ITQ):
        // statistics::Group(& ITQ),
        // STAGEQueueOwner(ITQ),
        // ADD_STAT(selectedByRR,
        //          statistics::units::Count::get(),
        //          "amount of times that scheduler select by using rubin"
        //         ),
        // ADD_STAT(
        //         selectedBySJF,
        //         statistics::units::Count::get(),
        //         "amount of times that scheduler select by using short job first"
        //         ),
        // ADD_STAT(
        //         batchMiss,
        //         statistics::units::Count::get(),
        //         "number of time that dram is not ready in all selected batch when scheduler need to select"
        //         ),
        // ADD_STAT(
        //         batchHit,
        //         statistics::units::Count::get(),
        //         "number of time that dram is not ready in all selected batch when scheduler need to select"
        //         ),
        // ADD_STAT(exploitBatch,
        //          statistics::units::Count::get(),
        //          "amount of packet that can tie within the last of fifo stage"),
        // ADD_STAT(startNewBatch,
        //          statistics::units::Count::get(),
        //          "amount of packet that must assign new batch number"),
        //  ADD_STAT(serveByWriteQ,
        //          statistics::units::Count::get(),
        //          "amount of memory packet that is serve by write queue"),
        // ADD_STAT(batchExpire,
        //          statistics::units::Count::get(),
        //          "expire batch"),
        // ADD_STAT(batchedSize,
        //         statistics::units::Count::get(),
        //         "batch size in stage1 before picked to stage3")//,
        // // ADD_STAT(diffPushTime,
        // //         statistics::units::Count::get(),
        // //         "differrent time between 2 packet arrive")
        // //,
        // // ADD_STAT(
        // //         maxSizeWriteQueue,
        // //         statistics::units::Count::get(),
        // //         "max size of mempacket that fill in each write queue"),
        // // ADD_STAT(
        // //         maxSizeReadQueue,
        // //         statistics::units::Count::get(),
        // //         "max size of mempacket that fill in each read queue")
        // {
        //         using namespace statistics;

        //         batchedSize
        //         .init(1024)
        //         .flags(nozero);
        //         //diffPushTime
        //         //.init(1);
        //         // TODO for now we deactivate it

        // }

        // void 
        // STAGE_SCHED_Queue::STAGE_SCHED_Stats::regStats(){
        //         using namespace statistics;
        //         exploitBatch .init    (STAGEQueueOwner.amtSrc);
        //         startNewBatch.init    (STAGEQueueOwner.amtSrc);
        //         // maxSizeWriteQueue.init(STAGEQueueOwner.maxWriteStageSize+10).flags(nozero);
        //         // maxSizeReadQueue .init(STAGEQueueOwner.maxReadStageSize +10).flags(nozero);


// }




}