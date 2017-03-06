#!/usr/bin/env python

import string, os, sys
import SUSYBSMAnalysis.HSCP.LaunchOnCondor as LaunchOnCondor

removeOld = False

datasets     = [
#   '/MinBias_noPU_TuneCUETP8M1_14TeV-pythia8/PhaseIIFall16DR82-NoPU_90X_upgrade2023_realistic_v1-v1/GEN-SIM-RECO',
#   '/MinBias_140PU_TuneCUETP8M1_14TeV-pythia8/PhaseIIFall16DR82-PU140_90X_upgrade2023_realistic_v1-v1/GEN-SIM-RECO',
#   '/MinBias_200PU_TuneCUETP8M1_14TeV-pythia8/PhaseIIFall16DR82-PU200_90X_upgrade2023_realistic_v1-v1/GEN-SIM-RECO',
#
#   '/DYJetsToLL_M-50_TuneCUETP8M1_14TeV-madgraphMLM-pythia8_ext1/PhaseIIFall16DR82-NoPU_90X_upgrade2023_realistic_v1-v1/GEN-SIM-RECO',
#   '/DYJetsToLL_M-50_TuneCUETP8M1_14TeV-madgraphMLM-pythia8_ext1/PhaseIIFall16DR82-PU140_90X_upgrade2023_realistic_v1_ext1-v1/GEN-SIM-RECO',
#   '/DYJetsToLL_M-50_TuneCUETP8M1_14TeV-madgraphMLM-pythia8_ext1/PhaseIIFall16DR82-PU200_90X_upgrade2023_realistic_v1_ext1-v1/GEN-SIM-RECO',
#
#   '/TTTo2L2Nu_TuneCUETP8M1_14TeV-powheg-pythia8/PhaseIIFall16DR82-NoPU_90X_upgrade2023_realistic_v1-v1/GEN-SIM-RECO',
#   '/TTTo2L2Nu_TuneCUETP8M1_14TeV-powheg-pythia8/PhaseIIFall16DR82-PU140_90X_upgrade2023_realistic_v1_ext1-v1/GEN-SIM-RECO',
#   '/TTTo2L2Nu_TuneCUETP8M1_14TeV-powheg-pythia8/PhaseIIFall16DR82-PU200_90X_upgrade2023_realistic_v1_ext1-v1/GEN-SIM-RECO',
#	
#   '/HSCPppstau_M_651_TuneCUETP8M1_14TeV_pythia8/PhaseIIFall16DR82-NoPU_90X_upgrade2023_realistic_v1-v1/GEN-SIM-RECO',
#   '/HSCPppstau_M_651_TuneCUETP8M1_14TeV_pythia8/PhaseIIFall16DR82-PU140_90X_upgrade2023_realistic_v1-v1/GEN-SIM-RECO',
#   '/HSCPppstau_M_651_TuneCUETP8M1_14TeV_pythia8/PhaseIIFall16DR82-PU200_90X_upgrade2023_realistic_v1-v1/GEN-SIM-RECO',
#
#   '/HSCPppstau_M_1218_TuneCUETP8M1_14TeV_pythia8/PhaseIIFall16DR82-NoPU_90X_upgrade2023_realistic_v1-v1/GEN-SIM-RECO',
#   '/HSCPppstau_M_1218_TuneCUETP8M1_14TeV_pythia8/PhaseIIFall16DR82-PU140_90X_upgrade2023_realistic_v1-v1/GEN-SIM-RECO',
#   '/HSCPppstau_M_1218_TuneCUETP8M1_14TeV_pythia8/PhaseIIFall16DR82-PU200_90X_upgrade2023_realistic_v1-v1/GEN-SIM-RECO',
#
#   '/HSCPppstau_M_1599_TuneCUETP8M1_14TeV_pythia8/PhaseIIFall16DR82-NoPU_90X_upgrade2023_realistic_v1-v1/GEN-SIM-RECO',
#   '/HSCPppstau_M_1599_TuneCUETP8M1_14TeV_pythia8/PhaseIIFall16DR82-PU140_90X_upgrade2023_realistic_v1-v1/GEN-SIM-RECO',
#   '/HSCPppstau_M_1599_TuneCUETP8M1_14TeV_pythia8/PhaseIIFall16DR82-PU200_90X_upgrade2023_realistic_v1-v1/GEN-SIM-RECO',
#
   '/HSCPppstau_M_200_TuneCUETP8M1_14TeV_pythia8/PhaseIIFall16DR82-NoPU_90X_upgrade2023_realistic_v1-v1/GEN-SIM-RECO',
   '/HSCPppstau_M_200_TuneCUETP8M1_14TeV_pythia8/PhaseIIFall16DR82-PU140_90X_upgrade2023_realistic_v1-v1/GEN-SIM-RECO',
   '/HSCPppstau_M_200_TuneCUETP8M1_14TeV_pythia8/PhaseIIFall16DR82-PU200_90X_upgrade2023_realistic_v1-v1/GEN-SIM-RECO'
]

outdir          = 'out'
server          = 'root://cms-xrd-global.cern.ch/'
storageDir      = "/storage/data/cms/store/user/jozobec/Phase2HSCP"
storageTransfer = True

def outDirName (dataset):
    outName = dataset.split('/')[1]
    if outName.find('PU') == -1 and dataset.find('PU') != -1:
       if dataset.find('140PU') != -1 or dataset.find('PU140') != -1:
          outName += '_140PU'
       elif dataset.find('200PU') != -1 or dataset.find('PU200') != -1:
          outName += '_200PU'
       elif dataset.find('NoPU') != -1 or dataset.find('noPU') != -1:
          outName += '_NoPU'
    return outName
   
def getDatasetFiles (dataset):
    return os.popen('das_client --limit=0 --query "file dataset=%s"' % dataset).read().split()

def createOutStructure ():
    transferDir = outdir if not storageTransfer else storageDir
    for dataset in datasets:
        if removeOld:
            os.system ('rm -rf %s/%s' % (transferDir, outDirName (dataset)))
        os.system ('mkdir -p %s/%s' % (transferDir, outDirName (dataset)))

def initProxy():
      print "You are going to run on a sample over grid using either CRAB or the AAA protocol, it is therefore needed to initialize your grid certificate"
      os.system('mkdir -p ~/x509_user_proxy; voms-proxy-init --voms cms -valid 192:00 --out ~/x509_user_proxy/x509_proxy')#all must be done in the same command to avoid environement problems.  Note that the first sourcing is only needed in Louvain


if sys.argv[1] == '1':
   initProxy ()
   createOutStructure()

   JobName = "dEdxSkimmer"
   FarmDirectory = "FARM2"
   os.system('rm -rf %s' % FarmDirectory)
   LaunchOnCondor.SendCluster_Create(FarmDirectory, JobName)
   LaunchOnCondor.Jobs_Queue = '8nh'
   LaunchOnCondor.Jobs_InitCmds   = ['export HOME=%s' % os.environ['HOME'], 'export X509_USER_PROXY=$HOME/x509_user_proxy/x509_proxy']

   for dataset in datasets:
      datasetMark = outDirName (dataset)
      print '===========================================================\n%s\n' % datasetMark
      Files = getDatasetFiles(dataset)
      for i in range (0, len(Files)):
         os.system ('cp dEdxSkimmer_Template_cfg.py dEdxSkimmer_cff.py')
         f = open ('dEdxSkimmer_cff.py', 'a')
         f.write ('\n')
         f.write ('process.Out.fileName = cms.untracked.string(\'dEdxSkim_%s_%i.root\')\n' % (datasetMark, i))
         f.write ('process.source.fileNames.extend([\'%s/%s\'])\n' % (server,Files[i]))
         f.close()
         if storageTransfer:
            LaunchOnCondor.Jobs_FinalCmds = ["gfal-copy file:////${PWD}/dEdxSkim_%s_%i.root srm://ingrid-se02.cism.ucl.ac.be:8444/srm/managerv2\?SFN=%s/%s/dEdxSkim_%s_%i.root && rm -f dEdxSkim_%s_i%.root" % (datasetMark, i, storageDir, datasetMark, datasetMark, i, datasetMark, i)] # if you do not use zsh, perhaps change '\?' to '?'
         else:
            LaunchOnCondor.Jobs_FinalCmds = ['mv dEdxSkim*.root %s/%s/%s/' % (os.getcwd(), outdir, datasetMark)]
         LaunchOnCondor.SendCluster_Push (["CMSSW", "dEdxSkimmer_cff.py"])
         os.system ('rm -f dEdxSkimmer_cff.py')
   LaunchOnCondor.SendCluster_Submit ()
