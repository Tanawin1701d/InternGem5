#ifndef __MEM_STAGESCHED_SMS__HH__
#define __MEM_STAGESCHED_SMS__HH__

#include "params/InterStage.hh"
#include "mem/mySchedule/inter_queue.hh"
#include "sim/system.hh"
#include "cpu/mpkc/mpkc.hh"

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
        bool                readByPass;
        bool                writeByPass;

        //stage1//////////
        bool 
        readQueueFull (unsigned int pkt_count, uint8_t subQueueId = 0) override;
        bool 
        writeQueueFull(unsigned int pkt_count, uint8_t subQueueId = 0) override;
        void 
        pushToQueues(MemPacket* mpkt, bool isRead) override;

            //  if stage3 is full incase bypassing, it also return false
        MPKC* 
        getMPKC(uint8_t subQueueId); // indeed, it is cpuid

        bool 
        shouldReadByPass(unsigned int pkt_count, uint8_t subQueueId = 0);

        bool 
        shouldWriteByPass(unsigned int pkt_count, uint8_t subQueueId = 0);

        // stage3//////////
        [[maybe_unused]]
        std::vector<MemPacketQueue*>
        getQueueToSelect( bool  read ) override;
        
        std::pair<MemPacket*, bool>
        chooseToDram(bool is_read) override;

        bool
        serveByWriteQueue(Addr addr, unsigned size) override;

        qos::MemCtrl::BusState 
        turnpolicy(qos::MemCtrl::BusState current_state) override;
        bool
        isWriteEmpty() override;
        bool
        isReadEmpty() override;
        bool
        writeStageExceed() override;
        //////////////////
        public:
            InterStage(const InterStageParams &p);

    };

}


}


#endif //__MEM_STAGESCHED_BUCKET__HH__