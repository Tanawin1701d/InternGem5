# build/X86/gem5.opt \
# --debug-flags=TraceCPUData \
# /media/tanawin/tanawin1701d/Project/gem5base/gem5/configs/example/etrace_replay.py \
# --inst-trace-file=/media/tanawin/tanawin1701d/Project/sms/pintool/msmsPin/protoFile/instrBeta \
# --data-trace-file=/media/tanawin/tanawin1701d/Project/sms/pintool/msmsPin/protoFile/dynamicData \
# --cpu-type=TraceCPU \
# --caches \
# --mem-size=24GB

build/X86/gem5.opt \
--stats-file=/media/tanawin/tanawin1701d/Project/Intern/InternGem5/m5out/fluidTrace/tracebuilderLim15.txt   \
--json-config=/media/tanawin/tanawin1701d/Project/Intern/InternGem5/m5out/fluidTrace/tracebuilderLim15.json   \
/media/tanawin/tanawin1701d/Project/Intern/InternGem5/configs/example/etrace_replay.py \
--inst-trace-file=/media/tanawin/tanawin1701d/Project/sms/pintool/msmsPin/generatedFile/instrDummy \
--data-trace-file=/media/tanawin/tanawin1701d/Project/sms/pintool/msmsPin/generatedFile/FluidDataSerial \
--cpu-type=TraceCPU \
--caches \
--mem-size=16GB \
--mem-type=SimpleMemory \