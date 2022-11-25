
import matplotlib.pyplot as plt
import argparse 
import time
parser = argparse.ArgumentParser("my argument")
parser.add_argument('-path'         , "--f" , type = str , default =  "/tmp/gem5.txt" , help = "file name")
parser.add_argument('-iter'         , "--i" , type = int , default =  1        , help = "number of iteration")

#file initializer
args           = parser.parse_args()
fileName       = getattr(args, "f"  )


srcFile = open(fileName, "r")
lines = srcFile.readlines()
perCoreArr = []
maxBank = 8
oldCore    = 0

def corePlot(perCoreArr, coreId):
    print("core  ", coreId)
    plt.scatter([x for (x, _) in perCoreArr ], [y for (_, y) in perCoreArr ])


for rawStr in lines:
    lineArr = rawStr.split()
    core    = int(lineArr[0])
    rank    = int(lineArr[1])
    bank    = int(lineArr[2])
    row     = int(lineArr[3])
    count   = int(lineArr[4])
    
    if (oldCore != core) :
        corePlot(perCoreArr, oldCore)
        oldCore = core
        perCoreArr.clear()
    perCoreArr.append( (row, maxBank * rank + bank) )

if not perCoreArr:
    corePlot(perCoreArr, oldCore)
plt.show()
    #print(core, rank, bank, row, count)






    