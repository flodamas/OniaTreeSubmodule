import FWCore.ParameterSet.Config as cms
import FWCore.ParameterSet.VarParsing as VarParsing
from Configuration.StandardSequences.Eras import eras

#----------------------------------------------------------------------------

globalTag = '151X_dataRun3_Prompt_v1'

isMC           = False # if input is MONTECARLO: True or if it's DATA: False
muonSelection  = "Tight" # Single muon selection: All, Glb(isGlobal), GlbTrk(isGlobal&&isTracker), Trk(isTracker), GlbOrTrk, Tight are available
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

addMuonIsolation = True
#----------------------------------------------------------------------------

# Print Onia Tree settings:
print( " " )
print( "[INFO] Settings: " )
print( "[INFO] isMC                 = " + ("True" if isMC else "False") )
print( "[INFO] applyEventSel        = " + ("True" if applyEventSel else "False") )
print( "[INFO] applyCuts            = " + ("True" if applyCuts else "False") )
print( "[INFO] SumETvariables       = " + ("True" if SumETvariables else "False") )
print( "[INFO] muonSelection        = " + muonSelection )
print( "[INFO] atLeastOneCand       = " + ("True" if atLeastOneCand else "False") )
print( "[INFO] OneMatchedHLTMu      = " + ("True" if OneMatchedHLTMu > -1 else "False") )
print( "[INFO] miniAOD              = " + ("True" if miniAOD else "False") )
print( "[INFO] UsePropToMuonSt      = " + ("True" if UsePropToMuonSt else "False") )
print( "[INFO] addMuonIsolation     = " + ("True" if addMuonIsolation else "False") )
print( "[INFO] addEventPlane        = " + ("True" if addEventPlane else "False") )

print( " " )

# set up process
process = cms.Process("HIOnia", eras.Run3_pp_on_PbPb_2025)

# setup 'analysis'  options
options = VarParsing.VarParsing ('analysis')

# Input and Output File Name

options.inputFiles = [
  'root://cmsxrootd.fnal.gov//store/hidata/HIRun2025A/HIPhysicsRawPrime24/MINIAOD/PbPbEW-PromptReco-v1/000/399/710/00000/35e71205-b440-4689-8689-d22475f4835f.root',
  'root://cmsxrootd.fnal.gov//store/hidata/HIRun2025A/HIPhysicsRawPrime24/MINIAOD/PbPbEW-PromptReco-v1/000/400/339/00000/b72d5207-f9e4-4904-af9b-a8b25045a2b4.root',
  'root://cmsxrootd.fnal.gov//store/hidata/HIRun2025A/HIPhysicsRawPrime24/MINIAOD/PbPbEW-PromptReco-v1/000/400/357/00000/e14979db-32ec-4842-8972-c3765fc27ceb.root',
  'root://cmsxrootd.fnal.gov//store/hidata/HIRun2025A/HIPhysicsRawPrime40/MINIAOD/PbPbEW-PromptReco-v1/000/399/767/00000/527cf17f-fd7d-40bd-8514-774afe51d260.root',
  'root://cmsxrootd.fnal.gov//store/hidata/HIRun2025A/HIPhysicsRawPrime40/MINIAOD/PbPbEW-PromptReco-v1/000/399/925/00000/a25645c7-8674-488b-9080-75c2aab48406.root',
  'root://cmsxrootd.fnal.gov//store/hidata/HIRun2025A/HIPhysicsRawPrime40/MINIAOD/PbPbEW-PromptReco-v1/000/400/059/00000/2229e9e9-c9ad-43e7-8b04-804c33db15bf.root',
  'root://cmsxrootd.fnal.gov//store/hidata/HIRun2025A/HIPhysicsRawPrime53/MINIAOD/PbPbEW-PromptReco-v1/000/400/357/00000/39c56d61-14a0-46e0-9fab-1911f4a61ded.root',
  'root://cmsxrootd.fnal.gov//store/hidata/HIRun2025A/HIPhysicsRawPrime53/MINIAOD/PbPbEW-PromptReco-v1/000/400/387/00000/e2127a1e-cbc8-416d-8bca-b4a7eefdf318.root',
  'root://cmsxrootd.fnal.gov//store/hidata/HIRun2025A/HIPhysicsRawPrime53/MINIAOD/PbPbEW-PromptReco-v1/000/400/426/00000/7dbf9fca-be00-453e-9416-64a75308016d.root',
  'root://cmsxrootd.fnal.gov//store/hidata/HIRun2025A/HIPhysicsRawPrime11/MINIAOD/PbPbEW-PromptReco-v1/000/399/720/00000/59f38378-d28c-43d5-81ff-b44d49d587e0.root',
  'root://cmsxrootd.fnal.gov//store/hidata/HIRun2025A/HIPhysicsRawPrime11/MINIAOD/PbPbEW-PromptReco-v1/000/399/937/00000/65d0ec45-53d4-44b9-8d60-57fc4d72776c.root'
]

options.outputFile = 'DimuonTree_HighPtMuons_PbPb2025_Data.root'
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
                        "HLT_HIL2SingleMu3_Open_v",#4
                        "HLT_HIL2SingleMu5_v",#5
                        "HLT_HIL2SingleMu7_v",#6
                        "HLT_HIL2SingleMu12_v",#7
			)
}


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

process.onia2MuMuPatGlbGlb.dimuonSelection       = cms.string("mass > 2.4 && abs(daughter('muon1').innerTrack.dz - daughter('muon2').innerTrack.dz) < 20")
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

process.hionia.addMuonIsolation = cms.bool(addMuonIsolation)

process.hionia.storeSameSign = cms.bool(True)

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
                                    cut = cms.string("mass > 2.4"),
                                    decay = cms.string("muonSelector muonSelector")
                                    )

  process.atLeastOneDimuon = cms.EDFilter("CandViewCountFilter",
                                        src = cms.InputTag("dimuonSelection"),
                                        minNumber = cms.uint32(1)
                                        )
  
  process.oniaTreeAna.replace(process.patMuonSequence,process.muonSelector * process.atLeastTwoMuons * process.dimuonSelection * process.atLeastOneDimuon * process.phfCoincFilterPF2Th4 * process.primaryVertexFilter * process.hltHI * process.patMuonSequence )

# needed for muon isolation
process.oniaTreeAna.replace(process.patMuonSequence, process.centralityBin * process.patMuonSequence )

if atLeastOneCand:
  process.oniaTreeAna.replace(process.onia2MuMuPatGlbGlb, process.onia2MuMuPatGlbGlb * process.onia2MuMuPatGlbGlbFilter)
  #BEWARE, pseudoDimuonFilterSequence asks for opposite-sign dimuon in given mass range. But saves a lot of time by filtering before running PAT muons
  process.oniaTreeAna.replace(process.patMuonSequence, process.pseudoDimuonFilterSequence * process.patMuonSequence)

process.oniaTreeAna = cms.Path(process.oniaTreeAna)
if miniAOD:
  from HiSkim.HiOnia2MuMu.onia2MuMuPAT_cff import changeToMiniAOD
  changeToMiniAOD(process, addIsolation = addMuonIsolation)
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

#process.options.numberOfThreads = 4


process.schedule  = cms.Schedule( process.oniaTreeAna )
