#include"inter_stage.hh"
#include "mem/mySchedule/stageScheduler/stages.hh"
#include "mem/mySchedule/stageScheduler/write_stages.hh"

namespace gem5::memory{


///stage scheduling////////////////////////////////////////

//stage1

bool
InterStage::readQueueFull(unsigned int pkt_count, uint8_t subQueueId){
        
        assert( (amtSrc > subQueueId) && (subQueueId >= 0) );
        readByPass =false;
        if (shouldReadByPass(pkt_count, subQueueId)){
                readByPass =true;
                return false;
        }else{
                // normally case, check stage1. because of stage did not intend to bypass
                return !readSide->canPush(subQueueId, pkt_count);
        }
}

bool
InterStage::writeQueueFull(unsigned int pkt_count, uint8_t subQueueId){
        
        assert( (amtSrc > subQueueId) && (subQueueId >= 0) );
        writeByPass = false;
        if (shouldWriteByPass(pkt_count, subQueueId)){
                writeByPass = true;
                return false;
        }else{
                return !writeSide->canPush(subQueueId, pkt_count);
        }
}

void
InterStage::pushToQueues(MemPacket* mpkt, bool isRead){
        // must copy this to mempacket because main pkt may change
        mpkt->cpuId          = mpkt->pkt->req->cpuId >= 0 ? mpkt->pkt->req->cpuId : 0;
        mpkt->fromNetwork    = mpkt->pkt->req->fromNetwork;
        // this function should be check by writeQueueFull or readQueueFull First.
        // make allow marker work
        ////////////////////
        if (isRead){
                if (readByPass){
                        readSide->pushToQueuesBypass(mpkt);
                }else{
                        readSide->pushToQueues( mpkt );
                }
                
        }else{
                if (writeByPass){
                        writeSide->pushToQueuesBypass(mpkt);
                }else{
                        writeSide->pushToQueues( mpkt );
                }
                
        }
}

MPKC*
InterStage::getMPKC(uint8_t subQueueId){
        return sys->getMPKC_MNG(subQueueId);
}

bool
InterStage::shouldReadByPass(unsigned int pkt_count, uint8_t subQueueId){
        MPKC* mpkc = getMPKC(subQueueId);
        return mpkc && readSide->shouldBypass(pkt_count, subQueueId, mpkc->getMPKC());
        // false because src has no mpkc inspector, mpkc has lower than thredshold or 
        // stage3 is full
}

bool
InterStage::shouldWriteByPass(unsigned int pkt_count, uint8_t subQueueId){
        MPKC* mpkc = getMPKC(subQueueId);
        return mpkc && writeSide->shouldBypass(pkt_count, subQueueId, mpkc->getMPKC());
        // false because src has no mpkc inspector, mpkc has lower than thredshold or 
        // stage3 is full
}

// stage3

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

        inter_Stage_stats.turnrr_all++;

        if ( current_state == qos::MemCtrl::BusState::WRITE){
                panic_if( !writeSide->isCoolDownStarted(),  "cool downing should be start here");
                
                if (writeSide->shouldCoolDownStop() || (writeSide->lower() && !isReadEmpty()) || isWriteEmpty()  ){ 
                        // cause of higher than cooldown time or there are free enough space to switch
                        
                        //stat reccord
                        if ( writeSide->shouldCoolDownStop() ){
                                inter_Stage_stats.turnToR_cooldown++;
                        }else if (writeSide->lower() && !isReadEmpty()){
                                inter_Stage_stats.turnToR_lower++;
                        }else if (isWriteEmpty()){
                                inter_Stage_stats.turnToR_noWrite++;
                        }
                        ////////////////////////////////////////////////
                        writeSide->stopOc();
                        return qos::MemCtrl::BusState::READ;
                }
                return qos::MemCtrl::BusState::WRITE;
        }
        
        
        //case read
        panic_if( writeSide->isCoolDownStarted(),  "cool downing should be start here");
        if  ( writeStageExceed() ){
                inter_Stage_stats.turnToW_exceed++;
        //if  ( writeStageExceed() || (isReadEmpty() && ( !isWriteEmpty() )) ){
                writeSide->startOc();
                return qos::MemCtrl::BusState::WRITE;
        }
        return qos::MemCtrl::BusState::READ;

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
        // write stage1 exceeds the high thredshold
}
bool
InterStage::writeStageLower(){
        return writeSide->lower();
        // write stage1 is lower the low thredshold
}

InterStage::InterStage(const InterStageParams &p):
InterQueue(p),
inter_Stage_stats(*this),
amtSrc    ( p.amt_src    ),
readSide  ( p.readStages ),
writeSide ( p.writeStages),
readByPass( false),
writeByPass(false)
//algo_stats(*this)
{

        assert(readSide  != nullptr);
        assert(writeSide != nullptr);
        readSide ->owner = this;
        writeSide->owner = this;

}

////////////////////////////////////////////////////////////////////////////////////////////////
InterStage::INTER_STAGE_Stats::INTER_STAGE_Stats(InterStage& ITQ):
statistics::Group(& ITQ),
STAGEQueueOwner(ITQ),
ADD_STAT(turnrr_all,
         statistics::units::Count::get(),
         "amount of times that scheduler decide turnaround policy"
        ),
ADD_STAT(turnToR_cooldown,
         statistics::units::Count::get(),
         "times that switch to read by cool down method"
        ),
ADD_STAT(turnToR_lower,
         statistics::units::Count::get(),
         "times that switch to read due to lower thredshold"
        ),
ADD_STAT(turnToR_noWrite,
         statistics::units::Count::get(),
         "times that switch to read due to lack of write request"
        ),
ADD_STAT(turnToW_exceed,
         statistics::units::Count::get(),
         "times that switch to write due to exceed of write request"
        )
{
        using namespace statistics;
}

void 
InterStage::INTER_STAGE_Stats::regStats(){
        using namespace statistics;

}




}