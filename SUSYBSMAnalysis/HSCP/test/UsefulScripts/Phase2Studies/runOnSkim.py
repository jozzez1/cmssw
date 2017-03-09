#!/usr/bin/env python

import urllib
import string
import os,sys,time
import SUSYBSMAnalysis.HSCP.LaunchOnCondor as LaunchOnCondor  
import glob
import commands
import json
import collections # kind of map

def getChunksFromList(MyList, n):
  return [MyList[x:x+n] for x in range(0, len(MyList), n)]

def initProxy():
   if(not os.path.isfile(os.path.expanduser('~/x509_user_proxy/x509_proxy')) or ((time.time() - os.path.getmtime(os.path.expanduser('~/x509_user_proxy/x509_proxy')))>600)):
      print "You are going to run on a sample over grid using either CRAB or the AAA protocol, it is therefore needed to initialize your grid certificate"
      os.system('mkdir -p ~/x509_user_proxy; voms-proxy-init --voms cms -valid 192:00 --out ~/x509_user_proxy/x509_proxy')#all must be done in the same command to avoid environement problems.  Note that the first sourcing is only needed in Louvain

if len(sys.argv)==1:
        print "Please pass in argument a number between 1 and 3"
        print "  1  - Run dEdxStudy on RECO, AOD, or dEdxSKIM files         --> submitting 1job per file"
        print "  2  - Hadd root files containing the histograms             --> interactive processing" 
        sys.exit()


LocalStorePath = '/storage/data/cms/store/user/jozobec/Phase2HSCP/'
datasetList = [
#   ['MinBias_NoPU'     , LocalStorePath + 'MinBias_noPU_TuneCUETP8M1_14TeV-pythia8/'],
#   ['MinBias_140PU'    , LocalStorePath + 'MinBias_140PU_TuneCUETP8M1_14TeV-pythia8/'],
#   ['MinBias_200PU'    , LocalStorePath + 'MinBias_200PU_TuneCUETP8M1_14TeV-pythia8/'],
#   ['DYJetsToLL_NoPU'  , LocalStorePath + 'DYJetsToLL_M-50_TuneCUETP8M1_14TeV-madgraphMLM-pythia8_ext1_NoPU/'],
#   ['DYJetsToLL_140PU' , LocalStorePath + 'DYJetsToLL_M-50_TuneCUETP8M1_14TeV-madgraphMLM-pythia8_ext1_140PU/'],
#   ['DYJetsToLL_200PU' , LocalStorePath + 'DYJetsToLL_M-50_TuneCUETP8M1_14TeV-madgraphMLM-pythia8_ext1_200PU/'],
#   ['TTTo2L2Nu_NoPU'   , LocalStorePath + 'TTTo2L2Nu_TuneCUETP8M1_14TeV-powheg-pythia8_NoPU/'],
#   ['TTTo2L2Nu_140PU'  , LocalStorePath + 'TTTo2L2Nu_TuneCUETP8M1_14TeV-powheg-pythia8_140PU/'],
#   ['TTTo2L2Nu_200PU'  , LocalStorePath + 'TTTo2L2Nu_TuneCUETP8M1_14TeV-powheg-pythia8_200PU/'],

   ['ppstau_651_NoPU'  , LocalStorePath + 'HSCPppstau_M_651_TuneCUETP8M1_14TeV_pythia8_NoPU/'],
   ['ppstau_651_140PU' , LocalStorePath + 'HSCPppstau_M_651_TuneCUETP8M1_14TeV_pythia8_140PU/'],
   ['ppstau_651_200PU' , LocalStorePath + 'HSCPppstau_M_651_TuneCUETP8M1_14TeV_pythia8_200PU/'],

   ['ppstau_1218_NoPU' , LocalStorePath + 'HSCPppstau_M_1218_TuneCUETP8M1_14TeV_pythia8_NoPU/'],
   ['ppstau_1218_140PU', LocalStorePath + 'HSCPppstau_M_1218_TuneCUETP8M1_14TeV_pythia8_140PU/'],
   ['ppstau_1218_200PU', LocalStorePath + 'HSCPppstau_M_1218_TuneCUETP8M1_14TeV_pythia8_200PU/'],

#   ['ppstau_1599_NoPU' , LocalStorePath + 'HSCPppstau_M_1599_TuneCUETP8M1_14TeV_pythia8_NoPU/'],
#   ['ppstau_1599_140PU', LocalStorePath + 'HSCPppstau_M_1599_TuneCUETP8M1_14TeV_pythia8_140PU/'],
#   ['ppstau_1599_200PU', LocalStorePath + 'HSCPppstau_M_1599_TuneCUETP8M1_14TeV_pythia8_200PU/'],

#   ['ppstau_200_NoPU'  , LocalStorePath + 'HSCPppstau_M_200_TuneCUETP8M1_14TeV_pythia8_NoPU/'],
#   ['ppstau_200_140PU' , LocalStorePath + 'HSCPppstau_M_200_TuneCUETP8M1_14TeV_pythia8_140PU/'],
#   ['ppstau_200_200PU' , LocalStorePath + 'HSCPppstau_M_200_TuneCUETP8M1_14TeV_pythia8_200PU/'],
]

remote_ls_command    = 'gfal-ls -l srm://ingrid-se02.cism.ucl.ac.be:8444/srm/managerv2\?SFN='
remote_access_prefix = 'root://ingrid-se03.cism.ucl.ac.be//'

isLocal = False  #allow to access data in Louvain from remote sites
if(commands.getstatusoutput("hostname -f")[1].find("ucl.ac.be")!=-1): isLocal = True
os.system('rm -rf ~/x509_user_proxy/x509_proxy')


if sys.argv[1]=='1':
        os.system("sh " + os.getcwd() + "/DeDxStudy.sh ") #just compile

	for DATASET in datasetList :
	   outdir =  os.getcwd() + "/Histos/"+DATASET[0]+"/"
	   os.system('mkdir -p ' + outdir)

	   JobName = "DEDXHISTO_"+DATASET[0]
	   FarmDirectory = "FARM_DEDXHISTO_"+DATASET[0]
	   LaunchOnCondor.Jobs_Queue = '8nh'
	   LaunchOnCondor.SendCluster_Create(FarmDirectory, JobName)

 	   FILELIST = []        
           if(DATASET[1][-1]=='/'): #file path is a directory, consider all files from the directory
              if(isLocal):
      	         FILELIST = LaunchOnCondor.GetListOfFiles('', DATASET[1]+'/*.root', '')
              else:
                 initProxy()
                 initCommand = 'export X509_USER_PROXY=~/x509_user_proxy/x509_proxy; voms-proxy-init --noregen; '
                 LaunchOnCondor.Jobs_InitCmds = [initCommand]
                 print initCommand + remote_ls_command + DATASET[1]+' | awk \'{print $9}\''
                 print commands.getstatusoutput(initCommand + remote_ls_command+DATASET[1] + ' | awk \'{print $9}\'')
                 LocalFileList = commands.getstatusoutput(initCommand + remote_ls_command + DATASET[1] + ' | awk \'{print $9}\'')[1].split('\n')

                 for f in LocalFileList:
                    if(f[-5:].find('.root')==-1):continue #only .root file considered
                    FILELIST += [remote_access_prefix + DATASET[1].replace('/storage/data/cms/store/','/store/')+f]
           else: #file path is an HSCP sample name, use the name to run the job
              FILELIST += [DATASET[1]]
             

           print FILELIST
           for inFileList in getChunksFromList(FILELIST,max(1,len(FILELIST)/50)): #50 jobs, this is a trade off between hadding time and processing time
              InputListCSV = ''
              for inFile in inFileList:
                 InputListCSV+= inFile + ','
              InputListCSV = InputListCSV[:-1] #remove the last duplicated comma
              LaunchOnCondor.SendCluster_Push  (["BASH", "sh " + os.getcwd() + "/DeDxStudy.sh " + InputListCSV + " out.root; mv out.root " + outdir+"dEdxHistos_%i.root" %  LaunchOnCondor.Jobs_Count ])
	   LaunchOnCondor.SendCluster_Submit()

elif sys.argv[1]=='2':
        for DATASET in datasetList :#+signalList :
           indir =  os.getcwd() + "/Histos/"+DATASET[0]+'/'
           os.system('rm -f Histos_'+DATASET[0]+'.root')
           os.system('find ' + indir + '*.root  -type f -size +1024c | xargs hadd -f Histos_'+DATASET[0]+'.root')
	# finally merge all the runs into the histogram with data
	#os.system('rm -f Histos_Data.root')
	#os.system('hadd -f Histos_Data.root Histos_Run*.root')

elif sys.argv[1]=='3':
        os.system('sh MakePlot.sh')

else:
   print "Invalid argument"
   sys.exit()

