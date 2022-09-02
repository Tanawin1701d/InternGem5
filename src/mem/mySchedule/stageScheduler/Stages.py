from multiprocessing.util import abstract_sockets_supported
from m5.params               import *
from m5.SimObject            import SimObject
#from m5.objects.InterStage   import *


class Stages(SimObject):
    type       = "Stages"
    cxx_header = "mem/mySchedule/stageScheduler/stages.hh"
    cxx_class  = "gem5::memory::Stages"
    
    st1_size_per_src      = Param.UInt64(1024, "size of per bucket src")
    st1_amt_src           = Param.UInt8(1, "amount of src")
    st1_formation_thred   = Param.Tick(40, "amount of time for batch formation")

    
    st2_tt_lotto          = Param.UInt32(100, "amount that loto that was given to all policy")
    st2_sjf_lotto         = Param.UInt32(50, "amount that loto that was given to sjf")
    st2_tf_dl             = Param.Tick  (1, "time to transfer from stage1 to stage3 per 1 memreq")

    st3_size_per_bank     = Param.UInt64(1024, "size of per bank queue")
    st3_amt_bank          = Param.UInt8 (8, "amount of bank")

    #owner                 = Param.InterStage(NULL, "interstage that control read and write queue")

    def init(self,nc, ow):
        self.st1_amt_src = nc
        #self.owner       = ow


class WriteStages(Stages):
    type       = "WriteStages"
    cxx_header = "mem/mySchedule/stageScheduler/write_stages.hh"
    cxx_class  = "gem5::memory::WriteStages"


    