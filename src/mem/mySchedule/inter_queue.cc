
#include "inter_queue.hh"
//#include "debug/interQ.hh"

namespace gem5
{


namespace memory{

////////////////////////////////// abstract policy
InterQueue::InterQueue(const InterQueueParams &p):
                            
                                SimObject(p),
                                stats(*this)
                                {
    
}

InterQueue::InterQueueStats::InterQueueStats(InterQueue& ITQ): 
statistics::Group(& ITQ),
INterQueueOwner(ITQ),
ADD_STAT(reachNwThreshold,
         statistics::units::Count::get(),
         "amount of time that interqueue scheduler must serve net work packet due to qos of network"
        )

{
}

void
InterQueue::InterQueueStats::regStats(){};

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
SimpleQueue::qSchedFill(std::vector<MemPacketQueue>* queues,
                        std::vector<MemPacketQueue>* opsiteQueue,
                         bool read){
    
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
ALGO_WF_Queue::qSchedFill(std::vector<MemPacketQueue>* queues,
                          std::vector<MemPacketQueue>* opsiteQueue,
                          bool read){
    
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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////// ALGO_NETQ_Queue

ALGO_NETQ_Queue::ALGO_NETQ_Queue(const ALGO_NETQ_QueueParams& params) : 
InterQueue(params),
NetAwareThds( params.NetAwareThds ){

}

uint8_t
ALGO_NETQ_Queue::qFillSel(std::vector<MemPacketQueue>* readQueues, 
                          std::vector<MemPacketQueue>* writeQueues,
                          PacketPtr pkt,
                          uint32_t burst_size ){
    pkt->qosValue( (int) pkt->req->fromNetwork );
    DPRINTF(interQ,"is prior set to %d\n", (int) pkt->req->fromNetwork);
    return (int) pkt->req->fromNetwork;

}

std::vector<MemPacketQueue*>  
ALGO_NETQ_Queue::qSchedFill(std::vector<MemPacketQueue>* queues,
                            std::vector<MemPacketQueue>* opsiteQueue,
                            bool read){
    
    assert(queues != nullptr);
    //DPRINTF(interQ,"net aware sched sel\n");
    std::vector<MemPacketQueue*> ret;

    // for (auto iter = queues->begin(); iter != queues->end(); ++iter){
    //     ret.push_back((MemPacketQueue*)iter);
    // }

    // if ((*queues)[cpuQueueId].size() >  (*queues)[networkQueueId].size()){
    //     ret.push_back(&(queues->at(cpuQueueId)));
    // }else{
    //     ret.push_back(&(queues->at(networkQueueId)));
    // }
    //DPRINTF(ALGO_NETQ_Queue,"netq encounter fill \n");
    //DPRINTF(ALGO_NETQ_Queue,"%d nw %d cpu \n");

    if ((!(*queues)[networkQueueId].empty() ) &&
        (
            ((curTick() - (*queues)[networkQueueId].front()->queueAddedTime) > NetAwareThds) ||
            (*queues)[cpuQueueId].empty()
            
        )
       )
    {
        if (((curTick() - (*queues)[networkQueueId].front()->queueAddedTime) > NetAwareThds)){
            stats.reachNwThreshold++;
            DPRINTF(ALGO_NETQ_Queue,"netq encounter limit of waiting \n");
        }else{
            DPRINTF(ALGO_NETQ_Queue,"netq encounter free avaiable \n");
        }
        ret.push_back(&(queues->at(networkQueueId)));
    }
    else{
        ret.push_back(&(queues->at(cpuQueueId)));
    }
    
    //DPRINTF(interQ,"net aware sched sel fin\n");


    return ret;


}


}

} 

