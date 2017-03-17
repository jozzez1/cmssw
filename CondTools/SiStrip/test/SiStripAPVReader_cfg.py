import FWCore.ParameterSet.Config as cms

process = cms.Process("SiStripAPVGainReader")

process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:run2_data', '')

process.MessageLogger = cms.Service("MessageLogger",
    cout = cms.untracked.PSet(
        threshold = cms.untracked.string('INFO')
    ),
    destinations = cms.untracked.vstring('SiStripAPVGainReader.log')
)


process.source = cms.Source("EmptySource",
    numberEventsInRun = cms.untracked.uint32(1),
    firstRun = cms.untracked.uint32(1)
)


process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(1)
)

process.poolDBESSource = cms.ESSource("PoolDBESSource",
    timetype = cms.untracked.string('runnumber'),
    connect = cms.string('sqlite_file://Gains_G1_279596.db'),
    toGet = cms.VPSet(cms.PSet(
        record = cms.string('SiStripSummaryRcd'),
        tag = cms.string('SiStripApvGainRcd_v1_hltvalidation')
        #tag = cms.string('SiStripApvGain_GR10_v1_hlt')
    ))
)

process.APVGainReader = cms.EDAnalyzer("SiStripAPVGainReader")
process.APVGainReader.printdebug  = cms.untracked.bool(True)
process.APVGainReader.outputFile  = cms.untracked.string('OldGains.log')
process.APVGainReader.gainType    = cms.untracked.uint32(0)

process.p1 = cms.Path(process.APVGainReader)


