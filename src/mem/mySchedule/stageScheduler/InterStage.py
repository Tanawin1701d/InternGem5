from m5.params import *
#from m5.SimObject import SimObject
from m5.objects.InterQueue      import *
from m5.objects.Stages          import *
#from m5.objects.WriteStages     import *


class InterStage(InterQueue):
     type              =  "InterStage"
     cxx_header        =  "mem/mySchedule/stageScheduler/inter_stage.hh"
     cxx_class         =  "gem5::memory::InterStage"

     amt_src           = Param.UInt8      (1   , "amount of src")
     readStages        = Param.Stages     (NULL, "read stage")
     writeStages       = Param.WriteStages(NULL, "write stage")

     def initStage(self, num_src):
          self.readStages  = Stages()
          self.writeStages = WriteStages()
          self.amt_src     = num_src
          self.readStages .init(self.amt_src, self)
          self.writeStages.init(self.amt_src, self)