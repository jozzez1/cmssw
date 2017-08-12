#!/bin/env python

import sys, os, string

def initProxy():
    print "You are going to run on a sample over grid using either CRAB or the AAA protocol, it is therefore needed to initialize your grid certificate"
    os.system('mkdir -p ~/x509_user_proxy; voms-proxy-init --voms cms -valid 192:00 --out ~/x509_user_proxy/x509_proxy')


AAAServerName = 'root://cms-xrd-global.cern.ch/'
datasetSet = ['/RelValProdTTbar_13/CMSSW_9_3_0_pre3-92X_mcRun2_asymptotic_v2-v1/MINIAODSIM',
        '/RelValTTbarLepton_13/CMSSW_9_3_0_pre3-SiStripCalMinBias-92X_upgrade2017_realistic_v10_resub-v1/ALCARECO',
        '/RelValTTbar_13/CMSSW_9_3_0_pre1-92X_mcRun2_asymptotic_v2-v1/GEN-SIM-RECO',
        '/RelValTTbar_13/CMSSW_9_2_7-TkAlZMuMu-92X_upgrade2017_realistic_v7-v2/ALCARECO',
        '/RelValProdMinBias/CMSSW_9_3_0_pre3-92X_mcRun1_realistic_v2-v1/GEN-SIM-RECO']

dataset = datasetSet[int(sys.argv[1])]

files2process = os.popen('das_client --limit=0 --query "file dataset=\''+dataset+'\'"').read().split()

arguments = '';
for i in range(0, len(files2process)-1):
   arguments += AAAServerName + files2process[i] + ","
arguments += AAAServerName + files2process[len(files2process)-1]


# the actual meat that executes stuff
command = 'sh RelValStudy.sh '+'\''+arguments+'\' '+'RelValResults_%i.root' % int(sys.argv[1])
initProxy()
#print command
os.system (command)
