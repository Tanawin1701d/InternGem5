#ifndef __MEM__QOS_INTER_QUEUE_HH___
#define __MEM__QOS_INTER_QUEUE_HH___

#include "sim/sim_object.hh"
#include "mem/mem_ctrl.hh"
#include "params/InterQueue.hh"
#include "params/SimpleQueue.hh"
#include "params/ALGO_WF_Queue.hh"
#include "enums/iterQSched.hh"
#include "debug/interQ.hh"



namespace gem5
{

namespace memory
{

    class InterQueue : public SimObject
    {
        public:

        std::vector<MemPacketQueue>* readQueue;
        std::vector<MemPacketQueue>* writeQueue;

        virtual uint8_t qFillSel(std::vector<MemPacketQueue>* readQueues, 
                                 std::vector<MemPacketQueue>* writeQueues, PacketPtr pkt,
                                 uint32_t burst_size) = 0;

        virtual std::vector<MemPacketQueue*>  qSchedFill(std::vector<MemPacketQueue>* queues,
                                                         bool                         read   ) = 0; // otherwise is write


        void setQ(std::vector<MemPacketQueue>* rq,
                  std::vector<MemPacketQueue>* wq
        );
        //static InterQueue* creator(enums::iterQSched  iterPolicy);
        InterQueue(const InterQueueParams &p);

    };

    class SimpleQueue : public InterQueue
    {
        public:
        
        uint8_t qFillSel(std::vector<MemPacketQueue>* readQueues, 
                                 std::vector<MemPacketQueue>* writeQueues, PacketPtr pkt,
                                 uint32_t burst_size);

        std::vector<MemPacketQueue*>  qSchedFill(std::vector<MemPacketQueue>* queues,
                                                 bool                         read);

        SimpleQueue(const SimpleQueueParams &params);

    };

    class ALGO_WF_Queue : public InterQueue
    {
        private:
        const int networkQueueId = 1;
        const int cpuQueueId = 0;
        public:
        uint8_t qFillSel(std::vector<MemPacketQueue>* readQueues, 
                         std::vector<MemPacketQueue>* writeQueues, PacketPtr pkt,
                         uint32_t burst_size);

        std::vector<MemPacketQueue*>  qSchedFill(std::vector<MemPacketQueue>* queues,
                                                 bool                         read);

        ALGO_WF_Queue(const ALGO_WF_QueueParams &params);


    };



}
}


#endif //__MEM__QOS_INTER_QUEUE_HH___
