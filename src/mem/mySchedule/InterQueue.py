from m5.params import *
from m5.SimObject import SimObject



class InterQueue(SimObject):
    type       = 'InterQueue'
    cxx_header = 'mem/mySchedule/inter_queue.hh'
    cxx_class  = 'gem5::memory::InterQueue'

    # queue_sched_policy = Param.iterQSched('single', 'queue selector policy')
