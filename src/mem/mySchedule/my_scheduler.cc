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

        ContexParam                            minRR       = INT_MAX;
        ContexParam                            minALL      = INT_MAX;
        
        gem5::memory::MemPacketQueue::iterator minRR_iter  = queue.end();
        gem5::memory::MemPacketQueue::iterator minALL_iter = queue.end();
        
        for (auto memIter = queue.begin(); memIter != queue.end(); memIter++ ){
            // convert to memory packet
            gem5::memory::MemPacket* memPkt = *memIter;
            //context  id 
            ContexParam cpr = memPkt->pkt->schedulerCID;
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
        }
        
        if (minRR != INT_MAX){
            // do next context first
            DPRINTF(scheduler,"now we select next cid %d\n", minRR);
            lastRR = minRR;
            return minRR_iter;
        }else if (minALL != INT_MAX){
            // if not switch back to first lowest context number
            DPRINTF(scheduler,"now we have loop to start cid %d\n", minALL);
            lastRR = minALL;
            assert(minALL_iter != queue.end());
            return minALL_iter;
        }else{
            // In case there are no mem packets to schedule
            DPRINTF(scheduler,"there are no things to select ");
            return queue.end();
        }
    }



}
}

