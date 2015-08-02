from WMCore.Configuration import Configuration
config = Configuration()
config.section_('General')
config.General.requestName = 'DeDxSkimmerMC'
config.General.transferOutputs = True
config.section_('JobType')
config.JobType.psetName = 'dEdxSkimmer_Template_cfg.py'
config.JobType.pluginName = 'Analysis'
#config.JobType.inputFiles = ['Data7TeV_Deco_SiStripDeDxMip_3D_Rcd.db']
#config.JobType.pyCfgParams = ['globalTag=GR_P_V56'] # it is read from the skimmer template
#config.JobType.pyCfgParams = ['globalTag=MCRUN2_74_V7']
config.JobType.outputFiles = ['dEdxSkim.root']
config.section_('Data')
#config.Data.inputDataset = '/ZeroBias/Run2015B-PromptReco-v1/RECO' # data sample
#config.Data.inputDataset = '/RelValZMM_13/CMSSW_7_4_0_pre9-MCRUN2_74_V7-v1/GEN-SIM-RECO' # MC sample
config.Data.inputDataset = '/RelValMinBias_13/CMSSW_7_4_6_patch6-MCRUN2_74_V9_scheduled-v1/GEN-SIM-RECO'
config.Data.publication = False
#config.Data.splitting = 'LumiBased'
#config.Data.lumiMask = 'Golden_JSON_Run251252.txt'
config.Data.splitting = 'FileBased'
config.Data.unitsPerJob = 1 # No. of lumisections/files each job works through
config.Data.inputDBS = 'global'
#config.Data.runRange = '100000-900000'
config.section_('User')
config.section_('Site')
config.Site.storageSite = 'T2_BE_UCL'
