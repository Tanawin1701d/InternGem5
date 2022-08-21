#ifndef __MEM_STAGESCHED_BUCKET__HH__
#define __MEM_STAGESCHED_BUCKET__HH__

#include"base/types.hh"
#include"base/logging.hh"
#include"sim/cur_tick.hh"
#include<vector>
#include<queue>
#include<unordered_map>


namespace gem5::memory{


    class Stages;
    class MemPacket;

    typedef uint64_t BATCHID;

    struct Batch{
                    Tick                    firstAddedTime = 0;
                    bool                    isBatchReady   = false;
                    MemPacketQueue          dayta;
    };

    class Bucket{
        public:
              enum  pushPolicy {OLDSMS, OVERTAKE}; //old SMS vs overtake to exploit rowhit
        private:
              Stages*                            owner;
        const Tick                               FORMATION_THRED;
              uint64_t                           curSize;
              uint64_t                           maxSize;
              pushPolicy                         pushPol;
              std::unordered_map<BATCHID, Batch> batchMap;
              std::deque        <BATCHID>        batchOrder;

        public:
        bool empty() { return curSize == 0; }
        bool canPush(uint64_t neededEntry);
        bool canPop();
        void push(MemPacket* mpkt);
        MemPacket* pop();
        MemPacket* front();
        void updateBatchStatus(); // update last batch is reach the thredshold
        bool isRowHit(MemPacket* a, MemPacket* b);
        void clear();
        Bucket(uint64_t maxSize, 
               enum pushPolicy pushPol,
               Tick FORMATION_THRED,
               Stages* owner
              );

    };

}

#endif //__MEM_STAGESCHED_BUCKET__HH__