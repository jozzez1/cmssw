import FWCore.ParameterSet.Config as cms

SiStripClusterChargeCutNone = cms.PSet(
    value     = cms.double(-1.0)
)
  
SiStripClusterChargeCutLoose = cms.PSet(
    value     = cms.double(860.0) 
)

SiStripClusterChargeCutTight = cms.PSet(
    value     = cms.double(972.0) 
)

