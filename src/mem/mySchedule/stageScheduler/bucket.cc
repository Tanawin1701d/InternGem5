#include"bucket.hh"
#include"mem/mem_ctrl.hh"
#include"stages.hh"


namespace gem5::memory{

uint64_t
Bucket::get_batchSize(BATCHID bid){
        assert(batchMap.find(bid) != batchMap.end());
        return batchMap[bid].dayta.size();
}

bool
Bucket::canPush(uint64_t  neededEntry){
        return (curSize + neededEntry) <= maxSize;
}

bool 
Bucket::canPop(){

        if (popPol == enums::SMS_PopPol::SMS_ppFIFO){
                return (curSize > 0) ? batchMap[batchOrder.front()].isBatchReady : false;
        }else if (popPol == enums::SMS_PopPol::SMS_FRFCFS){
                //TODO for now overtake is not implemented
                panic("FRFCFS injection stage1 policy is not implemented");
        }else{
                panic("invalid pop  policy");
        }

}

bool
Bucket::canMerge(Addr addr, unsigned size){
       
        for (BATCHID bid: batchOrder){
                assert( batchMap.find(bid) != batchMap.end() );
                for (MemPacket* mpkt : batchMap[bid].dayta){
                        if ( (addr >= mpkt->addr)&&( (addr+size) <= (mpkt->addr + mpkt->size) ) ){
                                return true;
                        }
                }
        }
        return false;
}

void
Bucket::push(MemPacket* mpkt){
        //owner->owner.algo_stats.diffPushTime.sample(mpkt->queueAddedTime - lastAdded);
        //timestamp of the packet
        mpkt->queueAddedTime = curTick();
        //update if batch is exceed formation thredshold
        updateBatchStatus();
        /// push mempkt
        //things must consider
        // 1.batch order
        // 2.batch map
        // 3.cursize
        // 4. mpkt batch id
        if (pushPol == enums::SMS_PushPol::SMS_phFIFO){
                if ( (curSize == 0) || 
                     batchMap[batchOrder.back()].isBatchReady ||
                     !isRowHit(batchMap[batchOrder.back()].dayta.back(), mpkt )
                   ){
                        //TODO I forgot to update batch in-case different row
                        if (curSize){
                                batchMap[batchOrder.back()].isBatchReady = true;
                        }
                        // we start new batch
                        BATCHID newBid = (curSize) ? batchOrder.back()+1 : 0;
                        batchOrder.push_back(newBid);
                        mpkt->batchId = newBid;
                        //construct new batch
                        Batch newStartBatch;
                        newStartBatch.firstAddedTime = curTick();
                        newStartBatch.dayta.push_back(mpkt);
                        batchMap.insert({newBid, newStartBatch});
                        //stat
                        owner->stage_stats.startNewBatch[bucketId]++;

                }else{
                        // we exploit batch
                        assert(!batchOrder.empty());
                        batchMap[batchOrder.back()].dayta.push_back(mpkt);
                        mpkt->batchId = batchOrder.back();
                        //stat
                        owner->stage_stats.exploitBatch[bucketId]++;

                }
                // update size of the bucket
                //update size
                curSize++;
                assert(curSize <= maxSize);
        }else if (pushPol == enums::SMS_PushPol::SMS_OVERTAKE){
                // TODO
                panic("OVERTAKE pushing stage1 is not implemented");
        }else{
                panic("invalid push policy");
        }

}

MemPacket*
Bucket::pop(){
        assert(curSize > 0);

        //things must consider
        // 1.batch order
        // 2.batch map
        // 3.cursize
        // 4.dont forgot to return 

        if (popPol == enums::SMS_PopPol::SMS_ppFIFO){

                //select batch
                BATCHID sel_bid = batchOrder.front();
                assert(batchMap[sel_bid].isBatchReady);
                //get mempacket
                MemPacket* mpkt = batchMap[sel_bid].dayta.front();
                batchMap[sel_bid].dayta.pop_front();
                curSize--;
                assert(mpkt != nullptr);
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
                return mpkt;

        }else if (popPol == enums::SMS_PopPol::SMS_FRFCFS){
                // TODO
                panic("frfcfs pop stage1 is not implemented");
        }else{
                panic("invalid pop policy in front() call");
        }
}

MemPacket*
Bucket::front(){
        assert( ( !batchOrder.empty() ) && (batchMap.find(batchOrder.front()) != batchMap.end()));

        if (popPol == enums::SMS_PopPol::SMS_ppFIFO){
                return batchMap[batchOrder.front()].dayta.front();
        }else if (popPol == enums::SMS_PopPol::SMS_FRFCFS){
                panic("frfcfs pop stage1 is not implemented");
        }else{
                panic("invalid pop policy on front() call ");
        }
}


void 
Bucket::updateBatchStatus(){
        // for now batch update status only concern last batch also for push policy OVERTAKE will dont care isBactchready variable
        //this is used to make batch ready when last batch reach the thredshold
        if (curSize == 0){
                assert(batchOrder.empty());
        }else{
                assert(!batchOrder.empty());
                if ( (curTick() - batchMap[batchOrder.back()].firstAddedTime) >= FORMATION_THRED){
                        //TODOFIX state maybe change
                        //owner->owner.algo_stats.batchExpire++;
                        owner->stage_stats.batchExpire[bucketId]++;
                        //stat
                        batchMap[batchOrder.back()].isBatchReady = true;
                }
        }
}

bool 
Bucket::isRowHit(MemPacket* a, MemPacket* b){
        return (a->rank == b->rank) && (a->bank == b->bank) && (a->row == b->row);
}

void 
Bucket::clear(){
        curSize     =  0;
        batchOrder.clear();
        batchMap  .clear();
}

Bucket::Bucket(uint64_t           _maxSize, 
               enums::SMS_PushPol _pushPol,
               enums::SMS_PopPol  _popPol,
               Tick               _FORMATION_THRED,
               Stages*            _owner,
               QUEUEID            _bucketId

               ):
               maxSize        (_maxSize),
               pushPol        (_pushPol),
               popPol         (_popPol),
               FORMATION_THRED(_FORMATION_THRED),
               owner          (_owner),
               curSize        (0),
               bucketId       (_bucketId)
               {}


}