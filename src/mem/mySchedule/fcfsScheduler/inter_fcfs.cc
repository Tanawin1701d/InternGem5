#include"inter_fcfs.hh"


namespace gem5::memory{


///stage scheduling////////////////////////////////////////

//stage1

bool
InterFcfs::readQueueFull(unsigned int pkt_count, uint8_t subQueueId){
        
        return readSide.size() > qmSize;
}

bool
InterFcfs::writeQueueFull(unsigned int pkt_count, uint8_t subQueueId){
        
        return writeSide.size() > qmSize;
}

void
InterFcfs::pushToQueues(MemPacket* mpkt, bool isRead){
        if (isRead){
                readSide.push_back(mpkt);
        }else{
                writeSide.push_back(mpkt);
        }
        
        if ( !(mctrl)->requestEventScheduled() ){
                mctrl->restartScheduler(curTick());
        }

}

// stage3

std::pair<MemPacket*, bool>
InterFcfs::chooseToDram(bool is_read){
        if (is_read && !readSide.empty()){
                if (mctrl->packetReady(readSide.front())){
                        MemPacket* mpkt = readSide.front();     
                        readSide.pop_front();
                        return { mpkt, true};
                }
        }else if ((!is_read) && (!writeSide.empty())){
                if (mctrl->packetReady(writeSide.front())){
                        MemPacket* mpkt = writeSide.front();     
                        writeSide.pop_front();
                        return { mpkt, true};
                }
        }

        return {nullptr, false};
}

bool
InterFcfs::serveByWriteQueue(Addr addr, unsigned size){
        for (auto mpkt: writeSide){
                if ((addr >= mpkt->addr) && ((addr + size)<=(mpkt->addr + mpkt->size))){
                        return true;
                }
        }
        return false;
}


std::vector<MemPacketQueue*>
InterFcfs::getQueueToSelect(bool read){
        return {nullptr};
}


qos::MemCtrl::BusState 
InterFcfs::turnpolicy(qos::MemCtrl::BusState current_state){

        if (writeStageExceed()){
                return qos::MemCtrl::BusState::WRITE;
        }else if (isReadEmpty() && !isWriteEmpty()){
                return qos::MemCtrl::BusState::WRITE;
        }else{
                return qos::MemCtrl::BusState::READ;
        }              
}
bool
InterFcfs::isWriteEmpty(){
        return writeSide.empty();
}
bool
InterFcfs::isReadEmpty(){
        return readSide.empty();
}
bool
InterFcfs::writeStageExceed(){
        return writeSide.size() > qmSize;
}

InterFcfs::InterFcfs(const InterFcfsParams &p):
qmSize(64),
InterQueue(p)
{
}

}