#ifndef __MEM__QOS_INTER_QUEUE_HH___
#define __MEM__QOS_INTER_QUEUE_HH___

#include "sim/sim_object.hh"
#include "mem/mem_ctrl.hh"
#include "params/InterQueue.hh"
// #include "params/SimpleQueue.hh"
// #include "params/ALGO_WF_Queue.hh"
// #include "params/ALGO_NETQ_Queue.hh"
#include "enums/iterQSched.hh"
#include "debug/interQ.hh"
#include "debug/ALGO_NETQ_Queue.hh"
#include "debug/SMS.hh"
#include <unordered_map>



namespace gem5
{

namespace memory
{

    class InterQueue : public SimObject
    {
        public:
        MemCtrl* mctrl;
        System*  sys;


        virtual bool
        readQueueFull(unsigned int pkt_count,  uint8_t subQueueId) = 0;
        virtual bool
        writeQueueFull(unsigned int pkt_count, uint8_t subQueueId) = 0;
        virtual void
        pushToQueues(MemPacket* mpkt, bool isRead) = 0;
        virtual std::vector<MemPacketQueue*>
        getQueueToSelect( bool  read ) = 0; // otherwise is write
        //virtual void // todo we may add iterator parameter for this notify select
        //notifySel(bool read, MemPacket* mpkt) = 0;
        virtual std::pair<MemPacket*, bool>
        chooseToDram(bool is_read) = 0;
        virtual bool 
        serveByWriteQueue(Addr addr, unsigned size) = 0;
        //select next bus state // due to main controller must handle draining state we must set default to read
        virtual qos::MemCtrl::BusState 
        turnpolicy(qos::MemCtrl::BusState current_state) = 0;
        virtual bool
        isWriteEmpty() = 0;
        virtual bool
        isReadEmpty() = 0;
        virtual bool
        writeStageExceed() = 0;


        InterQueue(const InterQueueParams &p);

    };

    

}
}


#endif //__MEM__QOS_INTER_QUEUE_HH___
