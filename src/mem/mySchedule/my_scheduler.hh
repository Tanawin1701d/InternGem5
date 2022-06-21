#ifndef __MEM_MYSCHEDULE_MY_SCHEDULER_HH__
#define __MEM_MYSCHEDULE_MY_SCHEDULER_HH__


#include "debug/scheduler.hh"
#include "debug/passingTest.hh"
#include "base/trace.hh"
#include "mem/mem_ctrl.hh"
#include "mem/mem_interface.hh"
#include "params/myScheduler.hh"
#include <set>
namespace gem5
{

namespace mySchedule
{


    typedef int CPUINDEXParam;


    class myScheduler : public SimObject
    {
      private:
        CPUINDEXParam  lastRR = INT_MIN;

      public:
        

        gem5::memory::MemCtrl* owner;
        gem5::memory::DRAMInterface* dram;

        myScheduler(const mySchedulerParams &p);

        gem5::memory::MemPacketQueue::iterator
        chooseRoundRubin(gem5::memory::MemPacketQueue& queue);

        gem5::memory::MemPacketQueue::iterator
        chooseFNFRFCFS(gem5::memory::MemPacketQueue& queue, Tick min_col_at);
    };
}

}
#endif //__MEM_MYSCHEDULE_MY_SCHEDULER_HH__
