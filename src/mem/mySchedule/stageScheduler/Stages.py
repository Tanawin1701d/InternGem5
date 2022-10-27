from m5.params               import *
from m5.SimObject            import SimObject
#from m5.objects.InterStage   import *

class SMS_PushPol    (Enum) : vals = ['SMS_phFIFO', 'SMS_OVERTAKE']
class SMS_PopPol     (Enum) : vals = ['SMS_ppFIFO', 'SMS_FRFCFS'  ]
class SMS_STAGE2_PICK(Enum) : vals = ['SMS_rr'  , 'SMS_sjf', 'SMS_s1mf']

class Stages(SimObject):
    type       = "Stages"
    cxx_header = "mem/mySchedule/stageScheduler/stages.hh"
    cxx_class  = "gem5::memory::Stages"
    
    st1_size_per_src      = Param.UInt64(1024, "size of per bucket src")
    st1_amt_src           = Param.UInt8(1, "amount of src")
    st1_formation_thred   = Param.Latency('40ns', "amount of time for batch formation")
    st1_vec_pushPol       = VectorParam.SMS_PushPol('SMS_phFIFO', 'stage1 push policy for each bucket')
    st1_popPol            = Param.SMS_PopPol ('SMS_ppFIFO', 'stage1 pop  policy')
    
    st2_tt_lotto          = Param.UInt32(100, "amount that loto that was given to all stage2 pick policy")
    st2_vec_lotto         = VectorParam.UInt32([50,50,0],"vector of stage2 lotto rr sjf s1mf respectively")
    st2_tf_dl             = Param.Latency('1ns', "time to transfer from stage 1 to stage 3")

    st3_BypassMPKC_thred  = Param.UInt64(3 , "maximum mpkc value that allow mempacket bypass")
    st3_BypassLim         = Param.UInt64(16, "maximum mempkt that remain before bypassing allowed")
    st3_size_per_bank     = Param.UInt64(1024, "size of per bank queue")
    st3_amt_bank          = Param.UInt8 (16, "amount of bank")

    #owner                 = Param.InterStage(NULL, "interstage that control read and write queue")

    def initBuck(self,nc):
        self.st1_amt_src     = nc
        self.st1_vec_pushPol = ["SMS_phFIFO" for _ in range(nc)]
        #self.owner           = ow


class WriteStages(Stages):
    type       = "WriteStages"
    cxx_header = "mem/mySchedule/stageScheduler/write_stages.hh"
    cxx_class  = "gem5::memory::WriteStages"

    exceed_thredshold    = Param.Int32(90, "thredshold limit to prevent buccket full")
    lower_thredshold     = Param.Int32(35, "min thredshold to ensure writeSide will not play ping-pong")
    wr_cool_down_thred   = Param.Latency('700ns', "time that write stage can occupy dram")

    def __init__(self) -> None:
        super().__init__()
        self.st2_vec_lotto = [0,0,100]