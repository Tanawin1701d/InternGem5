#ifndef __MEM_STAGESCHED_WRITE_STAGE__HH__
#define __MEM_STAGESCHED_WRITE_STAGE__HH__

//#include "params/WriteStages.hh"
#include "mem/mySchedule/stageScheduler/stages.hh"

namespace gem5{
    
namespace memory{


//predeclaration

typedef uint8_t QUEUEID;

        
        class WriteStages : public Stages{

            private:
            int                         exceed_thredshold;
            int                         lower_thredshold;    
            const Tick                  wr_cool_down_thred;
            Tick                        wr_cool_down_st; //write cooldown start time
            bool                        status_CoolDown;


            public:
            bool                        serveByWriteQueue(Addr addr, unsigned size);
            bool                        serveByWriteQueue(Addr addr, unsigned size, MemPacketQueue& srcToFind);
            bool                        exceed();
            bool                        lower();
            bool                        isExceedCD(); // is cool down exceed the thredshold
            void                        startOc();// start to let stage3 occupy the dram 
            bool                        shouldCoolDownStop() const;
            bool                        isCoolDownStarted() const;
            void                        stopOc(); // stop to let stage3 release resource

            WriteStages( const WriteStagesParams& ppc) : 
            exceed_thredshold(ppc.exceed_thredshold), 
            lower_thredshold(ppc.lower_thredshold),
            wr_cool_down_thred(ppc.wr_cool_down_thred),
            wr_cool_down_st(0),
            status_CoolDown(false)
            ,Stages( ppc){};

        };

    }
}

#endif // __MEM_STAGESCHED_WRITE_STAGE__HH__