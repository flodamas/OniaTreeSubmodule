from CRABAPI.RawCommand import crabCommand
from CRABClient.ClientExceptions import ClientException
from http.client import HTTPException

# We want to put all the CRAB project directories from the tasks we submit here into one common directory.
# That's why we need to set this parameter (here or above in the configuration file, it does not matter, we will not overwrite it).
from CRABClient.UserUtilities import config
config = config()

from CRABClient.UserUtilities import getUsername
username = getUsername()

##########################


config.section_("General")
config.General.workArea = 'crab_projects'
config.General.transferOutputs = True
config.General.transferLogs = False

config.section_("JobType")
config.JobType.pluginName = "Analysis"
config.JobType.psetName = "dimuonAnalyzer_PbPb2025_Data_cfg.py"

config.JobType.maxMemoryMB = 2400         # request high memory machines.
#config.JobType.numCores = 4
config.JobType.allowUndistributedCMSSW = True
config.JobType.maxJobRuntimeMin = 1200 # max = 2750

config.section_("Data")
config.Data.inputDBS = 'global'
#config.Data.totalUnits = -1
config.Data.splitting = "EventAwareLumiBased"
config.Data.unitsPerJob = 5000000

config.Data.allowNonValidInputDataset = True
config.Data.publication = False
config.Data.runRange = '399465-400426'
config.Data.lumiMask = 'https://cms-service-dqmdc.web.cern.ch/CAF/certification/Collisions25HI/Cert_Collisions2025_HI_399465_400426_Muon.json'

config.Data.outLFNDirBase = '/store/user/' + username + '/PbPb2025/'


config.section_("Site")
config.Site.storageSite = "T3_CH_CERNBOX"
#config.Site.whitelist = ["T2_US_*","T2_CH_CERN","T1_US_*"]

# Multi crab part

def submit(config):
    try:
        crabCommand('submit', config = config, dryrun=False)
    except HTTPException as hte:
        print("Failed submitting task: %s" % (hte.headers))
    except ClientException as cle:
        print("Failed submitting task: %s" % (cle))

# Submit the jobs: 60 PDs

for i in range(60):

    config.General.requestName = f'RawPrime{i}'
    config.Data.inputDataset = f"/HIPhysicsRawPrime{i}/HIRun2025A-PbPbEW-PromptReco-v1/MINIAOD"
    config.Data.outputDatasetTag = config.General.requestName

    print("Submitting CRAB job for: "+ config.Data.inputDataset)
    submit(config)