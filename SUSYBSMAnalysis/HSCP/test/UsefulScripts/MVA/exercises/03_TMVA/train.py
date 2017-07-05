#!/usr/bin/env python
#----------------------------------------------------------------------
# File: train.py
#1;95;0c Description: example of classification with TMVA
# Created: 01-June-2013 INFN SOS 2013, Vietri sul Mare, Italy, HBP
#   adapted for CMSDAS 2015 Bari HBP
#----------------------------------------------------------------------
import os, sys, re
from math import *
from string import *
from time import sleep
from array import array
from histutil import *
from ROOT import *
#----------------------------------------------------------------------
def getTree(filename, treename):
    hfile = TFile(filename)
    if not hfile.IsOpen():
        print "** can't open file %s" % filename
        sys.exit()
    tree = hfile.Get(treename)
    if tree == None:
        print "** can't find tree %s" % treename
        sys.exit()
    return (hfile, tree)
#----------------------------------------------------------------------
#----------------------------------------------------------------------
def main():
    print "\n", "="*80
    print "\tclassification with TMVA"
    print "="*80

    sigvsig = False  #True = Two signal files (one 'sig', one 'bkg')   False = Normal one sig and one bkg
    
    IasvPt = False  #True = IasvsPt   False = IhvsP

    Type2 = False #Type2 ntuples?

    DYswitch = False

    #signame ="DY_13TeV_M600_Q1_DY_13TeV_M600_Q3"
    signame = "Gluino_13TeV_M1400_f10"
    signameshort ="DY_13TeV_M600_Q1"
    

    sigtreename = "%sTRAIN" %signame
    if(DYswitch):sigtreename = "%sTRAIN"%signameshort

    bkgtreename = "Data"

    # get signal and background data for training/testing
    sigfilename = '../../../Data/Type0/TRAIN/Histos_%s_%sTRAIN.root' %(signame,signame)
    bkgfilename = '../../../Data/Type0TOY/Data13TeV_Run2015DregionTOY_TRAIN.root'

    # get the signal and background trees
    sigFile, sigTree = getTree(sigfilename, sigtreename)
    bkgFile, bkgTree = getTree(bkgfilename, bkgtreename)
    
    typename = "IhvP"
    
    # everything is done via a TMVA factory
    outputFile = TFile("TMVA_%s_%s.root"%(signame,typename), "recreate")
    factory = TMVA.Factory("%s_%s"%(signame,typename), outputFile,
                           "!V:Transformations=I;N;D")
    if(sigvsig==True):
        factory = TMVA.Factory("%s_%s_sigvsig"%(signame,typename),outputFile,
                              "!V:Transformations=I;N;D")

# execute the following if you want to test if a variable is found in a tree
#    sigTree.Scan("variablename")
#    print "ok1"
#    exit(0)

    # define input variables
    factory.AddVariable("P", 'F')
    factory.AddVariable("Ih", 'F')
    factory.AddVariable("eta", 'F')
    if(Type2==True):
        factory.AddVariable("TOF", 'F')

    # define from which trees data are to be taken
    factory.AddSignalTree(sigTree)
    factory.AddBackgroundTree(bkgTree)

    # remove problematic events and specify how
    # many events are to be used
    # for training and testing
    factory.PrepareTrainingAndTestTree(TCut("P>0.0"),
                                       "nTrain_Signal=10000:"\
                                       "nTest_Signal=1000:"\
                                       "nTrain_Background=10000:"\
                                       "nTest_Background=1000:"\
                                       "!V" )

    # define multivariate methods to be run
    factory.BookMethod( TMVA.Types.kMLP,
                        "MLP",
                        "!H:!V:"\
                        "VarTransform=N:"\
                        "HiddenLayers=10"\
                        "TrainingMethod=BFGS")

    #factory.BookMethod( TMVA.Types.kBDT,
    #                    "BDT",
    #                    "!V:"\
    #                    "BoostType=AdaBoost:"\
    #                    "NTrees=500:"\
    #                    "nEventsMin=100:"\
    #                    "nCuts=50")
  
    factory.TrainAllMethods()  
    factory.TestAllMethods()
    factory.EvaluateAllMethods()
    
    outputFile.Close()
#----------------------------------------------------------------------
try:
    main()
except KeyboardInterrupt:
    print "\nciao"
