from WMCore.Configuration import Configuration

config = Configuration()

config.section_("General")
config.General.requestName = "PromptJpsiEmbedded"
config.General.workArea = 'crab_projects'
config.General.transferOutputs = True
config.General.transferLogs = False

config.section_("JobType")
config.JobType.pluginName = "Analysis"
config.JobType.psetName = "hioniaanalyzer_PbPbPrompt_MC_cfg.py"
config.JobType.maxMemoryMB = 2000         # request high memory machines.
#config.JobType.numCores = 4
config.JobType.allowUndistributedCMSSW = True #Problems with slc7
config.JobType.maxJobRuntimeMin = 1200 #2750    # request longer runtime, ~48 hours.


config.section_("Data")
config.Data.inputDataset = '/DrellYan_HighMass_MadGraph_HydjetEmbedded_1610pre3/fdamas-PATwith161pre4_151X_mcRun3_2025_realistic_HI_v5-eaa0399b9218a690ee453ab5f1aeb831/USER'
config.Data.inputDBS = 'phys03'
config.Data.unitsPerJob = 40
#config.Data.totalUnits = -1
config.Data.splitting = "FileBased"
config.Data.allowNonValidInputDataset = True
config.Data.outputDatasetTag = config.General.requestName

config.Data.outLFNDirBase = '/store/user/fdamas/PPRef2024/RunPrepMC/'
config.Data.publication = False

config.section_("Site")
config.Site.storageSite = "T3_CH_CERNBOX"
config.Site.whitelist = ["T2_US_Vanderbilt"]
