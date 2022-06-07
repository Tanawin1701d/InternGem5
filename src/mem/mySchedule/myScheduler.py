from m5.params import *
from m5.SimObject import SimObject



class myScheduler(SimObject):
    type = 'myScheduler'
    cxx_header = "mem/mySchedule/my_scheduler.hh"
    cxx_class = 'gem5::mySchedule::myScheduler'
