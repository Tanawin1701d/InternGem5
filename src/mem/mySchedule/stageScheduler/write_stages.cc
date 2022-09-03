#include "params/WriteStages.hh"
#include "write_stages.hh"
#include "mem/mem_ctrl.hh"
#include "mem/mySchedule/stageScheduler/inter_stage.hh"

namespace gem5::memory{

//////////writeStageMetaData declaration////////////////////
bool 
WriteStages::serveByWriteQueue(Addr addr, 
                               unsigned size,
                               MemPacketQueue& srcToFind){
        for(MemPacket* mpkt: srcToFind){
                if ( (mpkt->addr <= addr) + 
                     ((addr +size) <= (mpkt->addr + mpkt->size)) 
                   )
                {
                        return true;
                }
        }
        return false;

}


bool 
WriteStages::serveByWriteQueue(Addr addr, unsigned size){
        for (auto& stage1_buck : stage1Data){
                if (stage1_buck.canMerge(addr, size)){
                        //owner.algo_stats.serveByWriteQ++;
                        return true;
                }
        }
        for (auto& stage3_q : stage3Data){
                if (serveByWriteQueue(addr, size, stage3_q)){
                        //owner.algo_stats.serveByWriteQ++;
                        return true;
                }
        }
        return false;
}

bool
WriteStages::exceed(){
        for (Bucket& bc : stage1Data){
                if (((float)bc.size()) >= ((float)exceed_thredshold*bc.get_maxSize()/100)   ){
                        return true;
                }
        }
        return false;
}

// WriteStages::WriteStages( const WriteStagesParams& ppc):
// {}

}