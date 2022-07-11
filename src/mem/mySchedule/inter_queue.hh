#ifndef __MEM__QOS_INTER_QUEUE_HH___
#define __MEM__QOS_INTER_QUEUE_HH___

#include "sim/sim_object.hh"
#include "mem/mem_ctrl.hh"
#include "params/InterQueue.hh"
#include "params/SimpleQueue.hh"
#include "params/ALGO_WF_Queue.hh"
#include "params/ALGO_NETQ_Queue.hh"
#include "enums/iterQSched.hh"
#include "debug/interQ.hh"
#include "debug/ALGO_NETQ_Queue.hh"



namespace gem5
{

namespace memory
{

    class InterQueue : public SimObject
    {
        public:

        struct InterQueueStats : public statistics::Group
        {
            InterQueueStats(InterQueue &ITQ);
            void regStats() override;
            InterQueue& INterQueueOwner;

            statistics::Scalar reachNwThreshold;
        };

        InterQueueStats stats;


        virtual uint8_t qFillSel(std::vector<MemPacketQueue>* readQueues, 
                                 std::vector<MemPacketQueue>* writeQueues,
                                 PacketPtr pkt,
                                 uint32_t burst_size) = 0;

        virtual std::vector<MemPacketQueue*>  qSchedFill(std::vector<MemPacketQueue>* queues,
                                                         std::vector<MemPacketQueue>* opsiteQueue,
                                                         bool                         read   ) = 0; // otherwise is write

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
                                                 std::vector<MemPacketQueue>* opsiteQueue,
                                                 bool                         read);

        SimpleQueue(const SimpleQueueParams &params);

    };

    class ALGO_WF_Queue : public InterQueue
    {
        private:
        const int networkQueueId = 1;
        const int cpuQueueId     = 0;
        public:
        uint8_t qFillSel(std::vector<MemPacketQueue>* readQueues, 
                         std::vector<MemPacketQueue>* writeQueues, PacketPtr pkt,
                         uint32_t burst_size);

        std::vector<MemPacketQueue*>  qSchedFill(std::vector<MemPacketQueue>* queues,
                                                 std::vector<MemPacketQueue>* opsiteQueue,
                                                 bool                         read);

        ALGO_WF_Queue(const ALGO_WF_QueueParams &params);


    };

    class ALGO_NETQ_Queue : public InterQueue 
    {
            private:
            const int networkQueueId = 1;
            const int cpuQueueId     = 0;
                  Tick NetAwareThds  = 1;
            public:
            uint8_t qFillSel(std::vector<MemPacketQueue>* readQueues, 
                         std::vector<MemPacketQueue>* writeQueues, PacketPtr pkt,
                         uint32_t burst_size);

            std::vector<MemPacketQueue*>  qSchedFill(std::vector<MemPacketQueue>* queues,
                                                 std::vector<MemPacketQueue>* opsiteQueue,
                                                 bool                         read);

        ALGO_NETQ_Queue(const ALGO_NETQ_QueueParams &params);

    };



}
}


#endif //__MEM__QOS_INTER_QUEUE_HH___
