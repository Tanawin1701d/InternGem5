from m5.params import *
from m5.SimObject import SimObject



class InterQueue(SimObject):
    type       = 'InterQueue'
    abstract   = True
    cxx_header = 'mem/mySchedule/inter_queue.hh'
    cxx_class  = 'gem5::memory::InterQueue'


class SimpleQueue(InterQueue):
    type       = 'SimpleQueue'
    cxx_header = 'mem/mySchedule/inter_queue.hh'
    cxx_class  = 'gem5::memory::SimpleQueue'

class ALGO_WF_Queue(InterQueue):
    type       =  'ALGO_WF_Queue'
    cxx_header = 'mem/mySchedule/inter_queue.hh'
    cxx_class  = 'gem5::memory::ALGO_WF_Queue'

class ALGO_NETQ_Queue(InterQueue):
    type         = 'ALGO_NETQ_Queue'
    cxx_header   = 'mem/mySchedule/inter_queue.hh'
    cxx_class    = 'gem5::memory::ALGO_NETQ_Queue'
    NetAwareThds = Param.Latency("10ns", "qos for network packet")

class STAGE_SCHED_Queue(InterQueue):
    type         =  "STAGE_SCHED_Queue"
    cxx_header   =  "mem/mySchedule/inter_queue.hh"
    cxx_class    =  "gem5::memory::STAGE_SCHED_Queue"
    numStages    = Param.Unsigned(4, "number of stage (typically n(cores+network pe))")
    rr_max_lotto = Param.Unsigned(50, "amount that loto that was given to round-rubin policy")
    max_lotto    = Param.Unsigned(100, "amount that loto that was given to round-rubin policy")
    maxReadStageSize = Param.Unsigned(128, "amount that loto that was given to round-rubin policy")
    maxWriteStageSize = Param.Unsigned(512, "amount that loto that was given to round-rubin policy")



    