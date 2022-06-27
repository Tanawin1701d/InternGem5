
#include "inter_queue.hh"
//#include "debug/interQ.hh"

namespace gem5
{


namespace memory{

////////////////////////////////// abstract policy
InterQueue::InterQueue(const InterQueueParams &p):SimObject(p){
    
}

void InterQueue::setQ(std::vector<MemPacketQueue>* rq, 
                      std::vector<MemPacketQueue>* wq
                    ){
                        readQueue = rq;
                         writeQueue = wq;
                    }


/////////////////////////////////// simple policy

SimpleQueue::SimpleQueue(const SimpleQueueParams& params) : InterQueue(params){

}

uint8_t
SimpleQueue::qFillSel(std::vector<MemPacketQueue>* readQueues, 
                      std::vector<MemPacketQueue>* writeQueues,
                      PacketPtr pkt,
                      uint32_t burst_size ){
    pkt->qosValue(0);
    DPRINTF(interQ,"return 0 qos prio\n");
    return 0;
}

std::vector<MemPacketQueue*>  
SimpleQueue::qSchedFill(std::vector<MemPacketQueue>* queues, bool read){
    
    assert(queues != nullptr); 

    DPRINTF(interQ,"sched sel\n");
    std::vector<MemPacketQueue*> ret;

    // for (auto iter = queues->begin(); iter != queues->end(); ++iter){
    //     ret.push_back((MemPacketQueue*)iter);
    // }

    for (int i = 0; i < queues->size(); ++i){
        ret.push_back( &(queues->at(i)) );
    }
    DPRINTF(interQ,"sched sel fin\n");


    return ret;


}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////// ALGO_WF_Queue

ALGO_WF_Queue::ALGO_WF_Queue(const ALGO_WF_QueueParams& params) : InterQueue(params){

}

uint8_t
ALGO_WF_Queue::qFillSel(std::vector<MemPacketQueue>* readQueues, 
                        std::vector<MemPacketQueue>* writeQueues,
                        PacketPtr pkt,
                        uint32_t burst_size ){
    pkt->qosValue( (int) pkt->req->fromNetwork );
    DPRINTF(interQ,"is prior set to %d\n", (int) pkt->req->fromNetwork);
    return (int) pkt->req->fromNetwork;

}

std::vector<MemPacketQueue*>  
ALGO_WF_Queue::qSchedFill(std::vector<MemPacketQueue>* queues, bool read){
    
    assert(queues != nullptr);
    DPRINTF(interQ,"wfq sched sel\n");
    std::vector<MemPacketQueue*> ret;

    // for (auto iter = queues->begin(); iter != queues->end(); ++iter){
    //     ret.push_back((MemPacketQueue*)iter);
    // }

    if ((*queues)[cpuQueueId].size() >  (*queues)[networkQueueId].size()){
        ret.push_back(&(queues->at(cpuQueueId)));
    }else{
        ret.push_back(&(queues->at(networkQueueId)));
    }
    
    DPRINTF(interQ,"wfq sched sel fin\n");


    return ret;


}





}

}

