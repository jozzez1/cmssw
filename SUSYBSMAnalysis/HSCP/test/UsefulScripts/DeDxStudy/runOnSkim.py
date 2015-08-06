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


if len(sys.argv)==1:
        print "Please pass in argument a number between 1 and 3"
        print "  1  - Run dEdxStudy on RECO, AOD, or dEdxSKIM files         --> submitting 1job per file"
        print "  2  - Hadd root files containing the histograms             --> interactive processing" 
        print "  3  - run the plotter on the hadded root files              --> interactive processing" 
        sys.exit()



datasetList = [
#  ["Run251252", "/storage/data/cms/store/user/jozobec/ZeroBias/crab_DeDxSkimmerNEW/150720_122314/0000/"],
#  ["MCMinBias", "/home/fynu/jzobec/scratch/CMSSW_7_4_6/src/SUSYBSMAnalysis/HSCP/test/UsefulScripts/SampleProduction/FARM_MC_13TeV_MinBias_TuneCUETP8M1_SIMAOD/outputs/"],
]

signalList = [
   ["MCGluino_M1000_f10", "Gluino_13TeV_M1000_f10"],
   ["MCGluino_M1400_f10", "Gluino_13TeV_M1400_f10"],
   ["MCGluino_M1800_f10", "Gluino_13TeV_M1800_f10"],
   ["MCGMStau_M494",      "GMStau_13TeV_M494"],
   ["MCStop_M1000",       "Stop_13TeV_M1000"],
]

if sys.argv[1]=='1':
        os.system("sh " + os.getcwd() + "/DeDxStudy.sh ") #just compile

	for DATASET in datasetList :
	   outdir =  os.getcwd() + "/Histos/"+DATASET[0]+"/"
	   os.system('mkdir -p ' + outdir)

	   JobName = "DEDXHISTO_"+DATASET[0]
	   FarmDirectory = "FARM_DEDXHISTO_"+DATASET[0]
	   LaunchOnCondor.Jobs_Queue = '8nh'
	   LaunchOnCondor.SendCluster_Create(FarmDirectory, JobName)

	   FILELIST = LaunchOnCondor.GetListOfFiles('', DATASET[1]+'/*.root', '')
           for inFileList in getChunksFromList(FILELIST,len(FILELIST)/15): #30 jobs, this is a trade off between hadding time and processing time
              InputListCSV = ''
  	      for inFile in inFileList:
                 InputListCSV+= inFile + ','
              InputListCSV = InputListCSV[:-1] #remove the last duplicated comma
              LaunchOnCondor.SendCluster_Push  (["BASH", "sh " + os.getcwd() + "/DeDxStudy.sh " + InputListCSV + " out.root; mv out.root " + outdir+"dEdxHistos_%i.root" %  LaunchOnCondor.Jobs_Count ])
	   LaunchOnCondor.SendCluster_Submit()

	for SIGNAL in signalList :
	   outdir =  os.getcwd() + "/Histos/"+SIGNAL[0]+"/"
	   os.system('mkdir -p ' + outdir)

	   JobName = "DEDXHISTO_"+SIGNAL[0]
	   FarmDirectory = "FARM_DEDXHISTO_"+SIGNAL[0]
	   LaunchOnCondor.Jobs_Queue = '8nh'
	   LaunchOnCondor.SendCluster_Create(FarmDirectory, JobName)
	   LaunchOnCondor.SendCluster_Push  (["BASH", "sh " + os.getcwd() + "/DeDxStudy.sh " + SIGNAL[1] + " out.root; mv out.root " + outdir+"dEdxHistos_%i.root" %  LaunchOnCondor.Jobs_Count ])
	   LaunchOnCondor.SendCluster_Submit()

elif sys.argv[1]=='2':
        for DATASET in datasetList :#+signalList :
           indir =  os.getcwd() + "/Histos/"+DATASET[0]+'/'
           os.system('rm -f Histos_'+DATASET[0]+'.root')
           os.system('hadd -f Histos_'+DATASET[0]+'.root ' + indir + '*.root')
	for DATASET in signalList:
	   indir =  os.getcwd() + "/Histos/"+DATASET[0]+'/'
	   os.system('cp ' + indir + 'dEdx*.root ' + 'Histos_'+DATASET[0]+'.root')

elif sys.argv[1]=='3':
        for DATASET in datasetList+signalList :
           os.system('sh MakePlot.sh Histos_'+DATASET[0]+'.root')

else:
   print "Invalid argument"
   sys.exit()

