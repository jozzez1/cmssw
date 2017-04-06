import FWCore.ParameterSet.Config as cms

process = cms.Process("FedCablingReader")

process.MessageLogger = cms.Service("MessageLogger",
    debugModules = cms.untracked.vstring(''),
#    cablingReader = cms.untracked.PSet(
#        threshold = cms.untracked.string('INFO')
#    ),
    destinations = cms.untracked.vstring('cablingReader.log')
)

process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:run2_data', '')

process.source = cms.Source("EmptySource",
    numberEventsInRun = cms.untracked.uint32(1),
    firstRun = cms.untracked.uint32(280870)
)

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(1)
)

process.poolDBESSource = cms.ESSource("PoolDBESSource",
    timetype = cms.untracked.string('runnumber'),
    connect = cms.string('sqlite_file:testNewCabling.db'),
#    connect = cms.string('frontier://FrontierProd/CMS_CONDITIONS'),
    toGet = cms.VPSet(cms.PSet(
        record = cms.string('SiStripFedCablingRcd'),
#        tag = cms.string('SiStripFedCabling_GR10_v1_hlt')
        tag = cms.string('testNewCabling_tag')
    ))
)

#process.es_prefer = cms.ESPrefer('SiStripConnectivity','sistripconn')
process.es_prefer = cms.ESPrefer('PoolDBESSource','poolDBESSource')

process.load("Configuration.StandardSequences.GeometryDB_cff")
#process.TrackerDigiGeometryESModule.applyAlignment = False
#process.SiStripConnectivity = cms.ESProducer("SiStripConnectivity")
#process.SiStripRegionConnectivity = cms.ESProducer("SiStripRegionConnectivity",
#                                                   EtaDivisions = cms.untracked.uint32(20),
#                                                   PhiDivisions = cms.untracked.uint32(20),
#                                                   EtaMax = cms.untracked.double(2.5)
#)

process.fedcablingreader = cms.EDAnalyzer("SiStripFedCablingReader",
                                        PrintFecCabling = cms.untracked.bool(True),
 #                                       PrintDetCabling = cms.untracked.bool(True),
                                        PrintRegionCabling = cms.untracked.bool(True)
)

process.p1 = cms.Path(process.fedcablingreader)


