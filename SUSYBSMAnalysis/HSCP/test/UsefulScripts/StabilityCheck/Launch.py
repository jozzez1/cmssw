#!/usr/bin/env python

import urllib
import string
import os
import sys
import SUSYBSMAnalysis.HSCP.LaunchOnCondor as LaunchOnCondor
import glob

print 'OPTIMIZATION'
FarmDirectory = "FARM"
JobName = "HSCPStability"
LaunchOnCondor.Jobs_RunHere = 1
LaunchOnCondor.SendCluster_Create(FarmDirectory, JobName)
#LaunchOnCondor.SendCluster_Push(["FWLITE", os.getcwd()+"/StabilityCheck.C", '"ANALYSE"'])
LaunchOnCondor.SendCluster_Push(["BASH", "sh " + os.getcwd()+"/StabilityCheck.sh"])
LaunchOnCondor.SendCluster_Submit()
