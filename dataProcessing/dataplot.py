import matplotlib.pyplot as plt

#Green ["#18B30C","#1DD90F","#22FF12"]
#Red ["#B30E02","#D91002","#FF1303"]
#Blue ["#3507B3","#4009D9","#4B0AFF"]
#Yellow ["#B38807","#D9A509","#FFC20A"]

def genInput(numPoints):
    input = {
        "size" : numPoints,
        "dataDir" : "sample",
        "programs" : ["serial", "parallelSearch", "partitionSpace", "processPool", "forkJoin"],
        "programsConfig" : {
            "serial" : {
                "name" : "Serial",
                "lines": ["Serial"],
                "color": ["#0AF5D9"],
                "marker": ["o"]
            },
            "parallelSearch" : {
                "name" : "Parallel Search",
                "lines" : ["Parallel Search: Min", "Parallel Search: Avg", "Parallel Search: Max"],
                "color": ["#3507B3","#4009D9","#4B0AFF"],
                "marker": ["v","o","^"]
            },
            "partitionSpace" : {
                "name" : "Partition Space",
                "lines" : ["Partition Space: Min", "Partition Space: Avg", "Partition Space: Max"],
                "color": ["#18B30C","#1DD90F","#22FF12"],
                "marker": ["v","o","^"]
            },
            "processPool" : {
                "name" : "Process Pool",
                "lines" : ["Process Pool"],
                "color": ["#D91002"],
                "marker": ["o"]
            },
            "forkJoin" : {
                "name" : "Fork-Join",
                "lines" : ["Fork-Join: Min", "Fork-Join: Avg", "Fork-Join: Max"],
                "color": ["#B38807","#D9A509","#FFC20A"],
                "marker": ["v","o","^"]
            }
        },
        "data" : {}
    }
    return input

def loadInput(datafile):
    data = []
    with open(datafile) as fp:
        for line in fp:
            s = line.split(" ")
            if len(s) == 3:
                data.append((float(s[1]),))
            elif len(s) == 10:
                data.append((float(s[2]),float(s[5]),float(s[8])))
            else:
                print("Unexpected line:"+line)
    return data

def genPlot(input, axs):
    for program in input["programs"]:
        input["data"][program] = loadInput(input["dataDir"]+"/"+program)
        data = input["data"][program]
        # print(data)
        for j in range(len(data[0])): #For min,avg,max or just single
            y = []
            for i in range(len(data)):
                y.append(data[i][j])
            config = input["programsConfig"][program]
            print(config)
            axs.plot(numProcesses, y, label=config["lines"][j], marker=config["marker"][j], color=config["color"][j])
        axs.set_title(format(input["size"], ",") + " points")
        axs.set_xticks(numProcesses)
        axs.legend()


numProcesses = [1,2,4,8,16,32]

fig, axs = plt.subplots(2, 2)
genPlot(genInput(4096), axs[0][0])
genPlot(genInput(16384), axs[0][1])
genPlot(genInput(65536), axs[1][0])
genPlot(genInput(262144), axs[1][1])
fig.suptitle("Runtimes by Worker Process Count")
fig.set_size_inches(10, 10)
plt.show()
