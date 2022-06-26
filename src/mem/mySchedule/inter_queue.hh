#ifndef __MEM__QOS_INTER_QUEUE_HH___
#define __MEM__QOS_INTER_QUEUE_HH___

#include "sim/sim_object.hh"
#include "mem/mem_ctrl.hh"
#include "params/InterQueue.hh"
//#include "enums/iterQSched.hh"



namespace gem5
{

namespace memory
{

    class InterQueue : public SimObject
    {
        //enums::iterQSched interSchedPolicy;
        //virtual uint8_t                       qFillSel(std::vector<MemPacketQueue>* readQueues, std::vector<MemPacketQueue>* writeQueues, PacketPtr pkt ) = 0;
        //virtual std::vector<MemPacketQueue*>  qSchedFill(std::vector<MemPacketQueue>* queues) = 0;
        virtual void test();

        //virtual uint8_t qFillSel(std::vector<MemPacketQueue>* readQueues, std::vector<MemPacketQueue>* writeQueues, PacketPtr pkt );

        public:
        InterQueue(const InterQueueParams &p);
    };



}
}


#endif //__MEM__QOS_INTER_QUEUE_HH___
