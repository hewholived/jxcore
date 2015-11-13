import os
import sys

benchmarks = ['./benchmark/fs/readfile.js', './benchmark/fs/write-stream-throughput.js', './benchmark/http/client-request-body.js', './benchmark/misc/child-process-read.js', './benchmark/misc/startup.js', './benchmark/net/dgram.js', './benchmark/net/net-c2s.js', './benchmark/net/net-pipe.js', './benchmark/net/net-s2c.js', './benchmark/net/tcp-raw-c2s.js', './benchmark/net/tcp-raw-pipe.js', './benchmark/net/tcp-raw-s2c.js', './benchmark/tls/throughput.js', './benchmark/tls/tls-connect.js']

TIMES = 10
MONITORDUR = "20"

def setupMonitor():
    os.system("rm /tmp/enableOracle")
    os.system('echo "0" > /tmp/enableMonitor')
    os.system("rm monitor*")

def changeDuration(benchmark, fr, to):
    os.system('sed -i "s/dur: \\[' + fr + '\\]/dur: \\[' + to + '\\]/g" ' + benchmark)

def getMonitorDb(benchmark):
    setupMonitor()
    changeDuration(benchmark, "[0-9.]*", MONITORDUR)
    if os.system("./jx " + benchmark + " > /dev/null" ) != 0:
        print("./jx " + benchmark + " > /dev/null exited with an error.")
        sys.stdout.flush()
        sys.exit(1)

def setupOracle():
    os.system('echo "0" > /tmp/enableOracle')

def printAverage(benchmark, dur, mode, filename):
    f = open(filename, "r")

    total = 0.0
    for line in f:
        line = line.strip()
        if line == "":
            continue
        vals = line.split(":")
        if len(vals) == 2:
            total += float(vals[1].strip())

    f.close()

    print benchmark + "," + mode + "," + dur + "," + str(total/TIMES)
    sys.stdout.flush()



def runAndGetAverage(benchmark):
    oracleOutFile = b.split("/")[3].split(".")[0] + "O.txt"
    baselineOutFile = b.split("/")[3].split(".")[0] + "B.txt"
    if benchmark == "./benchmark/fs/readfile.js":
        os.system("rm ./benchmark/fs/.removeme-benchmark-garbage")
    for dur in xrange(25, 525, 25):
        dur = str(float(dur/100.0))
        changeDuration(benchmark, "[0-9.]*", dur)

        for times in xrange(0, TIMES):
            setupOracle()
            if os.system("./jx " + benchmark + " >> " + oracleOutFile) != 0:
                print("./jx " + benchmark + " >> " + oracleOutFile + " exited with an error")
                sys.stdout.flush()
                sys.exit(1)
            if benchmark == "./benchmark/fs/readfile.js":
                os.system("rm ./benchmark/fs/.removeme-benchmark-garbage")
        
        printAverage(benchmark, dur, "Oracle", oracleOutFile)
        
        os.system("rm /tmp/enableOracle")
        for times in xrange(0, TIMES):
            if os.system("./jx " + benchmark + " >> " + baselineOutFile) != 0:
                print("./jx " + benchmark + " >> " + baselineOutFile + " exited with an error")
                sys.stdout.flush()
                sys.exit(1)
            if benchmark == "./benchmark/fs/readfile.js":
                os.system("rm ./benchmark/fs/.removeme-benchmark-garbage")
        
        printAverage(benchmark, dur, "Baseline", baselineOutFile)

        os.system("rm " + oracleOutFile + " " + baselineOutFile)


for b in benchmarks:
    print b
    sys.stdout.flush()
    if b == "./benchmark/fs/readfile.js":
        os.system("rm ./benchmark/fs/.removeme-benchmark-garbage")
    getMonitorDb(b)
    os.system("rm /tmp/enableMonitor")
    runAndGetAverage(b)

