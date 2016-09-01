import FWCore.ParameterSet.Config as cms
import Alignment.CommonAlignmentProducer.AlignmentTrackSelector_cfi

from RecoTracker.MeasurementDet.MeasurementTrackerEventProducer_cfi import *

# basic cuts
tracksForDeDx = Alignment.CommonAlignmentProducer.AlignmentTrackSelector_cfi.AlignmentTrackSelector.clone(
    src = 'generalTracks',
    filter = True, # originally was set to false
    applyBasicCuts = True,
    ptMin = 0.3,
    nHitMin = 6,
    chi2nMax = 10.,
)

from RecoVertex.BeamSpotProducer.BeamSpot_cff import *
from RecoTracker.TrackProducer.TrackRefitters_cff import *
RefitterForDeDx = TrackRefitter.clone(
      src = "tracksForDeDx",
      NavigationSchool = cms.string("")
)

from RecoTracker.DeDx.dedxEstimators_cff import *
dedxCalib = dedxHitInfo.clone()
dedxCalib.tracks=cms.InputTag("RefitterForDeDx")
dedxCalib.trajectoryTrackAssociation = cms.InputTag("RefitterForDeDx")
dedxCalib.minTrackPt = cms.double(0.0)

dedxSeq = cms.Sequence(tracksForDeDx * offlineBeamSpot * RefitterForDeDx * dedxCalib)

# define the output collections
EXOHSCPCalibSkim_EventContent=cms.PSet(
    outputCommands = cms.untracked.vstring(
      "drop *",
      "keep EventAux_*_*_*",
      "keep LumiSummary_*_*_*",
      "keep *_RefitterForDeDx_*_*",
      "keep *_offlinePrimaryVertices_*_*",
#      "keep *_siPixelClusters_*_DEDXUNCSKIM",
#      "keep *_siStripClusters_*_DEDXUNCSKIM",
#      "keep *_siPixelClusters_*_*",
      "keep *_siStripClusters_*_*",
      "keep *_dedxHitInfo_*_*",

      )
    )


