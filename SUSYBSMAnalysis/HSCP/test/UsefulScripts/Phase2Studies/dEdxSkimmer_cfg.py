

####################################
#        LaunchOnFarm Script       #
#     Loic.quertenmont@cern.ch     #
#            April 2010            #
####################################


import FWCore.ParameterSet.Config as cms

from Configuration.StandardSequences.Eras import eras

process = cms.Process("DEDXUNCSKIM", eras.Phase2C2_timing)

process.load("FWCore.MessageService.MessageLogger_cfi")
process.load('Configuration.Geometry.GeometryExtended2023D4Reco_cff')
process.load('Configuration.Geometry.GeometryExtended2023D4_cff')
process.load('Configuration.StandardSequences.MagneticField_38T_PostLS1_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_condDBv2_cff')
process.load('Configuration.StandardSequences.Services_cff')


process.load('Configuration.StandardSequences.RawToDigi_cff')
process.load('Configuration.StandardSequences.L1Reco_cff')
process.load('Configuration.StandardSequences.Reconstruction_cff')


process.options   = cms.untracked.PSet(
      wantSummary = cms.untracked.bool(True),
      SkipEvent = cms.untracked.vstring('ProductNotFound'),
)
process.MessageLogger.cerr.FwkReport.reportEvery = 1000

process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(-1) )
process.source = cms.Source("PoolSource",
   fileNames = cms.untracked.vstring(
   ),
   inputCommands = cms.untracked.vstring("keep *", "drop *_MEtoEDMConverter_*_*")
)


#process.GlobalTag.globaltag = GTAG
from Configuration.AlCa.GlobalTag_condDBv2 import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, "90X_upgrade2023_realistic_v1", '')


process.load('Alignment.CommonAlignmentProducer.AlignmentTrackSelector_cfi')
process.tracksForDeDx = process.AlignmentTrackSelector.clone(
    src = 'generalTracks',
    filter = True,
    applyBasicCuts = True,
    ptMin = 0.3,
    nHitMin = 6,
    chi2nMax = 10.,
    etaMin = cms.double(-5.0),
    etaMax = cms.double(+5.0),    
)

process.load('RecoVertex.BeamSpotProducer.BeamSpot_cff')
from RecoTracker.TrackProducer.TrackRefitter_cfi import *
process.RefitterForDeDx = TrackRefitter.clone(
      src = cms.InputTag("tracksForDeDx"),
      NavigationSchool = cms.string(""),
      TTRHBuilder = 'WithTrackAngle'
)

from RecoTracker.DeDx.dedxEstimators_cff import *
process.dedxHitInfo = dedxHitInfo.clone(
      tracks=cms.InputTag("RefitterForDeDx"),
      trajectoryTrackAssociation = cms.InputTag("RefitterForDeDx"),
      minTrackPt = cms.double(0.0),
      usePhase2Strip = cms.bool(True)
)

#make the pool output
process.Out = cms.OutputModule("PoolOutputModule",
     outputCommands = cms.untracked.vstring(
         # "keep *",
         "drop *",
         "keep EventAux_*_*_*",
         "keep LumiSummary_*_*_*",
         "keep *_RefitterForDeDx_*_DEDXUNCSKIM",
         "keep *_offlinePrimaryVertices_*_*",
         "keep *_siPixelClusters_*_*",
         "keep *_siPhase2Clusters_*_*",
         "keep *_dedxHitInfo_*_DEDXUNCSKIM",
         "keep *_genParticles_*_*",
    ),
    fileName = cms.untracked.string("dEdxSkim.root"),
    SelectEvents = cms.untracked.PSet(
       SelectEvents = cms.vstring('*')
    ),
)

#schedule the sequence
process.p = cms.Path(process.tracksForDeDx * process.offlineBeamSpot * process.RefitterForDeDx * process.dedxHitInfo)
#process.p = cms.Path(process.tracksForDeDx * process.offlineBeamSpot * process.dedxHitInfo)
process.endPath1 = cms.EndPath(process.Out)
process.schedule = cms.Schedule(process.p, process.endPath1)

# process.Out.fileName = cms.untracked.string('dEdxSkim_step3.root')
# process.source.fileNames.extend(['file:/afs/cern.ch/user/j/jpazzini/work/public/PhaseII/Files/step3.root'])

process.Out.fileName = cms.untracked.string('dEdxSkim_MinBias140.root')
process.source.fileNames.extend(['file:/afs/cern.ch/user/j/jpazzini/work/public/PhaseII/Files/MinBias140.root'])

#import PhysicsTools.PythonAnalysis.LumiList as LumiList
#process.source.lumisToProcess = LumiList.LumiList(filename = 'OneFillRuns.json').getVLuminosityBlockRange()
