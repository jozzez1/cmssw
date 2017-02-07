#! /usr/bin/env python

import os

indir = "Results/"
jobfilesdir = "FARM/inputs/" 
cmdfile = "FARM/inputs/HscpAnalysis.cmd"
samplesFiles = "Analysis_Samples.txt"
types = set()

#for root, dirs, files in os.walk(indir):
#    if "Type" not in root: continue
#    anaType = int(root.split("/")[-1].replace("Type",""))
#    types.add(anaType)

types.add(0)
types.add(2)

todo = []
with open(samplesFiles) as ifile:
    for iline, l in enumerate(ifile):
        line = l.strip()
        if len(line)==0 or line[0]=='#' : continue
        spl = [l.strip().strip('"') for l in line.split(",")]
        expectedFileName= "Histos_{}_{}.root".format(spl[2], spl[3])
        sampleString = "ANALYSE_{}_to_{}".format(iline, iline)
        for t in types:
            fp = indir+"Type{}".format(t)+"/"+expectedFileName
            if not os.path.isfile(fp) or os.path.getsize(fp)<1024:
                typeString = ", {},".format(t)
                for root, _, files in os.walk(jobfilesdir):
                    for f in files:
                        if "_HscpAnalysis.sh" not in f: continue
                        contents = open(os.path.join(root, f)).read()
                        if sampleString not in contents or typeString not in contents:
                            continue
                        todo.append(f)

redo=[]
with open(cmdfile) as f:
    for l in f:
        line = l.strip()
        for t in todo:
            if t in line:
                m = line.split('=')[1]
                m = m.split('/')[len(m.split('/'))-1]
                m = m.replace('.sh', '')
                redo.append(m)
                print line.split('=')[1]
#                os.system("sh" + line.split('=')[1])
                break

#newcmd = open("newcmd.cmd", "w")
#for executable in redo:
#   newcmd.write('Executable              = FARM/inputs/%s.sh\n' % executable)
#   newcmd.write('output                  = FARM/logs/%s.out\n' % executable)
#   newcmd.write('error                   = FARM/logs/%s.err\n' % executable)
#   newcmd.write('log                     = FARM/logs/%s.log\n' % executable)
#   newcmd.write('Queue 1\n')
#   newcmd.write('\n')
#newcmd.close()
