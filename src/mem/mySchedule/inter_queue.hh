#ifndef __MEM__QOS_INTER_QUEUE_HH___
#define __MEM__QOS_INTER_QUEUE_HH___

#include "sim/sim_object.hh"
#include "mem/mem_ctrl.hh"
#include "params/InterQueue.hh"
#include "params/SimpleQueue.hh"
#include "params/ALGO_WF_Queue.hh"
#include "params/ALGO_NETQ_Queue.hh"
#include "params/STAGE_SCHED_Queue.hh"
#include "enums/iterQSched.hh"
#include "debug/interQ.hh"
#include "debug/ALGO_NETQ_Queue.hh"
#include <unordered_map>



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
            statistics::Scalar switchRW;
        };

        InterQueueStats stats;


        virtual uint8_t qFillSel(std::vector<MemPacketQueue>* readQueues, 
                                 std::vector<MemPacketQueue>* writeQueues,
                                 PacketPtr pkt,
                                 uint32_t burst_size) = 0;

        virtual std::vector<MemPacketQueue*>  qSchedFill(std::vector<MemPacketQueue>* queues,
                                                         std::vector<MemPacketQueue>* opsiteQueue,
                                                         bool                         read   ) = 0; // otherwise is write

        virtual void notifySelect(MemPacket* mempkt, bool is_read,
                                  std::vector<MemPacketQueue>* queues
);

        virtual void turnpolicy(qos::MemCtrl::BusState& bs,
                                std::vector<MemPacketQueue>* readQueues, 
                                std::vector<MemPacketQueue>* writeQueues);
        //static InterQueue* creator(enums::iterQSched  iterPolicy);

        virtual void push_to_queue(MemPacket* mempkt,
                          std::vector<MemPacketQueue>* queues,
                          std::vector<MemPacketQueue>* opsiteQueue,
                          bool                         is_read,
                          uint8_t                      qid,
                          DRAMInterface*               dram
                          );

        

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

            
            void turnpolicy(qos::MemCtrl::BusState& bs,
                            std::vector<MemPacketQueue>* readQueues, 
                            std::vector<MemPacketQueue>* writeQueues
                            ) override;


        ALGO_NETQ_Queue(const ALGO_NETQ_QueueParams &params);

    };

    class STAGE_SCHED_Queue : public InterQueue{
        friend struct STAGE_SCHED_Stats;
        public:
            typedef uint64_t BATCHID;
        private:
        //stat///////////////////////////////////////////
        struct STAGE_SCHED_Stats : public statistics::Group
        {
            STAGE_SCHED_Stats(STAGE_SCHED_Queue& ITQ);
            void regStats() override;
            STAGE_SCHED_Queue& STAGEQueueOwner;

            statistics::Scalar selectedByRR;
            statistics::Scalar selectedBySJF;
            statistics::Scalar batchMiss;
            statistics::Scalar batchHit;
            statistics::Scalar amountPkt;
            statistics::Vector exploitBatch;
            statistics::Vector startNewBatch;
            statistics::Histogram maxSizeWriteQueue;
            statistics::Histogram maxSizeReadQueue;

            
        };


        STAGE_SCHED_Stats algo_stats;
        ////////////////////////////////////////////////
            typedef uint8_t  QUEUEID;
            enum state {exploiting, wait4Sel};
            enum StagePolicy {rr, sjf};

            
            
            struct batchState{
                BATCHID startBatchId = 0;
                BATCHID lastBatchId = 0;
                std::unordered_map <BATCHID, uint64_t> batchMap;
            };

            uint8_t  numStages;
            uint64_t maxReadStageSize;
            uint64_t maxWriteStageSize;
            uint8_t  rr_max_lotto;
            uint8_t  max_lotto;

            std::vector<batchState> readStages;
            std::vector<batchState> writeStages;
            state                   ReadCmdStatus;
            state                   WriteCmdStatus;
            uint8_t                 selectedlastReadStage_RR;
            uint8_t                 selectedlastWriteStage_RR;
            uint8_t                 selectedReadStage;
            uint8_t                 selectedWriteStage;

        public:
        // must override
        uint8_t qFillSel(std::vector<MemPacketQueue>* readQueues, 
                         std::vector<MemPacketQueue>* writeQueues,
                         PacketPtr pkt,
                         uint32_t burst_size);

        std::vector<MemPacketQueue*>  qSchedFill(std::vector<MemPacketQueue>* queues,
                                                 std::vector<MemPacketQueue>* opsiteQueue,
                                                 bool                         read   );

        virtual void notifySelect(MemPacket* mempkt, bool is_read,
                                  std::vector<MemPacketQueue>* queues);

        // must have for this 
        void push_to_queue(MemPacket*                  mempkt     ,
                          std::vector<MemPacketQueue>* queues     ,
                          std::vector<MemPacketQueue>* opsiteQueue,
                          bool                         is_read    ,
                          uint8_t                      qid        ,
                          DRAMInterface*               dram
                          );
    
        StagePolicy genlotto();

        bool readQueueFull(unsigned int neededEntries,
                           std::vector<MemPacketQueue>* read_queues,
                           int qid
                          );
        bool writeQueueFull(unsigned int neededEntries,
                           std::vector<MemPacketQueue>* write_queues,
                           int qid
                           );

        STAGE_SCHED_Queue(const STAGE_SCHED_QueueParams &p);



    
    };

}
}


#endif //__MEM__QOS_INTER_QUEUE_HH___
