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

            public:
            bool                        serveByWriteQueue(Addr addr, unsigned size);
            bool                        serveByWriteQueue(Addr addr, unsigned size, MemPacketQueue& srcToFind);
            bool                        exceed();
            bool                        lower();
            WriteStages( const WriteStagesParams& ppc) : 
            exceed_thredshold(ppc.exceed_thredshold), 
            lower_thredshold(ppc.lower_thredshold)
            ,Stages( ppc){};

        };

    }
}

#endif // __MEM_STAGESCHED_WRITE_STAGE__HH__