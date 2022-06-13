#include "learning_gem5/myPractice/myPractice.hh"
#include "debug/myCache.hh"
#include "base/trace.hh"

namespace gem5{

/////////////// retrieve port

myCache::retrievePort::retrievePort(const std::string& name, myCache* owner, bool isInstr)
            : ResponsePort(name, (SimObject*)owner), 
              myCacheOwner(owner),
              blockedPacket(nullptr),
              needRetry(false),
              isInstr(isInstr)
              {}

bool myCache::retrievePort::recvTimingReq(PacketPtr pkt){
    if ( !myCacheOwner->handleReq(pkt, isInstr) ){
        //DPRINTF(myCache, "receive packet and accept\n");
        needRetry = true;
        return false;
    }else{
        //DPRINTF(myCache, "receive packet and DECLINE\n");
        needRetry = false;
        return true;
    }
    
}

void myCache::retrievePort::recvRespRetry(){
    DPRINTF(myCache, "recvRespRetry\n");
    sendData(blockedPacket);
}

bool myCache::retrievePort::sendData(PacketPtr pkt){ 
    assert(pkt != nullptr);
            //DPRINTF(myCache, "send data back to responder\n");
    if (!sendTimingResp(pkt)){
        blockedPacket = pkt;
        return false;
    }
    blockedPacket = nullptr;
    return true;
}

void myCache::retrievePort::retry(){
    if (needRetry){
        sendRetryReq();
    }
}

AddrRangeList myCache::retrievePort::getAddrRanges() const{
    return myCacheOwner->mem_master_port.getAddrRanges();
}

Tick myCache::retrievePort::recvAtomic(PacketPtr pkt){
    panic("now we won't use atomic these so we use only timing");
}

void myCache::retrievePort::recvFunctional(PacketPtr pkt){
    //panic("now we won't use functional these so we use only timing");
    myCacheOwner->mem_master_port.sendFunctional(pkt);
}
/////////////////////////////////////////////////////////////

//////////////////////// sendport //////////////////////////////////////////////////////////

myCache::sendPort::sendPort(const std::string& name, myCache* owner):
    RequestPort(name, (SimObject*) owner), 
    blockedPacket(nullptr),
    myCacheOwner(owner)
    {}

bool myCache::sendPort::recvTimingResp(PacketPtr pkt){
    // need not to return false because every people wait for them
    myCacheOwner->handleRes(pkt);
    return true;
}

bool myCache::sendPort::sendData(PacketPtr pkt){
    if (!sendTimingReq(pkt)){
        blockedPacket = pkt;
        return false;
    }
    return true;
}

void myCache::sendPort::recvReqRetry(){
    assert(blockedPacket != nullptr);
    sendData(blockedPacket);
}
//////////////////////////////////////////////////////////////////

bool myCache::handleReq(PacketPtr pkt, bool isInstr){
    if (isWaitingDram){
        return false;
    }else{
        inComputePacket = pkt;
        this->isInstr = isInstr;
        isWaitingDram   = true;
        schedule(event, curTick()+1);
        return true;
    }
}

void myCache::handleRes(PacketPtr pkt){
    if (isInstr)
        i_cpu_slave_port.sendData(pkt);
    else{
        d_cpu_slave_port.sendData(pkt);
    }
    inComputePacket = nullptr;
    isWaitingDram = false;
    i_cpu_slave_port.retry();
    d_cpu_slave_port.retry();
    
}

void myCache::processEvent(){
    mem_master_port.sendData(inComputePacket);
}

Port & myCache::getPort(const std::string &if_name, PortID idx){
    if (if_name == "i_cpu_slave_port"){
        return i_cpu_slave_port;
    }else if (if_name == "d_cpu_slave_port"){
        return d_cpu_slave_port;
    }else if (if_name == "mem_master_port"){
        return mem_master_port;
    }else{
        return SimObject::getPort(if_name, idx);
    }
}

myCache::myCache(const myCacheParams& classParam):
    SimObject(classParam),
    event([this]{processEvent();}, "fffftgreg.event"),
    inComputePacket(nullptr),
    isWaitingDram(false),
    i_cpu_slave_port(classParam.name + ".i_cpu_slave_port", this, true),
    d_cpu_slave_port(classParam.name + ".d_cpu_slave_port", this, false),
    mem_master_port(classParam.name + ".mem_master_port", this)
    {}

myCache::~myCache(){}

}