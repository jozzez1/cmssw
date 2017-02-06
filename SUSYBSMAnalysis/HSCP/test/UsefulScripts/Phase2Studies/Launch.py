#!/usr/bin/env python

import string, os, sys

WriteLogfile = True

#toRun contains 3 steps: 1, 2 and 3
toRun = ["cmsDriver.py Configuration/GenProduction/python/ThirteenTeV/HSCPppstau_M_1599_TuneZ2star_13TeV_pythia6_cff.py --mc \
         --fileout file:step1.root \
         --eventcontent RAWSIM \
         --datatier GEN-SIM \
         --conditions 90X_upgrade2023_realistic_v1 \
         --beamspot HLLHC14TeV \
         --step GEN,SIM \
         --python_filename step1_cfg.py \
         --magField 38T_PostLS1 \
         --geometry Extended2023D4 \
         --era Phase2C2_timing \
         --customise SimG4Core/CustomPhysics/Exotica_HSCP_SIM_cfi.customise \
         --customise SimG4Core/Application/customiseSequentialSim.customiseSequentialSim \
         --no_exec \
         -n 10",

         "cmsDriver.py --filein file:step1.root --fileout step2.root --mc \
         --eventcontent FEVTDEBUGHLT --datatier GEN-SIM-DIGI-RAW \
         --conditions 90X_upgrade2023_realistic_v1 --step DIGI:pdigi_valid,L1,DIGI2RAW,HLT:@fake \
         --python_filename step2_cfg.py \
         --magField 38T_PostLS1 \
         --geometry Extended2023D4 \
         --era Phase2C2_timing \
         --beamspot HLLHC14TeV \
         --no_exec \
         -n -1",

         "cmsDriver.py --filein file:step2.root --fileout step3.root --mc \
          --eventcontent RECOSIM,AODSIM \
          --datatier GEN-SIM-RECO,AODSIM \
          --conditions 90X_upgrade2023_realistic_v1 --step RAW2DIGI,L1Reco,RECO \
          --python_filename step3_cfg.py \
          --magField 38T_PostLS1 \
          --geometry Extended2023D4 \
          --era Phase2C2_timing \
          --beamspot HLLHC14TeV \
          --runUnscheduled \
          --no_exec \
          -n -1"]

def error_msg ():
    print 'Did not specify the step!\n'
    print 'Correct program usage:\n'
    print '\t%s <step index>' % sys.argv[0]
    print ' where step index can be 1-%i' % len(toRun)
    raise SystemExit

if len(sys.argv) != 2:
    error_msg()
if int(sys.argv[1]) > 3 or int(sys.argv[1]) < 1:
    erorr_msg()

prepend_cmsenv = "eval `scramv1 runtime -sh`;"

#SCRAM_ARCH
if os.environ.get('SCRAM_ARCH') != 'slc6_amd64_gcc530':
   os.environ['SCRAM_ARCH']='slc6_amd64_gcc530'

end_commands = "; cmsRun step%s_cfg.py" % (sys.argv[1]) if not WriteLogfile else "; unbuffer cmsRun step%s_cfg.py 2>&1 | tee step%s.log" % (sys.argv[1], sys.argv[1])

#run the command
os.system('echo ' + toRun[int(sys.argv[1])-1] + '\'' + end_commands + '\'')
os.system(prepend_cmsenv + toRun[int(sys.argv[1])-1] + end_commands)
exit(0)

