#ifndef __MEM_STAGESCHED_STAGE__HH__
#define __MEM_STAGESCHED_STAGE__HH__

#include"bucket.hh"
#include "params/Stages.hh"
#include "params/WriteStages.hh"
#include "sim/sim_object.hh"
#include "base/types.hh"



namespace gem5::memory{


//predeclaration

typedef uint8_t QUEUEID;

class InterStage; // sms main

class   Stages : public SimObject{
            public:
            enum STAGE2_STATE  {pick, drain};
            enum STAGE2_PICK   {rr  , sjf  };
            STAGE_MEM_SCHED*             owner;
            //stageG
            std::vector<uint64_t>        stageGSize; // use to track sjf
            //stage1
            uint64_t                     stage1PktCount;
            QUEUEID                      stage1AMTBUCKET;
            std::vector<Bucket>          stage1Data;
            Tick                         stage1_FORMATION_THRED;
            //stage2
            uint32_t                     TOTALLOTO;
            uint32_t                     SJFLOTTO;
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
            bool                        canPush(QUEUEID qid, uint64_t neededEntry);
            void                        pushToQueues(MemPacket* mpkt);
            void                        updateAllBatchStatus();
            //stage2
            STAGE2_PICK                 genlotto();
            void                        processStage2Event();
            //stage3
            bool                        stage3full(uint8_t bankNum, uint64_t  reqEntry = 1);
            std::pair<MemPacket*, bool> chooseToDram();
            //debuger
            [[maybe_unused]]void        printStage(std::vector<MemPacketQueue>& stageRef);
            [[maybe_unused]]void        printStage(std::vector<BucketMeta>& stageRef);
            //constructor
                                        Stages( StagesParams& p );
            
        };
        
        class WriteStages : public Stages{
            public:
            bool                        serveByWriteQueue(Addr addr, unsigned size);
            bool                        serveByWriteQueue(Addr addr, unsigned size, MemPacketQueue& srcToFind);
                                        writeStages( WriteStagesParams& p);
        };

}

#endif // __MEM_STAGESCHED_STAGE__HH__