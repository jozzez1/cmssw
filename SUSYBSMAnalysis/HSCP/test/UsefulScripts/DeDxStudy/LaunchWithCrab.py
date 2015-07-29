#!/usr/bin/env python

import urllib
import string
import os
import sys
import SUSYBSMAnalysis.HSCP.LaunchOnCondor as LaunchOnCondor 
import glob

from os import listdir
from os.path import isfile, join

if len(sys.argv)==1:       
	print "Please pass in argument a number between 1 and 5"
        print "  1  - create the EDM files which we have to study"
        print "  2  - Submit each dEdx_Skim file to be studied"
        print "  3  - Merge all output files and run the plot generating script"
	sys.exit()

STORE = "/storage/data/cms/store/user/jozobec/ZeroBias/crab_DeDxSkimmerNEW/150720_122314/0000/" # Data
#STORE = "/nfs/scratch/fynu/jzobec/CMSSW_7_4_6/src/SUSYBSMAnalysis/HSCP/test/UsefulScripts/SampleProduction/FARM_MC_13TeV_MinBias_TuneCUETP8M1_SIMAOD/outputs #MC

if sys.argv[1]=='1':
	print 'SKIMMING DEDX EDM'
	startJob = raw_input("This will delete the old crab dir! Proceed? (y/n) ")
	if (startJob == "n"): sys.exit(0)
	print 'starting on crab ...'
	sys.exit(0)
	os.system('rm -rf crab_DeDxSkimmerNew')
	os.system('crab submit -c crab_cfg.py')
	
elif sys.argv[1]=='2':	
        print 'POST-SKIMMING'
        FarmDirectory = "FARM"
        JobName = "DeDxStudy"
	LaunchOnCondor.Jobs_RunHere = 1
	LaunchOnCondor.SendCluster_Create(FarmDirectory, JobName)	
        rootfiles = [f for f in listdir(STORE) if isfile(join(STORE,f)) ]
        index = -1
	os.system("sh DeDxStudy.sh")
        for file in rootfiles :
		index+=1
		LaunchOnCondor.SendCluster_Push(["BASH", os.getcwd()+"/DeDxStudy.sh "+STORE+file+' dEdxSkim_'+str(index)+'_ready.root'])
	LaunchOnCondor.SendCluster_Submit()

elif sys.argv[1]=='3':
	CMSSW_VERSION = os.getenv('CMSSW_VERSION','CMSSW_VERSION')
	if CMSSW_VERSION == 'CMSSW_VERSION':
		print 'please setup your CMSSW environement'
		sys.exit(0)

	print 'MERGING AND PLOTTING'
        FarmDirectory = "FARM"
        JobName = "DeDxMakePlots"
        LaunchOnCondor.Jobs_RunHere = 1
        LaunchOnCondor.SendCluster_Create(FarmDirectory, JobName)
	os.system('rm -f dEdxHistosNew.root')
	os.system('hadd -f dEdxHistosNew.root dEdxSkim_*_ready.root')
	os.system('sh MakePlot.sh') 

