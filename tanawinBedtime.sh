scons build/X86/gem5.opt -j 9

time build/X86/gem5.opt \
 configs/learning_gem5/newBench.py \
 --pasec \
 --cpu-type=DerivO3CPU \
 --l1d_size=1kB \
 --l1i_size=16kB \
 --caches \
 -n 4 \
 --interQmemSched=STAGE_SCHED_Queue \
 --l2cache