import FWCore.ParameterSet.Config as cms
import FWCore.ParameterSet.VarParsing as VarParsing
from Configuration.StandardSequences.Eras import eras

#----------------------------------------------------------------------------

# Setup Settings for ONIA TREE: 2025 PbPb data

isMC           = False # if input is MONTECARLO: True or if it's DATA: False
muonSelection  = "Glb" # Single muon selection: All, Glb(isGlobal), GlbTrk(isGlobal&&isTracker), Trk(isTracker), GlbOrTrk are available
applyEventSel  = True # Only apply Event Selection if the required collections are present
applyCuts      = False # At HiAnalysis level, apply kinematic acceptance cuts + identification cuts (isSoftMuon (without highPurity) or isTightMuon, depending on TightGlobalMuon flag) for muons from selected di(tri)muons + hard-coded cuts on the di(tri)muon that you would want to add (but recommended to add everything in LateDimuonSelection, applied at the end of HiSkim)
SumETvariables = True  # Whether to write out SumET-related variables
atLeastOneCand = False # Keep only events that have one selected dimuon. BEWARE this can cause trouble in .root output if no event is selected by onia2MuMuPatGlbGlbFilter!
OneMatchedHLTMu = -1   # Keep only di(tri)muons of which the one(two) muon(s) are matched to the HLT Filter of this number. You can get the desired number in the output of oniaTree. Set to -1 for no matching.
#############################################################################
miniAOD        = True # whether the input file is in miniAOD format (default is AOD)
UsePropToMuonSt = True # whether to use L1 propagated muons (works only for miniAOD now)
pdgId = 443 # J/Psi : 443, Y(1S) : 553

addEventPlane = False
#----------------------------------------------------------------------------

# Print Onia Tree settings:
print( " " )
print( "[INFO] Settings used for ONIA TREE: " )
print( "[INFO] isMC                 = " + ("True" if isMC else "False") )
print( "[INFO] applyEventSel        = " + ("True" if applyEventSel else "False") )
print( "[INFO] applyCuts            = " + ("True" if applyCuts else "False") )
print( "[INFO] SumETvariables       = " + ("True" if SumETvariables else "False") )
print( "[INFO] muonSelection        = " + muonSelection )
print( "[INFO] atLeastOneCand       = " + ("True" if atLeastOneCand else "False") )
print( "[INFO] OneMatchedHLTMu      = " + ("True" if OneMatchedHLTMu > -1 else "False") )
print( "[INFO] miniAOD              = " + ("True" if miniAOD else "False") )
print( "[INFO] UsePropToMuonSt      = " + ("True" if UsePropToMuonSt else "False") )
print( "[INFO] addEventPlane        = " + ("True" if addEventPlane else "False") )

print( " " )

# set up process
process = cms.Process("HIOnia", eras.Run3_pp_on_PbPb_2025)

# setup 'analysis'  options
options = VarParsing.VarParsing ('analysis')

# Input and Output File Name

options.inputFiles = [
  'root://cmsxrootd.fnal.gov//store/hidata/HIRun2025A/HIPhysicsRawPrime1/MINIAOD/PbPbEW-PromptReco-v1/000/399/589/00000/97032cc3-16bb-4f43-8a25-a4c8d0aa8482.root',
  'root://cmsxrootd.fnal.gov//store/hidata/HIRun2025A/HIPhysicsRawPrime24/MINIAOD/PbPbEW-PromptReco-v1/000/399/710/00000/35e71205-b440-4689-8689-d22475f4835f.root',
  'root://cmsxrootd.fnal.gov//store/hidata/HIRun2025A/HIPhysicsRawPrime24/MINIAOD/PbPbEW-PromptReco-v1/000/399/710/00000/9aa7a097-b77f-44e5-943c-50310094cd41.root',
  'root://cmsxrootd.fnal.gov//store/hidata/HIRun2025A/HIPhysicsRawPrime24/MINIAOD/PbPbEW-PromptReco-v1/000/400/339/00000/b72d5207-f9e4-4904-af9b-a8b25045a2b4.root',
  'root://cmsxrootd.fnal.gov//store/hidata/HIRun2025A/HIPhysicsRawPrime24/MINIAOD/PbPbEW-PromptReco-v1/000/400/357/00000/e14979db-32ec-4842-8972-c3765fc27ceb.root'
]

options.outputFile = 'Oniatree_PbPb2025PromptRecoData.root'
options.secondaryOutputFile = "Jpsi_Dataset.root"

options.maxEvents = -1 # -1 means all events

# Get and parse the command line arguments
options.parseArguments()

triggerList    = {
		# Double Muon Trigger List
		'DoubleMuonTrigger' : cms.vstring(
                        "HLT_HIL1DoubleMu0_MaxDr3p5_Open_v",#0
                        "HLT_HIL1DoubleMu0_v",#1
                        "HLT_HIL1DoubleMu0_SQ_v",#2
                        "HLT_HIL2DoubleMu0_Open_v",#3
                        ),
                # Single Muon Trigger List
                'SingleMuonTrigger' : cms.vstring(
                        "HLT_HIL1SingleMu0_Open_v",#4
                        "HLT_HIL1SingleMu0_v",#5
                        "HLT_HIL2SingleMu3_Open_v",#6
                        "HLT_HIL2SingleMu5_v",#7
                        "HLT_HIL2SingleMu7_v",#8
                        "HLT_HIL2SingleMu12_v",#9
			)
}

# Global tag, see https://github.com/cms-sw/cmssw/blob/master/Configuration/AlCa/python/autoCond.py
if isMC:
  globalTag = 'auto:phase1_2025_realistic_hi'
else:
  globalTag = '151X_dataRun3_Prompt_v1'

#----------------------------------------------------------------------------

# load the Geometry and Magnetic Field
process.load('Configuration.StandardSequences.Reconstruction_cff')
process.load('Configuration.StandardSequences.Services_cff')
process.load('Configuration.Geometry.GeometryDB_cff')
process.load('Configuration.StandardSequences.MagneticField_38T_cff')

# Global Tag:
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, globalTag, '')

### For Centrality
process.load("RecoHI.HiCentralityAlgos.CentralityBin_cfi")
process.centralityBin.Centrality = cms.InputTag("hiCentrality")
process.centralityBin.centralityVariable = cms.string("HFtowers")
print('\n\033[31m~*~ USING NOMINAL CENTRALITY TABLE FOR 2024 PbPb DATA ~*~\033[0m\n')
process.GlobalTag.snapshotTime = cms.string("9999-12-31 23:59:59.000")
process.GlobalTag.toGet.extend([
    cms.PSet(record = cms.string("HeavyIonRcd"),
        tag = cms.string("CentralityTable_HFtowers200_DataPbPb_periHYDJETshape_run3v140x01_offline_Nominal"),
        connect = cms.string("frontier://FrontierProd/CMS_CONDITIONS"),
        label = cms.untracked.string("HFtowers")
        ),
    ])

#----------------------------------------------------------------------------

# For OniaTree Analyzer
from HiAnalysis.HiOnia.oniaTreeAnalyzer_cff import oniaTreeAnalyzer
oniaTreeAnalyzer(process,
                 muonTriggerList=triggerList,
                 muonSelection=muonSelection, L1Stage=2, isMC=isMC, pdgID=pdgId, outputFileName=options.outputFile
)

process.onia2MuMuPatGlbGlb.dimuonSelection       = cms.string("mass > 2.0 && abs(daughter('muon1').innerTrack.dz - daughter('muon2').innerTrack.dz) < 25")
process.onia2MuMuPatGlbGlb.lowerPuritySelection  = cms.string("pt > 10.0 && abs(eta) < 2.4 && isGlobalMuon")

#if applyCuts:
process.onia2MuMuPatGlbGlb.LateDimuonSel = cms.string("userFloat(\"vProb\")>0.001")

process.hionia.CentralitySrc    = cms.InputTag("hiCentrality")
process.hionia.CentralityBinSrc = cms.InputTag("centralityBin","HFtowers")
process.hionia.SumETvariables   = cms.bool(SumETvariables)
process.hionia.applyCuts        = cms.bool(applyCuts)
process.hionia.AtLeastOneCand   = cms.bool(atLeastOneCand)
process.hionia.OneMatchedHLTMu  = cms.int32(OneMatchedHLTMu)
process.hionia.checkTrigNames   = cms.bool(False)#change this to get the event-level trigger info in hStats output (but creates lots of warnings when fake trigger names are used)

process.hionia.useEvtPlane      = cms.untracked.bool(addEventPlane)

process.hionia.storeSameSign = cms.bool(True)

process.oniaTreeAna.replace(process.hionia, process.centralityBin * process.hionia )

if applyEventSel:
  # Offline event filters
  process.load('HeavyIonsAnalysis.EventAnalysis.collisionEventSelection_cff')
  process.load('HeavyIonsAnalysis.EventAnalysis.hffilter_cfi')
  process.load('HeavyIonsAnalysis.EventAnalysis.hffilterPF_cfi')
  
  # HLT trigger firing events
  import HLTrigger.HLTfilters.hltHighLevel_cfi
  process.hltHI = HLTrigger.HLTfilters.hltHighLevel_cfi.hltHighLevel.clone()
  process.hltHI.HLTPaths = ["HLT_HIL2SingleMu*_v*", "HLT_HIMinimumBiasHF1AND*_v*"]
  process.hltHI.throw = False
  process.hltHI.andOr = True

  # Muon filtering

  MUONCUT = "isGlobalMuon && pt > 10.0 && abs(eta) < 2.4"
  
  process.muonSelector = cms.EDFilter("PATMuonRefSelector",
                                        src = cms.InputTag("slimmedMuons"),
                                        cut = cms.string(MUONCUT),
                                        filter = cms.bool(True)
  )

  process.atLeastTwoMuons = cms.EDFilter("MuonRefPatCount",
                                 src = cms.InputTag("slimmedMuons"),
                                cut = cms.string(MUONCUT),
                                 minNumber = cms.uint32(2)
                                 )

  process.dimuonSelection = cms.EDProducer("CandViewShallowCloneCombiner",
                                    checkCharge = cms.bool(False),
                                    cut = cms.string("mass > 2.0"),
                                    decay = cms.string("muonSelector muonSelector")
                                    )

  process.atLeastOneDimuon = cms.EDFilter("CandViewCountFilter",
                                        src = cms.InputTag("dimuonSelection"),
                                        minNumber = cms.uint32(1)
                                        )
  
  process.oniaTreeAna.replace(process.patMuonSequence,process.muonSelector * process.atLeastTwoMuons * process.dimuonSelection * process.atLeastOneDimuon * process.phfCoincFilterPF2Th4 * process.primaryVertexFilter * process.hltHI * process.patMuonSequence )

if atLeastOneCand:
  process.oniaTreeAna.replace(process.onia2MuMuPatGlbGlb, process.onia2MuMuPatGlbGlb * process.onia2MuMuPatGlbGlbFilter)
  #BEWARE, pseudoDimuonFilterSequence asks for opposite-sign dimuon in given mass range. But saves a lot of time by filtering before running PAT muons
  process.oniaTreeAna.replace(process.patMuonSequence, process.pseudoDimuonFilterSequence * process.patMuonSequence)

process.oniaTreeAna = cms.Path(process.oniaTreeAna)
if miniAOD:
  from HiSkim.HiOnia2MuMu.onia2MuMuPAT_cff import changeToMiniAOD
  changeToMiniAOD(process)
  process.unpackedMuons.addPropToMuonSt = cms.bool(UsePropToMuonSt)

#----------------------------------------------------------------------------
#Options:
process.source = cms.Source("PoolSource",
#process.source = cms.Source("NewEventStreamFileReader", # for streamer data
		fileNames = cms.untracked.vstring( options.inputFiles ),
		)
process.TFileService = cms.Service("TFileService",
		fileName = cms.string( options.outputFile )
		)
process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(options.maxEvents) )
process.options   = cms.untracked.PSet(wantSummary = cms.untracked.bool(True))

process.options.numberOfThreads = 4


process.schedule  = cms.Schedule( process.oniaTreeAna )
