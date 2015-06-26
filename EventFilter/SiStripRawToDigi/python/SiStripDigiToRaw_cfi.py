import FWCore.ParameterSet.Config as cms

SiStripDigiToRaw = cms.EDProducer(
    "SiStripDigiToRawModule",
    InputModuleLabel = cms.string('simSiStripDigis'),
    InputDigiLabel = cms.string('ZeroSuppressed'),
    FedReadoutMode = cms.string('ZERO_SUPPRESSED'),
    #FedReadoutMode = cms.string('ZERO_SUPPRESSED_TOPBOT'),
    UseFedKey = cms.bool(False),
    UseWrongDigiType = cms.bool(False)
    )

