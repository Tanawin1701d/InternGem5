#include"bucket.hh"
#include"mem/mem_ctrl.hh"


namespace gem5::memory{

bool
Bucket::canPush(uint64_t  neededEntry){
        return (curSize + neededEntry) <= maxSize;
}

bool 
Bucket::canPop(){

        if (pushPol == OLDSMS){
                return (curSize > 0) ? batchMap[batchOrder.front()].isBatchReady : false;
        }else if (pushPol == OVERTAKE){
                //TODO for now overtake is not implemented
                panic("OVERTAKE injection stage1 policy is not implemented");
        }else{
                panic("invalid inject  policy");
        }

}

void
Bucket::push(MemPacket* mpkt){
        //owner->owner.algo_stats.diffPushTime.sample(mpkt->queueAddedTime - lastAdded);
        //timestamp of the packet
        mpkt->queueAddedTime = curTick();
        //update if batch is exceed formation thredshold
        updateBatchStatus();

        /// push mempkt
        if (pushPol == OLDSMS){
                if ( (curSize == 0) || batchMap[batchOrder.back()].isBatchReady){
                        // we start new batch
                        BATCHID newBid = (curSize) ? batchOrder.back()+1 : 0;
                        batchOrder.push_back(newBid);
                        mpkt->batchId = newBid;
                        Batch newStartBatch;
                        newStartBatch.dayta.push_back(mpkt);
                        batchMap.insert({newBid, newStartBatch});
                }else{
                        // we exploit batch
                        assert(!batchOrder.empty());
                        batchMap[batchOrder.back()].dayta.push_back(mpkt);
                        mpkt->batchId = batchOrder.back();
                }
                // update size of the bucket
                //update size
                curSize++;
        }else if (pushPol == OVERTAKE){
                // TODO
                panic("OVERTAKE pushing stage1 is not implemented");
        }else{
                panic("invalid push policy");
        }

}

MemPacket*
Bucket::pop(){
        assert(curSize > 0);

        if (pushPol == OLDSMS){

                //select batch
                BATCHID sel_bid = batchOrder.front();
                assert(batchMap[sel_bid].isBatchReady);
                //get mempacket
                MemPacket* mpkt = batchMap[sel_bid].dayta.front();
                batchMap[sel_bid].dayta.pop_front();
                curSize--;
                //get remain batch size
                uint64_t rem_batch_size = batchMap[sel_bid].dayta.size();
                assert(rem_batch_size >= 0);
                //update state
                if ( rem_batch_size > 0){
                        //pass
                }else{
                        batchOrder.pop_front();
                        batchMap.erase(sel_bid);
                }

        }else if (pushPol == OVERTAKE){
                // TODO
                panic("OVERTAKE pop stage1 is not implemented");
        }else{
                panic("invalid push policy");
        }
}

MemPacket*
Bucket::front(){
        assert( ( !batchOrder.empty() ) && (batchMap.find(batchOrder.front()) != batchMap.end()));
        return batchMap[batchOrder.front()].dayta.front();
}


void 
Bucket::updateBatchStatus(){
        //this is used to make batch ready when last batch reach the thredshold
        if (curSize == 0){
                assert(batchOrder.empty());
        }else{
                assert(!batchOrder.empty());
                if ( (curTick() - batchMap[batchOrder.back()].firstAddedTime) >= FORMATION_THRED){
                        //TODOFIX state maybe change
                        //owner->owner.algo_stats.batchExpire++;
                        batchMap[batchOrder.back()].isBatchReady = true;
                }
        }
}

bool 
isRowHit(MemPacket* a, MemPacket* b){
        return (a->rank == b->rank) && (a->bank == b->bank) && (a->row == b->row);
}

void 
Bucket::clear(){
        curSize     =  0;
        batchOrder.clear();
        batchMap  .clear();
}

Bucket::Bucket(uint64_t maxSize, 
               enum pushPolicy pushPol,
               Tick FORMATION_THRED,
               Stages* owner               
               ):
               maxSize(maxSize),
               pushPol(pushPol),
               FORMATION_THRED(FORMATION_THRED),
               owner(owner),
               curSize(0)
               {}


}