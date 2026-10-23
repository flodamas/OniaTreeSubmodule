import FWCore.ParameterSet.Config as cms
import FWCore.ParameterSet.VarParsing as VarParsing
from Configuration.StandardSequences.Eras import eras

#----------------------------------------------------------------------------

globalTag = '141X_mcRun3_2024_realistic_ppRef5TeV_v7'

isMC           = True # if input is MONTECARLO: True or if it's DATA: False
muonSelection  = "All" # Single muon selection: All, Glb(isGlobal), GlbTrk(isGlobal&&isTracker), Trk(isTracker), GlbOrTrk
applyEventSel  = True # Only apply Event Selection if the required collections are present
applyCuts      = False # At HiAnalysis level, apply kinematic acceptance cuts + identification cuts (isSoftMuon (without highPurity) or isTightMuon, depending on TightGlobalMuon flag) for muons from selected di(tri)muons + hard-coded cuts on the di(tri)muon that you would want to add (but recommended to add everything in LateDimuonSelection, applied at the end of HiSkim)
SumETvariables = False  # Whether to write out SumET-related variables
atLeastOneCand = False # Keep only events that have one selected dimuon. BEWARE this can cause trouble in .root output if no event is selected by onia2MuMuPatGlbGlbFilter!
OneMatchedHLTMu = -1   # Keep only dimuons of which the one(two) muon(s) are matched to the HLT Filter of this number. You can get the desired number in the output of oniaTree. Set to -1 for no matching.
#############################################################################
miniAOD        = True # whether the input file is in miniAOD format (default is AOD)
UsePropToMuonSt = True # whether to use L1 propagated muons (works only for miniAOD now)
pdgId = 23 # J/Psi : 443, Y(1S) : 553, Z : 23

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

print( " " )

# set up process
process = cms.Process("HIOnia", eras.Run3_2024_ppRef)

# setup 'analysis'  options
options = VarParsing.VarParsing ('analysis')

# Input and Output File Name
options.outputFile = "Powheg_ZZto4L_ppRef2024MC.root"
options.secondaryOutputFile = "Jpsi_DataSet.root"
options.inputFiles =[
  'root://cmsxrootd.fnal.gov//store/mc/RunIIIpp5p36Winter24MiniAOD/ZZTo4L_TuneCP5_5p36TeV_powheg-pythia8/MINIAODSIM/141X_mcRun3_2024_realistic_ppRef5TeV_v7-v2/2820000/02eb0721-abc1-4411-ae12-cc0499ad29f0.root'
]
options.maxEvents = -1 # -1 means all events

# Get and parse the command line arguments
options.parseArguments()

triggerList    = {
		# Double Muon Trigger List
		'DoubleMuonTrigger' : cms.vstring(
			      "HLT_PPRefL1DoubleMu0_Open_v",
            "HLT_PPRefL1DoubleMu0_v",
            "HLT_PPRefL1DoubleMu0_SQ_v",
            "HLT_PPRefL2DoubleMu0_Open_v",
            "HLT_PPRefL2DoubleMu0_v",
            ),
        # Single Muon Trigger List
        'SingleMuonTrigger' : cms.vstring(
            "HLT_PPRefL1SingleMu7_v",
            "HLT_PPRefL1SingleMu12_v",
            "HLT_PPRefL2SingleMu7_v",
            "HLT_PPRefL2SingleMu12_v",
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


#----------------------------------------------------------------------------

# For OniaTree Analyzer
from HiAnalysis.HiOnia.oniaTreeAnalyzer_cff import oniaTreeAnalyzer
oniaTreeAnalyzer(process,
                 muonTriggerList=triggerList,
                 muonSelection=muonSelection, L1Stage=2, isMC=isMC, pdgID=pdgId, outputFileName=options.outputFile
)

process.onia2MuMuPatGlbGlb.dimuonMassHypothesis  = cms.double(91.19)
process.onia2MuMuPatGlbGlb.dimuonSelection       = cms.string("mass > 40")
process.onia2MuMuPatGlbGlb.lowerPuritySelection  = cms.string("pt > 10.")

if applyCuts:
  process.onia2MuMuPatGlbGlb.LateDimuonSel         = cms.string("userFloat(\"vProb\")>0.01")

#process.hionia.CentralitySrc    = cms.InputTag("hiCentrality")
#process.hionia.CentralityBinSrc = cms.InputTag("centralityBin","HFtowers")
#process.hionia.muonLessPV       = cms.bool(False)
process.hionia.SumETvariables   = cms.bool(SumETvariables)
process.hionia.applyCuts        = cms.bool(applyCuts)
process.hionia.AtLeastOneCand   = cms.bool(atLeastOneCand)
process.hionia.OneMatchedHLTMu  = cms.int32(OneMatchedHLTMu)
process.hionia.checkTrigNames   = cms.bool(False)#change this to get the event-level trigger info in hStats output (but creates lots of warnings when fake trigger names are used)
process.hionia.genealogyInfo    = cms.bool(True)
process.hionia.isHI = cms.untracked.bool(False)

process.hionia.addMuonIsolation = cms.bool(addMuonIsolation)

if applyEventSel:
  process.load('HeavyIonsAnalysis.EventAnalysis.collisionEventSelection_cff')
  process.oniaTreeAna.replace(process.patMuonSequence, process.primaryVertexFilter * process.patMuonSequence )

process.oniaTreeAna = cms.Path(process.oniaTreeAna)
if miniAOD:
  from HiSkim.HiOnia2MuMu.onia2MuMuPAT_cff import changeToMiniAOD
  changeToMiniAOD(process)
  process.unpackedMuons.addPropToMuonSt = cms.bool(UsePropToMuonSt)

  if applyEventSel:
    process.oniaTreeAna.replace(process.hionia, process.beamScrapingFilter * process.hionia ) # must be called after unpacking

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

process.schedule  = cms.Schedule( process.oniaTreeAna )
