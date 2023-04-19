# build/X86/gem5.opt \
# --debug-flags=TraceCPUData \
# /media/tanawin/tanawin1701d/Project/gem5base/gem5/configs/example/etrace_replay.py \
# --inst-trace-file=/media/tanawin/tanawin1701d/Project/sms/pintool/msmsPin/protoFile/instrBeta \
# --data-trace-file=/media/tanawin/tanawin1701d/Project/sms/pintool/msmsPin/protoFile/dynamicData \
# --cpu-type=TraceCPU \
# --caches \
# --mem-size=24GB

build/X86/gem5.opt \
--stats-file=/media/tanawin/tanawin1701d/Project/Intern/InternGem5/m5out/fluidTrace/tracebuilderReal.txt   \
--json-config=/media/tanawin/tanawin1701d/Project/Intern/InternGem5/m5out/fluidTrace/tracebuilderReal.json   \
/media/tanawin/tanawin1701d/Project/Intern/InternGem5/configs/example/etrace_replay.py \
--inst-trace-file=/media/tanawin/tanawin1701d/Project/Intern/InternGem5/m5out/system.cpu.traceListener.instTrace.proto.gz \
--data-trace-file=/media/tanawin/tanawin1701d/Project/Intern/InternGem5/m5out/system.cpu.traceListener.dataTrace.proto.gz \
--cpu-type=TraceCPU \
--caches \
--mem-size=16GB \
--mem-type=SimpleMemory