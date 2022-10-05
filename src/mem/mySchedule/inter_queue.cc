
#include "inter_queue.hh"
#include "mem/mem_interface.hh"

//#include "debug/interQ.hh"

namespace gem5
{


namespace memory{



///interQueue
InterQueue::InterQueue(const InterQueueParams& p) : 
mctrl(p.mc),
sys(p.system),
SimObject(p)
{}



}

} 

