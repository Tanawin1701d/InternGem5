#ifndef __MEM_STAGESCHED_STAGE__HH__
#define __MEM_STAGESCHED_STAGE__HH__

#include <vector>
#include"bucket.hh"
#include "params/Stages.hh"
#include "params/WriteStages.hh"
#include "sim/sim_object.hh"
#include "enums/SMS_STAGE2_PICK.hh"
#include "base/statistics.hh"
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
            //stats
            struct Stages_Stats : public statistics::Group
            {
                Stages_Stats(Stages& ITQ);
                void regStats() override;
                Stages& stages_owner;

                statistics::Scalar    selectedByRR;
                statistics::Scalar    selectedBySJF;
                statistics::Scalar    selectedByS1MF;
                statistics::Scalar    dramChooseHit;
                statistics::Scalar    dramChoosemiss;
                statistics::Scalar    amountPkt;
                statistics::Scalar    amountBypass;
                statistics::Scalar    BypassExceedLim;
                statistics::Scalar    BypassExceedMPKC;
                statistics::Scalar    BypassStage3Lim;
                statistics::Scalar    serveByWriteQ;
                statistics::Vector    batchExpire;
                statistics::Vector    exceedStage1;
                statistics::Vector    exploitBatch;
                statistics::Vector    startNewBatch;
                statistics::Histogram batchedSize;
                statistics::Histogram selStage3_hitDBG;
                statistics::Histogram selStage3_missDBG;
                statistics::Histogram MPKC_val;
                
                //statistics::Histogram diffPushTime;
                // statistics::Histogram maxSizeReadQueue;


            };
            Stages_Stats stage_stats;
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
            uint64_t                     stage3BypassMPKC_thred;
            uint64_t                     stage3BypassLim;
            uint64_t                     stage3PktCount;
            uint8_t                      stage3AmtBank;
            std::vector<uint64_t>        stage3Size; // maxsize
            std::vector<MemPacketQueue>  stage3Data;

            //stageG // global state
            bool                        empty();
            bool                        shouldBypass( unsigned int pkt_count,
                                                      uint8_t      subQueueId,  
                                                      uint64_t     mpkc_val
                                                    );
            //stage1
            bool                        canPush(QUEUEID qid, uint64_t neededEntry);
            void                        pushToQueues(MemPacket* mpkt);
            void                        updateAllBatchStatus();
            //stage2
            enums::SMS_STAGE2_PICK      genlotto();
            void                        processStage2Event();
            //stage3
            bool                        stage3full(uint8_t bankNum, uint64_t  reqEntry = 1);
            void                        pushToQueuesBypass(MemPacket* mpkt);
            std::pair<MemPacket*, bool> chooseToDram();
            //debuger
            [[maybe_unused]]void        printStage(std::vector<MemPacketQueue>& stageRef);
            [[maybe_unused]]void        printStage(std::vector<Bucket>& stageRef);
            //constructor
            Stages( const StagesParams& p );
            
        };

    }
}

#endif //__MEM_STAGESCHED_STAGE__HH__