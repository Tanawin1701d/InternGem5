#ifndef __MEM__QOS_INTER_QUEUE_HH___
#define __MEM__QOS_INTER_QUEUE_HH___

#include "sim/sim_object.hh"
#include "mem/mem_ctrl.hh"
#include "params/InterQueue.hh"
// #include "params/SimpleQueue.hh"
// #include "params/ALGO_WF_Queue.hh"
// #include "params/ALGO_NETQ_Queue.hh"
#include "params/STAGE_SCHED_Queue.hh"
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

        InterQueue(const InterQueueParams &p);

    };

    class STAGE_SCHED_Queue : public InterQueue{
        friend struct STAGE_SCHED_Stats;
        public:
            
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
            statistics::Scalar serveByWriteQ;
            statistics::Scalar batchExpire;
            statistics::Vector exploitBatch;
            statistics::Vector startNewBatch;
            statistics::Histogram batchedSize;
            //statistics::Histogram diffPushTime;
            // statistics::Histogram maxSizeReadQueue;

            
        };
        STAGE_SCHED_Stats algo_stats;
        //typedef//////////////////////////////////////////
        typedef uint64_t BATCHID;
        typedef uint8_t  QUEUEID;

        
        class stageMetaData{
            public:
            enum STAGE2_STATE  {pick, drain};
            enum STAGE2_PICK   {rr  , sjf  };
            // struct BucketMeta{
            //     std::unordered_map<BATCHID, uint64_t> bs;
            // };
            struct BatchMeta{
                Tick     firstAddedTime = 0;
                bool     isBatchReady   = false;
                uint64_t batchSize      = 0;
            };

            struct BucketMeta{
                
                Tick FORMATION_THRED;
                Tick lastAdded = 0;
                stageMetaData* owner;
                std::unordered_map<BATCHID, BatchMeta> batchMap;
                MemPacketQueue dayta;
                uint64_t curSize = 0;
                uint64_t maxSize = 0;
                BATCHID  lastBatchId = -1;

                bool empty() { return curSize == 0; }
                bool canPush(uint64_t neededEntry);
                bool canPop();
                void push(MemPacket* mpkt);
                MemPacket* pop();
                MemPacket* front();
                void updateBatchStatus(); // update last batch is reach the thredshold
                bool isRowHit(MemPacket* a, MemPacket* b);
                void clear();
            
            };

            STAGE_SCHED_Queue&           owner;
            uint32_t                     TOTALLOTO;
            uint32_t                     SJFLOTTO;
            //stageG
            std::vector<uint64_t>        stageGSize; // use to track sjf
            //stage1
            uint64_t                     stage1PktCount;
            QUEUEID                      stage1AMTBUCKET;
            std::vector<BucketMeta>      stage1Meta;
            Tick                         stage1_FORMATION_THRED;
            //stage2
            QUEUEID                      lastRRBucket;
            QUEUEID                      selBucket;
            STAGE2_STATE                 stage2CurState;
            Tick                         stage2TFDL;
            EventFunctionWrapper         nextStage2Event;
            //stage3
            QUEUEID                      selBank;
            uint64_t                     stage3PktCount;
            uint8_t                      stage3AmtBank;
            std::vector<uint64_t>        stage3Size;
            std::vector<MemPacketQueue>  stage3Data;

            //stage1
            void pushToQueues(MemPacket* mpkt);
            void updateAllBatchStatus();
            //stage2
            STAGE2_PICK genlotto();
            void processStage2Event();
            //stage3
            bool stage3full(uint8_t bankNum, uint64_t  reqEntry = 1);
            std::pair<MemPacket*, bool>
            chooseToDram();
            //debuger
            void printStage(std::vector<MemPacketQueue>& stageRef);

            void printStage(std::vector<BucketMeta>& stageRef);
            //constructor
            stageMetaData( STAGE_SCHED_Queue& own,
                            uint32_t  tt_lotto,
                            uint32_t  sjf_lotto,
                            uint64_t  stage1_sizePerSrc,
                            uint8_t   amtSrc,
                            Tick      stage1_FORMATION_THRED,
                            Tick      stage2_TFDELAY,
                            uint64_t  stage3_sizePerBank,
                            uint8_t   amtBank
                        );
            
        };
        
        class writeStageMetaData : public stageMetaData{
            public:
            bool serveByWriteQueue(Addr addr, unsigned size);
            bool serveByWriteQueue(Addr addr, unsigned size, MemPacketQueue& srcToFind);
            writeStageMetaData( STAGE_SCHED_Queue& own,
                            uint32_t  tt_lotto,
                            uint32_t  sjf_lotto,
                            uint64_t  stage1_sizePerSrc,
                            uint8_t   amtSrc,
                            Tick      stage1_FORMATION_THRED,
                            Tick      stage2_TFDELAY,
                            uint64_t  stage3_sizePerBank,
                            uint8_t   amtBank
                        );
        };

        QUEUEID            amtSrc;
        stageMetaData      readSide;
        writeStageMetaData writeSide;

        //stage1//////////
        bool 
        readQueueFull (unsigned int pkt_count, uint8_t subQueueId = 0) override;
        bool 
        writeQueueFull(unsigned int pkt_count, uint8_t subQueueId = 0) override;
        void 
        pushToQueues(MemPacket* mpkt, bool isRead) override;

        //////////////////

        // stage2 is laid in struct//////////
        //////////////////

        // stage3//////////
        [[maybe_unused]]
        std::vector<MemPacketQueue*>
        getQueueToSelect( bool  read ) override;
        
        std::pair<MemPacket*, bool>
        chooseToDram(bool is_read);

        bool
        serveByWriteQueue(Addr addr, unsigned size);
        //void
        //notifySel(bool read, MemPacket* mpkt) override;
        //////////////////
        public:
            STAGE_SCHED_Queue(const STAGE_SCHED_QueueParams &p);

    };

}
}


#endif //__MEM__QOS_INTER_QUEUE_HH___
