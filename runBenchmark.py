import os
import sys
import sqlite3

benchmarks = ['./benchmark/http/client-request-body.js', './benchmark/misc/startup.js', './benchmark/net/dgram.js', './benchmark/net/net-c2s.js', './benchmark/net/net-pipe.js', './benchmark/net/net-s2c.js', './benchmark/tls/throughput.js', './benchmark/tls/tls-connect.js']
#benchmarks = ['./benchmark/net/net-c2s.js', './benchmark/net/net-pipe.js', './benchmark/net/net-s2c.js', './benchmark/tls/throughput.js', './benchmark/tls/tls-connect.js']
#benchmarks = ['./benchmark/tls/tls-connect.js']
benchmarks = ['./benchmark/http/client-request-body.js', './benchmark/net/net-pipe.js', './benchmark/tls/throughput.js', './benchmark/tls/tls-connect.js']

TIMES = 10
MONITORDUR = "20"

def setupMonitor():
    os.system('echo "0" > /tmp/enableMonitor')
    os.system("rm monitor*")

def changeDuration(benchmark, fr, to):
    os.system('sed -i "s/dur: \\[' + fr + '\\]/dur: \\[' + to + '\\]/g" ' + benchmark)

def getMonitorDb(benchmark):
    setupMonitor()
    changeDuration(benchmark, "[0-9.]*", MONITORDUR)
    #if os.system("./jx " + benchmark ) != 0:
    if os.system("./jx " + benchmark + " > /dev/null" ) != 0:
        print("./jx " + benchmark + " > /dev/null exited with an error.")
        sys.stdout.flush()
        sys.exit(1)

def setupOracle():
    os.system('echo "0" > /tmp/enableOracle')

def getAverage(filename):
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
    return total/TIMES




def runAndGetAverage(benchmark):
    oracleOutFile = b.split("/")[3].split(".")[0] + "O.txt"
    baselineOutFile = b.split("/")[3].split(".")[0] + "B.txt"
    secondTime = False
    for dur in xrange(25, 325, 25):
        dur = str(float(dur/100.0))
        changeDuration(benchmark, "[0-9.]*", dur)

        for times in xrange(0, TIMES):
            setupOracle()
            if os.system("./jx " + benchmark + " >> " + oracleOutFile) != 0:
                print("./jx " + benchmark + " >> " + oracleOutFile + " exited with an error")
                sys.stdout.flush()
                sys.exit(1)
        
        os.system("rm /tmp/enableOracle")
        for times in xrange(0, TIMES):
            if os.system("./jx " + benchmark + " >> " + baselineOutFile) != 0:
                print("./jx " + benchmark + " >> " + baselineOutFile + " exited with an error")
                sys.stdout.flush()
                sys.exit(1)
        
        baselineValue = getAverage(baselineOutFile)
        oracleValue = getAverage(oracleOutFile)
        speedup = (oracleValue - baselineValue)* 100 / baselineValue
        print benchmark + "," + dur + ":Oracle=" + str(oracleValue) + ":Baseline=" + str(baselineValue) + ":Speedup=" + str(speedup)
        sys.stdout.flush()

        os.system("rm " + oracleOutFile + " " + baselineOutFile)
        if speedup < 0:
            if secondTime:
                break;
            else:
                secondTime = True


for b in benchmarks:
    print b
    sys.stdout.flush()
    os.system("rm /tmp/enableOracle")
    getMonitorDb(b)
    #conn = sqlite3.connect("monitor1.db")
    #c = conn.cursor()
    #c.execute("SELECT COUNT(*) FROM HOTFUNCS")
    #all_rows = c.fetchall()
    #print all_rows
    #conn = sqlite3.connect("monitor2.db")
    #c = conn.cursor()
    #c.execute("SELECT COUNT(*) FROM HOTFUNCS")
    #all_rows = c.fetchall()
    #print all_rows
    os.system("rm /tmp/enableMonitor")
    runAndGetAverage(b)

