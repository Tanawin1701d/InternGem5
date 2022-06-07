#ifndef __MEM_MYSCHEDULE_MY_SCHEDULER_HH__
#define __MEM_MYSCHEDULE_MY_SCHEDULER_HH__


#include "debug/scheduler.hh"
#include "mem/mem_ctrl.hh"
#include "params/myScheduler.hh"

namespace gem5
{

namespace mySchedule
{
    class myScheduler : public SimObject
    {
      public:
        gem5::memory::MemCtrl* owner;
        myScheduler(const mySchedulerParams &p);
        gem5::memory::MemPacketQueue::iterator
        chooseRoundRubin(gem5::memory::MemPacketQueue& queue) const;



    };
}

}
#endif //__MEM_MYSCHEDULE_MY_SCHEDULER_HH__
