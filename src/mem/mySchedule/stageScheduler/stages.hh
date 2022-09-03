#ifndef __MEM_STAGESCHED_STAGE__HH__
#define __MEM_STAGESCHED_STAGE__HH__

#include <vector>
#include"bucket.hh"
#include "params/Stages.hh"
#include "params/WriteStages.hh"
#include "sim/sim_object.hh"
#include "enums/SMS_STAGE2_PICK.hh"
//#include "base/types.hh"
#include "debug/SMS.hh"



namespace gem5{
    
namespace memory{


//predeclaration

typedef uint8_t QUEUEID;

class InterStage; // sms main

class   Stages : public SimObject{
            public:
            enum STAGE2_STATE  {pick, drain};
            InterStage*                  owner;
            //stageG
            std::vector<uint64_t>        stageGSize; // use to track sjf
            //stage1
            uint64_t                     stage1PktCount;
            QUEUEID                      stage1AMTBUCKET;
            std::vector<Bucket>          stage1Data;
            Tick                         stage1_FORMATION_THRED;
            //stage2
            uint32_t                     TOTALLOTTO;
            std::vector<uint32_t>        LOTTO;
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

            //stageG // global state
            bool                        empty();
            //stage1
            bool                        canPush(QUEUEID qid, uint64_t neededEntry);
            void                        pushToQueues(MemPacket* mpkt);
            void                        updateAllBatchStatus();
            //stage2
            enums::SMS_STAGE2_PICK      genlotto();
            void                        processStage2Event();
            //stage3
            bool                        stage3full(uint8_t bankNum, uint64_t  reqEntry = 1);
            std::pair<MemPacket*, bool> chooseToDram();
            //debuger
            [[maybe_unused]]void        printStage(std::vector<MemPacketQueue>& stageRef);
            [[maybe_unused]]void        printStage(std::vector<Bucket>& stageRef);
            //constructor
            Stages( const StagesParams& p );
            
        };

    }
}

#endif // __MEM_STAGESCHED_STAGE__HH__