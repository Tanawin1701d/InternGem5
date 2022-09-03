#ifndef __MEM_STAGESCHED_BUCKET__HH__
#define __MEM_STAGESCHED_BUCKET__HH__

#include"base/types.hh"
#include"base/logging.hh"
#include"sim/cur_tick.hh"
#include "enums/SMS_PushPol.hh"
#include "enums/SMS_PopPol.hh"
#include<vector>
#include<queue>
#include<unordered_map>


namespace gem5::memory{


    class Stages;
    class MemPacket;

    typedef uint64_t BATCHID;
    typedef std::deque<MemPacket*> MemPacketQueue;
    struct Batch{
                    Tick                    firstAddedTime = 0;
                    bool                    isBatchReady   = false;
                    MemPacketQueue          dayta;
    };

    class Bucket{
        private:
              Stages*                            owner;
        const Tick                               FORMATION_THRED;
              uint64_t                           curSize;
              uint64_t                           maxSize;
              enums::SMS_PushPol                 pushPol;
              enums::SMS_PopPol                  popPol;
              std::unordered_map<BATCHID, Batch> batchMap;
              std::deque        <BATCHID>        batchOrder;

        public:
        uint64_t size() const {return curSize;}
        uint64_t get_maxSize() const { return maxSize; }
        bool empty() { return curSize == 0; }
        bool canPush(uint64_t neededEntry);
        bool canPop();
        bool canMerge(Addr addr, unsigned  size);
        void push(MemPacket* mpkt);
        MemPacket* pop();
        MemPacket* front();
        void updateBatchStatus(); // update last batch is reach the thredshold
        bool isRowHit(MemPacket* a, MemPacket* b);
        void clear();
        Bucket(uint64_t           _maxSize, 
               enums::SMS_PushPol _pushPol,
               enums::SMS_PopPol  _popPol,
                    Tick          _FORMATION_THRED,
                    Stages*       _owner
              );

    };

}

#endif //__MEM_STAGESCHED_BUCKET__HH__