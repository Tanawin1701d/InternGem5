#ifndef __MPKI__HH__
#define __MPKI__HH__

#include <cstdint>

namespace gem5{

    // standfor miss per kilo instruction to allow memctrl access mpki value
    
    // mpki will be attach to base cpu to allow any cpus update instruction count
    class MPKC {

        private:
            uint64_t cycle_count;
            uint64_t miss_count;
            uint64_t current_MPKC;
        public:
            MPKC();
            void updateMiss(); // update miss by last level cache
            void updateCycle(); // this will be trigged by cpu
            uint64_t getMPKC(); // this memory controllers are used to calculate 

    };



}

#endif