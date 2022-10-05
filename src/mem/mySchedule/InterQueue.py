from m5.params import *
from m5.proxy import *
from m5.SimObject import SimObject


class InterQueue(SimObject):
    type       = 'InterQueue'
    abstract   = True
    cxx_header = 'mem/mySchedule/inter_queue.hh'
    cxx_class  = 'gem5::memory::InterQueue'

    mc         = Param.MemCtrl(NULL, "memory ctrl owner")
    system     = Param.System(Parent.any, "System that the controller belongs to.")
