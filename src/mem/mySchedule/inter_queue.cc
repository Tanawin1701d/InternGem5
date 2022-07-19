
#include "inter_queue.hh"
#include "mem/mem_interface.hh"

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
        ),
ADD_STAT(
        switchRW,
        statistics::units::Count::get(),
        "amount of switch RW due to qos of network"
        )
{
}

void
InterQueue::InterQueueStats::regStats(){};

void 
InterQueue::notifySelect(MemPacket* mempkt, bool is_read,                          std::vector<MemPacketQueue>* queues
                          std::vector<MemPacketQueue>* queues
){}

void
InterQueue::turnpolicy(qos::MemCtrl::BusState& bs,
                        std::vector<MemPacketQueue>* readQueues, 
                        std::vector<MemPacketQueue>* writeQueues){}
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

void push_to_queue(MemPacket* mempkt,
                    std::vector<MemPacketQueue>* queues,
                    std::vector<MemPacketQueue>* opsiteQueue,
                    bool                         is_read,
                    uint8_t                      qid,
                    DRAMInterface*               dram
                    ){
                        (*queues)[qid].push_back(mempkt);
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

void
ALGO_NETQ_Queue::turnpolicy(qos::MemCtrl::BusState& bs,
                            std::vector<MemPacketQueue>* readQueues, 
                            std::vector<MemPacketQueue>* writeQueues){
    
    if ((!(*readQueues )[networkQueueId].empty()) &&
        (!(*writeQueues)[networkQueueId].empty())
    ){

        bs = 
            (*readQueues )[networkQueueId].front()->queueAddedTime >=
            (*writeQueues)[networkQueueId].front()->queueAddedTime ? 
            qos::MemCtrl::BusState::WRITE : qos::MemCtrl::BusState::READ;
    }else if(!(*readQueues )[networkQueueId].empty()) {
        DPRINTF(ALGO_NETQ_Queue,"net aware turnpolicy select read\n");
        bs = qos::MemCtrl::BusState::READ;
        stats.switchRW++;
    }else if(!(*writeQueues)[networkQueueId].empty()){
        DPRINTF(ALGO_NETQ_Queue,"net aware turnpolicy select write\n");
        bs = qos::MemCtrl::BusState::WRITE;
        stats.switchRW++;
    }
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////// STAGE_SCHED_Queue

// must override
uint8_t 
STAGE_SCHED_Queue::qFillSel(std::vector<MemPacketQueue>* readQueues, 
                                 std::vector<MemPacketQueue>* writeQueues,
                                 PacketPtr pkt,
                                 uint32_t burst_size){

    int curCpuId = pkt->req->cpuId;
    if ( curCpuId >= 0){
        // for now we also treat cpu as network cpu id;
    }else{
        curCpuId = 0;
        // cold start;
    }

    pkt->qosValue(curCpuId);
    return curCpuId;
}

std::vector<MemPacketQueue*>
STAGE_SCHED_Queue::qSchedFill(std::vector<MemPacketQueue>* queues,
                              std::vector<MemPacketQueue>* opsiteQueue,
                              bool                         read   ){

    std::vector<batchState>* relatedStages;
    state*                   relatedState;
    uint8_t*                 relatedSeletedStageNum;// if exploit state if not it can be anythings

    if (read){
        relatedStages          = &readStages;
        relatedState           = &ReadCmdStatus;
        relatedSeletedStageNum = &selectedReadStage;
    }else{
        relatedStages          = &writeStages;
        relatedState           = &WriteCmdStatus;
        relatedSeletedStageNum = &selectedWriteStage;
    }

    // return template
    std::vector<MemPacketQueue*> ret;
    //////////////////////////

    if (*relatedState == state::exploiting){

        // return that state
        ret.push_back(&((*queues)[*relatedSeletedStageNum])); // for now it is cpu id
    }else if ( *relatedState == state::wait4Sel ){
        //select new stage
        // use lottery
        StagePolicy next_policy = genlotto();
        if (next_policy == StagePolicy::rr){

            uint8_t nextStage = ((*relatedSeletedStageNum) + 1)%numStages;
            while( nextStage !=  *relatedSeletedStageNum){
                if ((*relatedStages)[nextStage].startBatchId < (*relatedStages)[nextStage].lastBatchId){
                    ret.push_back(&(*queues)[nextStage]);
                    *relatedSeletedStageNum = nextStage;
                    break;
                }
            }
        }else if (next_policy == StagePolicy::sjf){
            uint8_t nextStage = 0;
            uint8_t  ans_stage = nextStage;
            uint64_t ans_amt   = UINT64_MAX;

            for (;nextStage < numStages; nextStage++){
                if (!(*queues)[nextStage].empty()){
                    BATCHID  bid = ((*queues)[nextStage].front())->batchId;
                    uint64_t amt = (*relatedStages)[nextStage].batchMap[bid];
                    if ( ans_amt > amt ){
                        ans_stage = nextStage;
                    }
                }
            }
            *relatedSeletedStageNum = nextStage;
            ret.push_back(&(*queues)[ans_stage]);
        }else{
            panic("unknow scheduling policy\n");
        }

    }else{
        panic("unknown stage state\n");
    }

    return ret;




}

void
STAGE_SCHED_Queue::notifySelect(MemPacket* mempkt, bool is_read,
                          std::vector<MemPacketQueue>* queues
){
    std::vector<batchState>* relatedStages;
    state*                   relatedState;
    uint8_t*                 relatedSeletedStageNum;// if exploit state if not it can be anythings

    if (is_read){
        relatedStages          = &readStages;
        relatedState           = &ReadCmdStatus;
        relatedSeletedStageNum = &selectedReadStage;
    }else{
        relatedStages          = &writeStages;
        relatedState           = &WriteCmdStatus;
        relatedSeletedStageNum = &selectedWriteStage;
    }

    if ( mempkt ){

    uint64_t& batchSizeRef = (*relatedStages)[*relatedSeletedStageNum]
                             .batchMap[mempkt->batchId];
    assert( batchSizeRef >= 1);

    *relatedState = (batchSizeRef >= 2) ? state::exploiting :
                                          state::wait4Sel;
    batchSizeRef-=1;
    }else{
        *relatedState =  (*queues)[*relatedSeletedStageNum].empty() ? state::exploiting : state::wait4Sel;
    }
}

void
STAGE_SCHED_Queue::push_to_queue(MemPacket* mempkt,
              std::vector<MemPacketQueue>* queues,
              std::vector<MemPacketQueue>* opsiteQueue,
              bool                         is_read,
              uint8_t                      qid,
              DRAMInterface*               dram
             ){
    bool AssignNewBatch = true;
             // for now we assume that gem5 handle merger for us perfectly
    std::vector<batchState>* relatedStages = is_read ? &readStages 
                                                     : &writeStages;


    assert(qid <= queues->size());


    if (!(*queues)[qid].empty()){
        MemPacket* lastMemPkt = (*queues)[qid].back();
        if ( dram->isSameRow(lastMemPkt, mempkt) ){
             AssignNewBatch = false;
        }
    }


    batchState* curbatchState = &((*relatedStages)[qid]);
    if (AssignNewBatch){
        if ((*queues)[qid].empty()){
            //restart that statge
            mempkt->batchId = 0; // assign batch id
            curbatchState->batchMap.clear();// clear map
            curbatchState->batchMap[0]  = 1; // 1 packet in first batch
            curbatchState->startBatchId = 0;// reset start index
            curbatchState->lastBatchId  = 1;// reset stop index
        }else{
            //not row hit take queue
            mempkt->batchId = curbatchState->lastBatchId; // assign batch id
            curbatchState->batchMap[curbatchState->lastBatchId] = 1;// reset stop index
            curbatchState->lastBatchId++; // 1 packet in first batch
        }
    }else{
        // no change last batch 
        assert(curbatchState->startBatchId < curbatchState->lastBatchId);
        mempkt->batchId = curbatchState->lastBatchId-1;
        curbatchState->batchMap[curbatchState->lastBatchId-1]++;
    }

    



}

STAGE_SCHED_Queue::StagePolicy

STAGE_SCHED_Queue::genlotto(){
    uint8_t lot = (uint8_t)(rand() % max_lotto);
    return (lot <= rr_max_lotto) ? StagePolicy::rr : StagePolicy::sjf;
}

STAGE_SCHED_Queue::STAGE_SCHED_Queue(const STAGE_SCHED_QueueParams &p):
numStages(p.numStages),
rr_max_lotto(p.rr_max_lotto),
max_lotto(p.max_lotto)
{
    readStages.resize(numStages);
    writeStages.resize(numStages);
    ReadCmdStatus       = state::wait4Sel;
    WriteCmdStatus      = state::wait4Sel;
    selectedReadStage   = 0;
    selectedWriteStage  = 0;
}

}

} 

