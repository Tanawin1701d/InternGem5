from m5.params import *
from m5.objects.SimObject import SimObject


class myCache(SimObject):
    type = 'myCache'
    cxx_header = 'learning_gem5/myPractice/myPractice.hh'
    cxx_class  = 'gem5::myCache'

    i_cpu_slave_port  = ResponsePort("receieve data from cpu side port")
    d_cpu_slave_port  = ResponsePort("receieve data from cpu side port")
    mem_master_port = RequestPort("request to cross bar")
    hit_cycle       = Param.Cycles(1,"cycle that cache use to process event")
    #size            = Param.memorySize('1MB', "size of cache")
