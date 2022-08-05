from m5.params import *
from m5.SimObject import SimObject


class InterQueue(SimObject):
    type       = 'InterQueue'
    abstract   = True
    cxx_header = 'mem/mySchedule/inter_queue.hh'
    cxx_class  = 'gem5::memory::InterQueue'
    mc         = Param.MemCtrl(NULL, "memory ctrl owner")



# class SimpleQueue(InterQueue):
#     type       = 'SimpleQueue'
#     cxx_header = 'mem/mySchedule/inter_queue.hh'
#     cxx_class  = 'gem5::memory::SimpleQueue'

# class ALGO_WF_Queue(InterQueue):
#     type       =  'ALGO_WF_Queue'
#     cxx_header = 'mem/mySchedule/inter_queue.hh'
#     cxx_class  = 'gem5::memory::ALGO_WF_Queue'

# class ALGO_NETQ_Queue(InterQueue):
#     type         = 'ALGO_NETQ_Queue'
#     cxx_header   = 'mem/mySchedule/inter_queue.hh'
#     cxx_class    = 'gem5::memory::ALGO_NETQ_Queue'
#     NetAwareThds = Param.Latency("10ns", "qos for network packet")

class STAGE_SCHED_Queue(InterQueue):
     type              =  "STAGE_SCHED_Queue"
     cxx_header        =  "mem/mySchedule/inter_queue.hh"
     cxx_class         =  "gem5::memory::STAGE_SCHED_Queue"

     tt_lotto          = Param.Unsigned(100, "amount that loto that was given to all policy")
     sjf_lotto         = Param.Unsigned(50 , "amount that loto that was given to sjf policy")

     stage1_amtSrc            = Param.Unsigned(4, "number of source (amount of buckets per read/write in stage1)")     
     stage1_sizePerSrc        = Param.Unsigned(64, "size of src buffer")
     stage3_amtBank           = Param.Unsigned(8, "amount of bank that stage3 requires")
     stage3_sizePerBank       = Param.Unsigned(64, "size of bank buffer")

     stage2_TFDELAY           = Param.Latency("1ns", "time to transfer to stage3")
     stage1_FORMATION_THRED   = Param.Latency("10ns", "max time that start batch wait for batch formation")
