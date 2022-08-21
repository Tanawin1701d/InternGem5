import this
from m5.params import *
#from m5.SimObject import SimObject
from m5.InterQueue      import *
from m5.Stages          import *
from m5.WriteStages     import *


class InterStage(InterQueue):
     type              =  "InterStage"
     cxx_header        =  "mem/mySchedule/stageScheduler/inter_stage.hh"
     cxx_class         =  "gem5::memory::InterStage"

     amt_src           = Param.UInt8      (1   , "amount of src")
     readStages        = Param.Stages     (NULL, "read stage")
     writeStages       = Param.WriteStages(NULL, "write stage")

     def initStage(self):
          self.readStages  = Stages()
          self.writeStages = WriteStages()
          
          self.readStages .init(self.amt_src, self)
          self.writeStages.init(self.amt_src, self)