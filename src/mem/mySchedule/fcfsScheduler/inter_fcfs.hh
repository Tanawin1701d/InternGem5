#ifndef __MEM_FCFSSCHED_SMS__HH__
#define __MEM_FCFSSCHED_SMS__HH__

#include "params/InterFcfs.hh"
#include "mem/mySchedule/inter_queue.hh"

namespace gem5{

namespace memory{




class InterFcfs : public InterQueue{
        public:

        MemPacketQueue readSide;
        MemPacketQueue writeSide;
        int            qmSize;

        bool 
        readQueueFull (unsigned int pkt_count, uint8_t subQueueId = 0) override;
        bool 
        writeQueueFull(unsigned int pkt_count, uint8_t subQueueId = 0) override;
        void 
        pushToQueues(MemPacket* mpkt, bool isRead) override;

        [[maybe_unused]]
        std::vector<MemPacketQueue*>
        getQueueToSelect( bool  read ) override;
        
        std::pair<MemPacket*, bool>
        chooseToDram(bool is_read) override;

        bool 
        serveByWriteQueue(Addr addr, unsigned size) override;

        qos::MemCtrl::BusState 
        turnpolicy(qos::MemCtrl::BusState current_state) override;

        bool
        isWriteEmpty() override;
        bool
        isReadEmpty() override;
        bool
        writeStageExceed() override;

        void 
        notifyWriteBlockOccur(){return;};
        //////////////////
        public:
            InterFcfs(const InterFcfsParams &p);

    };

}


}


#endif //__MEM_STAGESCHED_BUCKET__HH__