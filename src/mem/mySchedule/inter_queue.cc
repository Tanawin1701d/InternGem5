
#include "inter_queue.hh"

namespace gem5
{


namespace memory{

InterQueue::InterQueue(const InterQueueParams &p):SimObject(p){
    
}

void InterQueue::test(){ 

}


uint8_t 
InterQueue::qFillSel(std::vector<MemPacketQueue>* readQueues, std::vector<MemPacketQueue>* writeQueues, PacketPtr pkt ){ 

    panic("this is unusesd base class [qFillSel]");

}

std::vector<MemPacketQueue*>  
InterQueue::qSchedFill(std::vector<MemPacketQueue>* queues){
    
    panic("this is unusesd base class [qSchedFill]");

}

/////////////////////////////////// simple policy
uint8_t
SimpleQueue::qFillSel(std::vector<MemPacketQueue>* readQueues, std::vector<MemPacketQueue>* writeQueues, PacketPtr pkt ){
    return 0;
}

std::vector<MemPacketQueue*>  
SimpleQueue::qSchedFill(std::vector<MemPacketQueue>* queues){
    
    assert(queues != nullptr); 

    std::vector<MemPacketQueue*> ret(queues->size());

    // for (auto iter = queues->begin(); iter != queues->end(); ++iter){
    //     ret.push_back((MemPacketQueue*)iter);
    // }

    for (int i = 0; i < queues->size(); ++i){
        ret.push_back( &(queues->at(i)) );
    }


    return ret;


}











}

}