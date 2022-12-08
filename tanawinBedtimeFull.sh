# build/X86/gem5.opt \
#                       configs/example/ruby_random_test.py  \
#                       --num-cpus=16 \
#                       --num-dirs=16  \
#                       --network=garnet \
#                       --topology=Mesh_XY \
#                       --mesh-rows=4

#scons build/X86/gem5.opt -j 9

# time build/X86/gem5.opt \
#  --stats-file=/media/tanawin/tanawin1701d/Project/Intern/InternGem5/m5out/fluidanimateRubyTester/sms1.txt \
#  --json-config=/media/tanawin/tanawin1701d/Project/Intern/InternGem5/m5out/fluidanimateRubyTester/sms1.json \
#  configs/learning_gem5/newBench.py \
#  --memAccessDebugStat=/media/tanawin/tanawin1701d/Project/Intern/InternGem5/m5out/fluidanimateRubyTester/sms1.txt \
#  --ruby \
#  --vcs-per-vnet=256 \
#  --topology=Mesh_XY \
#  --mesh-rows=4 \
#  --num-dirs=16  \
#  --network=simple \
#  --mem-size=8192MB \
#  --pasec \
#  --cpu-type=DerivO3CPU \
#  --l1d_size=1kB \
#  --l1i_size=16kB \
#  -n 16 \
#  --interQmemSched=STAGE_SCHED_Queue \


# time build/X86/gem5.opt \
#  --stats-file=/media/tanawin/tanawin1701d/Project/Intern/InternGem5/m5out/fluidanimateRubyTester/sms1.txt \
#  --json-config=/media/tanawin/tanawin1701d/Project/Intern/InternGem5/m5out/fluidanimateRubyTester/sms1.json \
#  configs/learning_gem5/newBench.py \
#  --memAccessDebugStat=/media/tanawin/tanawin1701d/Project/Intern/InternGem5/m5out/fluidanimateRubyTester/sms1.txt \
#  --ruby \
#  --vcs-per-vnet=256 \
#  --topology=Mesh_XY \
#  --mesh-rows=4 \
#  --num-dirs=16  \
#  --network=simple \
#  --mem-size=8192MB \
#  --pasec \
#  --cpu-type=DerivO3CPU \
#  --l1d_size=1kB \
#  --l1i_size=16kB \
#  -n 16 \
#  --interQmemSched=STAGE_SCHED_Queue

time build/X86/gem5.opt \
 --stats-file=/media/tanawin/tanawin1701d/Project/Intern/InternGem5/m5out/testFull/frfcfs16_2.txt \
 --json-config=/media/tanawin/tanawin1701d/Project/Intern/InternGem5/m5out/testFull/frfcfs16_2.json \
 /media/tanawin/tanawin1701d/Project/Intern/InternGem5/configs/example/myFs.py \
 --frame-capture\
 --disk-image=/media/tanawin/tanawin1701d/Project/Intern/gem5e/disk-images/db/parsec_db-image/parsec_db \
 --script=/media/tanawin/tanawin1701d/Project/Intern/InternGem5/configs/example/myScript/mulbench.rcS \
 --kernel=/media/tanawin/tanawin1701d/Project/Intern/InternGem5/fullSysSup/vmlinux-4.19.83 \
 --memAccessDebugStat=/media/tanawin/tanawin1701d/Project/Intern/InternGem5/m5out/testFull/frfcfs16mdb_2.txt \
 --mem-size=8192MB \
 -n 8 \
 --cpu-type=DerivO3CPU \
 --l1d_size=16kB \
 --l1i_size=16kB \
 --caches \
 --memSched=frfcfs \
 --memSchedSize=64 \
 --l2cache




# time build/X86/gem5.opt \
#  --stats-file=/media/tanawin/tanawin1701d/Project/Intern/InternGem5/m5out/fluidanimate_8core/frfcfs.txt \
#  --json-config=/media/tanawin/tanawin1701d/Project/Intern/InternGem5/m5out/fluidanimate_8core/frfcfs.json \
#  configs/learning_gem5/newBench.py \
#  --mem-size=3072MB \
#  --pasec \
#  --cpu-type=DerivO3CPU \
#  --l1d_size=1kB \
#  --l1i_size=16kB \
#  --caches \
#  -n 8 \
#  --memSched=frfcfs \
#  --l2cache

#  time build/X86/gem5.opt \
#  --stats-file=/media/tanawin/tanawin1701d/Project/Intern/InternGem5/m5out/fluidanimate_8core/fcfs.txt \
#  --json-config=/media/tanawin/tanawin1701d/Project/Intern/InternGem5/m5out/fluidanimate_8core/fcfs.json \
#  configs/learning_gem5/newBench.py \
#  --mem-size=3072MB \
#  --pasec \
#  --cpu-type=DerivO3CPU \
#  --l1d_size=1kB \
#  --l1i_size=16kB \
#  --caches \
#  -n 8 \
#  --memSched=fcfs \
#  --l2cache


