#include"mpkc.hh"


namespace gem5 {

MPKC::MPKC():
miss_count(0),
current_MPKC(0)
{}


void
MPKC::updateMiss(){
    miss_count++;
}

void
MPKC::updateCycle(){
    if (cycle_count == 1000){
        current_MPKC = miss_count;
        miss_count     = 0;
        cycle_count    = 1;
    }
    cycle_count++;
}

uint64_t 
MPKC::getMPKC(){
    return current_MPKC;
}

}