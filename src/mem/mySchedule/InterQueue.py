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