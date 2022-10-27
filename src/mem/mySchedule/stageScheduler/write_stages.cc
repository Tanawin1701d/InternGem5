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
                        stage_stats.serveByWriteQ++;
                        return true;
                }
        }
        for (auto& stage3_q : stage3Data){
                if (serveByWriteQueue(addr, size, stage3_q)){
                        //owner.algo_stats.serveByWriteQ++;
                        stage_stats.serveByWriteQ++;
                        return true;
                }
        }
        return false;
}

bool
WriteStages::exceed(){
        int bucketCount = 0;
        for (Bucket& bc : stage1Data){
                if (((float)bc.size()) >= ((float)exceed_thredshold*bc.get_maxSize()/100)   ){
                        stage_stats.exceedStage1[bucketCount]++;
                        return true;
                }
                bucketCount++;
        }
        return false;
}

bool
WriteStages::lower(){
        // this will ensure all bucket have current size lower than thred
        int bucketCount = 0;
        for (Bucket& bc : stage1Data){
                if (((float)bc.size()) >= ((float)lower_thredshold*bc.get_maxSize()/100)   ){
                        stage_stats.lowerStage1[bucketCount]++;
                        return false;
                }
                bucketCount++;
        }
        return true;
}

void
WriteStages::startOc(){
        panic_if(status_CoolDown, "stage scheduling policy intend to start new"
                                  " cooldown while previoused time slot is not stopping.");
        
        status_CoolDown = true;
        wr_cool_down_st = curTick();

}

bool
WriteStages::shouldCoolDownStop() const{
        return curTick() - wr_cool_down_st >= wr_cool_down_thred;

}


bool
WriteStages::isCoolDownStarted() const{
        return status_CoolDown;
}

void
WriteStages::stopOc(){
        panic_if(!status_CoolDown, "stage scheduling policy intend to stop cooldown"
                                        " while previoused time slot is not started.");
        status_CoolDown = false;
}

// WriteStages::WriteStages( const WriteStagesParams& ppc):
// {}

}