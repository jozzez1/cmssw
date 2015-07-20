from WMCore.Configuration import Configuration
config = Configuration()
config.section_('General')
config.General.requestName = 'DeDxSkimmerNEW'
config.General.transferOutputs = True
config.section_('JobType')
config.JobType.psetName = 'dEdxSkimmer_Template_cfg.py'
config.JobType.pluginName = 'Analysis'
#config.JobType.inputFiles = ['Data7TeV_Deco_SiStripDeDxMip_3D_Rcd.db']
#config.JobType.pyCfgParams = ['globalTag=GR_P_V56'] # it is read from the skimmer template
config.JobType.outputFiles = ['dEdxSkim.root']
config.section_('Data')
config.Data.inputDataset = '/ZeroBias/Run2015B-PromptReco-v1/RECO'
config.Data.publication = False
config.Data.unitsPerJob = 5 # No. of lumisections each job works through
config.Data.splitting = 'LumiBased'
config.Data.inputDBS = 'global'
config.Data.lumiMask = 'Golden_JSON_Run251252.txt'
#config.Data.runRange = '100000-900000'
config.section_('User')
config.section_('Site')
config.Site.storageSite = 'T2_BE_UCL'
