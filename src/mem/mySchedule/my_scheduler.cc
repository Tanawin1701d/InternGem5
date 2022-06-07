
#include "mem/mySchedule/my_scheduler.hh"

namespace gem5
{
namespace mySchedule
{

    myScheduler::myScheduler(const mySchedulerParams &p) : SimObject(p)
    {

    }

    gem5::memory::MemPacketQueue::iterator
    myScheduler::chooseRoundRubin(gem5::memory::MemPacketQueue& queue) const{
        return queue.begin();
    }



}
}

