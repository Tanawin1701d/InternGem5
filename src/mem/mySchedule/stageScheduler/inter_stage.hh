#ifndef __MEM_STAGESCHED_SMS__HH__
#define __MEM_STAGESCHED_SMS__HH__

#include "params/InterStage.hh"
#include "mem/mySchedule/inter_queue.hh"
#include "mem/mySchedule/stageScheduler/stages.hh"

namespace gem5{

namespace memory{




class InterStage : public InterQueue{
        //friend struct STAGE_SCHED_Stats;
        public:
            
        private:
        //stat///////////////////////////////////////////
        // struct STAGE_SCHED_Stats : public statistics::Group
        // {
        //     STAGE_SCHED_Stats(STAGE_SCHED_Queue& ITQ);
        //     void regStats() override;
        //     STAGE_SCHED_Queue& STAGEQueueOwner;

        //     statistics::Scalar selectedByRR;
        //     statistics::Scalar selectedBySJF;
        //     statistics::Scalar batchMiss;
        //     statistics::Scalar batchHit;
        //     statistics::Scalar amountPkt;
        //     statistics::Scalar serveByWriteQ;
        //     statistics::Scalar batchExpire;
        //     statistics::Vector exploitBatch;
        //     statistics::Vector startNewBatch;
        //     statistics::Histogram batchedSize;
        //     //statistics::Histogram diffPushTime;
        //     // statistics::Histogram maxSizeReadQueue;

            
        // };
        // STAGE_SCHED_Stats algo_stats;
        //typedef//////////////////////////////////////////
        typedef uint64_t BATCHID;
        typedef uint8_t  QUEUEID;

        
        
        QUEUEID             amtSrc;
        Stages*             readSide;
        WriteStages*        writeSide;

        //stage1//////////
        bool 
        readQueueFull (unsigned int pkt_count, uint8_t subQueueId = 0) override;
        bool 
        writeQueueFull(unsigned int pkt_count, uint8_t subQueueId = 0) override;
        void 
        pushToQueues(MemPacket* mpkt, bool isRead) override;

        // stage3//////////
        [[maybe_unused]]
        std::vector<MemPacketQueue*>
        getQueueToSelect( bool  read ) override;
        
        std::pair<MemPacket*, bool>
        chooseToDram(bool is_read) override;

        bool
        serveByWriteQueue(Addr addr, unsigned size);
        //void
        //notifySel(bool read, MemPacket* mpkt) override;
        //////////////////
        public:
            STAGE_SCHED_Queue(const InterStageParams &p);

    };

}


}


#endif //__MEM_STAGESCHED_BUCKET__HH__