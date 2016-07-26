#/bin/env/python

import os, sys, string, json

resubmit              = False
resubmitOnTheFly      = True
checkFileSize         = True
checkLumiContent      = False
EmptyFileSize         = 2321123
resubmitScriptName    = "resubmit.cmd" if not resubmitOnTheFly else "resubmitOnTheFly.cmd"

def initProxy():
   print "You are going to run on a sample over grid using either CRAB or the AAA protocol, it is therefore needed to initialize your grid certificate"
   os.system('mkdir -p ~/x509_user_proxy; voms-proxy-init --voms cms -valid 192:00 --out ~/x509_user_proxy/x509_proxy')#all must be done in the same command to avoid environement problems.  Note that the first sourcing is only needed in Louvain

#
# option 1 just lists runs that have to be rerun
# option 2 creates a new .cmd file that can be used to resubmit on condor
#
if len(sys.argv)==1 or sys.argv[1]=="1":
   runs=os.listdir("out")
   for run in runs:
      if not os.path.isdir("out/"+run):
         continue
      doubleMuon=0
      singleMuon=0
      MET       =0
      listOfFiles = os.listdir("out/%s" % run)
      for file in listOfFiles:
         fileStats=os.stat("out/%s/%s" % (run, file))
         if int(fileStats.st_size) < EmpyFileSize:
           print "File", file, "has to be reprocessed!"
           continue
         if file.find("SingleMuon")!=-1:
            singleMuon += 1
            continue
         elif file.find("DoubleMuon")!=-1:
            doubleMuon += 1
            continue
         elif file.find("MET")!=-1:
            MET += 1
            continue
         else:
            print "Some weird file was found ..."
            continue
      if (doubleMuon != singleMuon) or (doubleMuon != MET) or (singleMuon != MET):
         correctMET       =int(os.popen("grep %s FARM/inputs/*.sh | grep MET        | wc -l" % run).read())
         correctSingleMuon=int(os.popen("grep %s FARM/inputs/*.sh | grep SingleMuon | wc -l" % run).read())
         correctDoubleMuon=int(os.popen("grep %s FARM/inputs/*.sh | grep DoubleMuon | wc -l" % run).read())
   
         if (correctMET != MET) or (correctSingleMuon != singleMuon) or (correctDoubleMuon != doubleMuon):
            print "Run %s: MET (%i/%i), SingleMuon (%i/%i), DoubleMuon (%i/%i)" % (run, MET, correctMET, singleMuon, correctSingleMuon, doubleMuon, correctDoubleMuon)

if sys.argv[1]=="2":
   shellJobs = os.listdir("FARM/inputs")
   shellJobs.sort()
   jobsToRerun=[]
   for file in shellJobs:
      fileIsGood=False
      if not file.find(".sh")!=-1: continue
      endPath=os.popen('grep HSCP_ FARM/inputs/%s' % file).read().split()[2]

      if os.path.isfile(endPath):
         if not checkFileSize: 
            fileIsGood = True
            continue

         fileStats=os.stat(endPath)
         if int(fileStats.st_size) > EmptyFileSize:
            if not checkLumiContent:
               fileIsGood = True
               continue
       
            runs = []
            os.system('edmLumisInFiles.py %s --out tmp.json' % endPath)
            runList = json.load('tmp.json').items()
            for entry in runList:
               runs.append("%s" % str(entry[0]))
            os.system('rm tmp.json')
            if not len(runs)<1: fileIsGood = True
	    
      if not fileIsGood:
         jobsToRerun.append(file)

   if resubmitOnTheFly:
      runningJobs = os.popen('condor_q %s -long | grep Cmd' % os.environ['USER']).read()
      runningJobs = runningJobs.split('\n')
      runningJobs.pop()
      for job in runningJobs:
         runningScript = job.split('/')[len(job.split('/'))-1]
         runningScript = runningScript.replace('"', '')
         if runningScript in jobsToRerun:
            jobsToRerun.remove(runningScript)

   if len(jobsToRerun) > 0:
      print "creating resubmit command script:"
      f = open(resubmitScriptName, "w")
      f.write('Universe                = vanilla\n')
      f.write('Environment             = CONDORJOBID=$(Process)\n')
      f.write('notification            = Error\n')
      f.write('requirements            = (CMSFARM=?=True)&&(Memory > 200)\n')
      f.write('should_transfer_files   = YES\n')
      f.write('when_to_transfer_output = ON_EXIT\n')
      for job in jobsToRerun:
         job = job.split('.')[0]
         f.write('\n')
         f.write('Executable              = FARM/inputs/%s.sh\n' % job)
         f.write('output                  = FARM/logs/%s.out\n'  % job)
         f.write('error                   = FARM/logs/%s.err\n'  % job)
         f.write('log                     = FARM/logs/%s.log\n'  % job)
         f.write('Queue 1\n')
      f.close()
      print "%s written" % resubmitScriptName

      if resubmit:
         initProxy()
         os.system('condor_submit %s' % resubmitScriptName)

