


#ifndef LEARNING_GEM5_MYCACHE
#define LEARNING_GEM5_MYCACHE

#include "mem/port.hh"
#include "params/myCache.hh"
#include "sim/sim_object.hh"
#include "sim/eventq.hh"
#include "base/types.hh"
#include "debug/myCache.hh"

namespace gem5{

class myCache : public SimObject{

    public:
        class retrievePort : public ResponsePort{
            private:
                myCache*  myCacheOwner;
                PacketPtr blockedPacket;
                bool      needRetry; // need to send retry req
                bool      isInstr;
            public:
                retrievePort(const std::string& name, myCache* owner, bool isInstr);
                bool recvTimingReq(PacketPtr pkt);
                void recvRespRetry();
                bool sendData(PacketPtr pkt);
                void retry();
                //////////////////////////////////////////////
                AddrRangeList getAddrRanges() const;
                Tick recvAtomic(PacketPtr pkt);
                void recvFunctional(PacketPtr pkt);
        };
        class sendPort : public RequestPort{
            private:
                PacketPtr blockedPacket;
                myCache* myCacheOwner;   
            public:
                sendPort(const std::string& name, myCache* owner);
                bool recvTimingResp(PacketPtr pkt);
                bool sendData(PacketPtr pkt);
                void recvReqRetry();
                
        };

        retrievePort i_cpu_slave_port;
        retrievePort d_cpu_slave_port;

        sendPort     mem_master_port;

        EventFunctionWrapper event;
    
        PacketPtr            inComputePacket;
        bool                 isInstr;
        bool                 isWaitingDram;
        myCache(const myCacheParams& classParam);
        ~myCache();

        bool handleReq(PacketPtr recvPkt, bool isInstr);
        void handleRes(PacketPtr recvPkt);
        void processEvent();

        Port &
        getPort(const std::string &if_name, PortID idx);
        
        
        
};

}


#endif // LEARNING_GEM5_MYCACHE