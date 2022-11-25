import argparse 
parser = argparse.ArgumentParser("my argument")
parser.add_argument('-path'         , "--f" , type = str , default =  "/tmp/gem5.txt" , help = "file name")
parser.add_argument('-iter'         , "--i" , type = int , default =  1        , help = "number of iteration")

#file initializer
args           = parser.parse_args()
fileName       = getattr(args, "f"  )


srcFile = open(fileName, "r")
lines = srcFile.readlines()
perCoreArr = []
allCoreSet = set()
maxBank    = 8
oldCore    = 0
sameCount  = 0
totalcount = 0

def coreCheck():
    global sameCount
    global totalcount
    for x in perCoreArr:
        test = set({x})
        if ( test.issubset(allCoreSet) ):
            sameCount = sameCount + 1
        else:
            allCoreSet.add(x)
        totalcount = totalcount + 1
    

for rawStr in lines:
    lineArr = rawStr.split()
    core    = int(lineArr[0])
    rank    = int(lineArr[1])
    bank    = int(lineArr[2])
    row     = int(lineArr[3])
    count   = int(lineArr[4])
    
    if (oldCore != core) :
        coreCheck()
        oldCore = core
        perCoreArr.clear()
    perCoreArr.append( (row, maxBank * rank + bank) )

if not perCoreArr:
    coreCheck()

print("there are same : ", sameCount)
print("there are mpkt : ", totalcount)
print("same ratio     : ", sameCount / totalcount)

    #print(core, rank, bank, row, count)






    