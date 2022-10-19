from m5.params import *
from m5.objects.InterQueue      import *

class InterFcfs(InterQueue):
     type              =  "InterFcfs"
     cxx_header        =  "mem/mySchedule/fcfsScheduler/inter_fcfs.hh"
     cxx_class         =  "gem5::memory::InterFcfs"

     