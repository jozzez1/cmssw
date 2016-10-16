#!/usr/bin/env python

# Original Author:  Loic Quertenmont


import urllib
import string
import os,sys,time,math
import SUSYBSMAnalysis.HSCP.LaunchOnCondor as LaunchOnCondor
import glob
import commands
import json
import collections # kind of map

#script parameters #feel free to edit those
JSON         = 'OneFillRuns.json'
LOCALTIER    = 'T2_BE_UCL'
DATASETMASKS = ['/ZeroBias/Run2016*-PromptReco-v*/RECO']
ISLOCAL      = False #automatically assigned


goodLumis = {}
def LoadJson(jsonPath):
   jsonFile = open(jsonPath,'r')
   runList=json.load(jsonFile,encoding='utf-8').items()
   runList.sort()
   for run in runList :
      goodLumis[int(run[0])] = []
      for lumi in run[1] : goodLumis[int(run[0])].append(lumi)

def IsGoodRun(R):
   if(R in goodLumis): return True
   return False

def IsGoodLumi(R, L):
   for lumi in goodLumis[R]:
      if(L>=lumi[0] and L<=lumi[1]): return True
   return False

def GetRunFromFile(F):
   return int(F.split('/')[8])*1000+int(F.split('/')[9])   

   
def getChunksFromList(MyList, n):
  return [MyList[x:x+n] for x in range(0, len(MyList), n)]


def initProxy():
#   if(not os.path.isfile(os.path.expanduser('~/x509_user_proxy/x509_proxy')) or ((time.time() - os.path.getmtime(os.path.expanduser('~/x509_user_proxy/x509_proxy')))>600 and  int(commands.getstatusoutput('(export X509_USER_PROXY=~/x509_user_proxy/x509_proxy;voms-proxy-init --noregen;voms-proxy-info -all) | grep timeleft | tail -n 1')[1].split(':')[2])<8 )):
      print "You are going to run on a sample over grid using either CRAB or the AAA protocol, it is therefore needed to initialize your grid certificate"
      os.system('mkdir -p ~/x509_user_proxy; voms-proxy-init --voms cms -valid 192:00 --out ~/x509_user_proxy/x509_proxy')#all must be done in the same command to avoid environement problems.  Note that the first sourcing is only needed in Louvain


def filesFromDataset(dataset):
   ISLOCAL=False
   command_out = commands.getstatusoutput('das_client --limit=0 --query "site dataset='+dataset+' | grep site.name,site.dataset_fraction"')
   for site in command_out[1].split('\n'):
      if(LOCALTIER in site and '100.00%' in site): 
         ISLOCAL=True
         break

   Files = {}
   command_out = commands.getstatusoutput('das_client --limit=0 --query "file dataset='+dataset+'"')
   for f in command_out[1].split():
      run = GetRunFromFile(f)
      if(not IsGoodRun(run)):continue
      if(run not in Files):  Files[run] = [] #make sure that we have a collection for this run
      
      if(ISLOCAL and LOCALTIER=='T2_CH_CERN'): Files[run] += ["root://eoscms//eos/cms"+f]
      elif(ISLOCAL):                           Files[run] += [f]
      else       :                             Files[run] += ['root://cms-xrd-global.cern.ch/' + f]
   return Files
  

if len(sys.argv)==1:
        print "Please pass in argument a number between 0 and 3"
        print "  0  - cleanup the workspace (all results are erased)        --> interactive processing" 
        print "  1  - Make EDM files for the considered datasets            --> submitting several jobs"
        print "  2  - Merge the EDM files from several dataset              --> one job per run" 
        sys.exit()


LoadJson(JSON)
if sys.argv[1]=='0':
   os.system("rm -f *.d *.so *.pcm *.root *.pyc") 
   os.system("rm -rdf FARM_TEMP out pictures") 



if sys.argv[1]=='1':

   #get the list of sample to process from das and datasetmask query
   print("Initialize your grid proxy in case you need to access remote samples\n")
   initProxy()

   datasetList = []
   for DATASETMASK in DATASETMASKS:
      command_out = commands.getstatusoutput('das_client --limit=0 --query "dataset='+DATASETMASK+'"')
      print 'das_client --limit=0 --query "dataset='+DATASETMASK+'"'
      print command_out
      datasetList += command_out[1].split()

   #get the list of samples to process from a local file
   #datasetList= open('DatasetList','r')
   JobName = "dEdxSkimmer"
   FarmDirectory = "FARM"
   LaunchOnCondor.SendCluster_Create(FarmDirectory, JobName)
   LaunchOnCondor.Jobs_Queue = '8nh'

   os.system("mkdir -p out");
   for DATASET in datasetList :
      DATASET = DATASET.replace('\n','')
      FILELIST = filesFromDataset(DATASET)
      LaunchOnCondor.Jobs_InitCmds = []
      if(not ISLOCAL):LaunchOnCondor.Jobs_InitCmds = ['export HOME=%s' % os.environ['HOME'],
	'export X509_USER_PROXY=${HOME}/x509_user_proxy/x509_proxy; voms-proxy-init --noregen;']

      print DATASET + " : " 
      for RUN in FILELIST:
         run = str(RUN)
         EndPath = "/storage/data/cms/store/user/jozobec/dEdxCalib/%s" % run
         os.system("mkdir -p %s" % EndPath);
         print str(RUN) + " --> %i files to process"  % len(FILELIST[RUN])
         INDEX=0
         for inFileList in getChunksFromList(FILELIST[RUN],1):

            os.system("cp dEdxSkimmer_Template_cfg.py dEdxSkimmer_cfg.py")
            f = open("dEdxSkimmer_cfg.py", "a")
            os.system("cp dEdxSkimmer_Template_cfg.py dEdxSkimmer_cfg.py")
            f = open("dEdxSkimmer_cfg.py", "a")
            f.write("\n")
            f.write("process.Out.fileName = cms.untracked.string('dEdxSkim_%i_%i.root')\n" % (RUN, INDEX))
            f.write("\n")
            for inFile in inFileList:
                f.write("process.source.fileNames.extend(['"+inFile+"'])\n")
            f.write("\n")
            f.write("#import PhysicsTools.PythonAnalysis.LumiList as LumiList\n")
            f.write("#process.source.lumisToProcess = LumiList.LumiList(filename = '"+JSON+"').getVLuminosityBlockRange()")
            f.write("\n")

            if("/ALCARECO" in DATASET):
               f.write("\n")
               f.write("process.tracksForDeDx.src = cms.InputTag('ALCARECOSiStripCalMinBias') #for SiStripCalMinBias ALCARECO format\n")
               f.write("\n")
            f.close()   
#            LaunchOnCondor.Jobs_FinalCmds = ["mv dEdxSkim.root %s/out/%i/dEdxSkim_%i.root" % (os.getcwd(), RUN, INDEX)]
            LaunchOnCondor.Jobs_FinalCmds = ["lcg-cp -n 10 -D srmv2 -b file://${PWD}/dEdxSkim_%i_%i.root srm://ingrid-se02.cism.ucl.ac.be:8444/srm/managerv2\?SFN=%s/dEdxSkim_%i.root && rm -f dEdxSkim_%i_%i.root" % (RUN, INDEX, EndPath, INDEX, RUN, INDEX)] # if you do not use zsh, change '\?' -> '?'
            LaunchOnCondor.SendCluster_Push  (["CMSSW", "dEdxSkimmer_cfg.py" ])
            os.system("rm -f dEdxSkimmer_cfg.py")
            INDEX+=1

   LaunchOnCondor.SendCluster_Submit()


