#include <set>
#include "my_scheduler.hh"


namespace gem5
{
namespace mySchedule
{

    myScheduler::myScheduler(const mySchedulerParams &p) : SimObject(p)
    {

    }

    gem5::memory::MemPacketQueue::iterator
    myScheduler::chooseRoundRubin(gem5::memory::MemPacketQueue& queue) {
        // gem5::memory::MemPacket* memPkt = *(queue.begin());
        //     //DPRINTF(scheduler, "%p \n" ,queue.front()->pkt->req);
        // if (  (memPkt->pkt->req != shared_ptr<Request>(nullptr))
        // && (memPkt->pkt->req->hasContextId()) )
        //     DPRINTF(scheduler, "%d \n" ,queue.front()->pkt->req->contextId());
        // else
        //     DPRINTF(scheduler, "there is no contextId \n");
        // DPRINTF(scheduler, "pass pkt\n");
        //gem5::memory::MemPacket* memPkt = *(queue.begin());
        //DPRINTF(scheduler,"%d\n",memPkt->pkt->schedulerCID);

        CPUINDEXParam                            minRR       = INT_MAX;
        CPUINDEXParam                            minALL      = INT_MAX;
        
        gem5::memory::MemPacketQueue::iterator minRR_iter  = queue.end();
        gem5::memory::MemPacketQueue::iterator minALL_iter = queue.end();

        CPUINDEXParam                            readyNumNext= INT_MAX;
        CPUINDEXParam                            readyNumTurn= INT_MAX;
        
        for (auto memIter = queue.begin(); memIter != queue.end(); memIter++ ){
            // convert to memory packet
            gem5::memory::MemPacket* memPkt = *memIter;
            //context  id
             
            CPUINDEXParam cpr = memPkt->pkt->req->cpuId;

            // if (memPkt->pkt->fromNetwork){
            //     DPRINTF(passingTest,  "this come from network=============== %d\n", cpr );
            // }else{
            //     DPRINTF(passingTest,  "this come from cpu contextId %d\n", cpr );
            // }
            



            readyNumTurn = std::min(readyNumTurn, cpr);
            if( (cpr > lastRR) && (cpr < readyNumNext) ){
                readyNumNext = cpr;
            }
            ///////////////////// this session is for packet ready
            // for larger context ID that have lowest context id
            if (owner->packetReady(memPkt)){
                if ((cpr > lastRR) && (cpr < minRR)){
                    minRR_iter = memIter;
                    minRR      = cpr;
                }
                if (cpr < minALL){
                    minALL_iter = memIter;
                    minALL     = cpr;
                }
            }
            //////////////////////////////////////////////////////////
        }
        
        if ( readyNumNext != INT_MAX ){
            // do next context first
            if (minRR == readyNumNext){
                DPRINTF(scheduler,"now we select next cid %d\n", minRR);
                lastRR = minRR;
                return minRR_iter;
            }else{
                DPRINTF(scheduler,"there are next thread that want to schedule but it consequent thread can't issue request\n");
                return queue.end();
            }
        }else if ( (readyNumTurn <= lastRR ) && (readyNumTurn != INT_MAX) ){
            // if not switch back to first lowest context number
            if (minALL == readyNumTurn){
                DPRINTF(scheduler,"now we have loop to start cid %d\n", minALL);
                lastRR = minALL;
                //assert(minALL_iter != queue.end());
                return minALL_iter;
            }else{
                DPRINTF(scheduler,"there are loop back thread that want to schedule but it can't issue request\n");
                return queue.end();
            }
        }else{
            // In case there are no mem packets to schedule
            DPRINTF(scheduler,"there are no things to select \n");
            return queue.end();
        }
    }

    gem5::memory::MemPacketQueue::iterator
    myScheduler::chooseFNFRFCFS(gem5::memory::MemPacketQueue& queue, Tick min_col_at){
        gem5::memory::MemPacketQueue::iterator ret = queue.end();
        ret = dram->chooseFNFRFCFS(queue, min_col_at, true).first;
        if (ret != queue.end())
            return ret;
        return dram->chooseFNFRFCFS(queue, min_col_at, false).first;
    }
}
}

