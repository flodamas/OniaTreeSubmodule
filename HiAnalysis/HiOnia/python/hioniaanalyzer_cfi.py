import FWCore.ParameterSet.Config as cms

hionia = cms.EDAnalyzer('HiOniaAnalyzer',
                        #-- Collections
                        srcMuon          = cms.InputTag("patMuonsWithTrigger"),
                        srcMuonNoTrig    = cms.InputTag("patMuonsWithoutTrigger"),
                        srcDimuon        = cms.InputTag("onia2MuMuPatGlbGlb"),
                        srcSV            = cms.InputTag("inclusiveSecondaryVerticesLoose",""), # Name of SV collection
                        genParticles     = cms.InputTag("genParticles"),
                        EvtPlane         = cms.InputTag("hiEvtPlaneFlat"),
                        primaryVertexTag = cms.InputTag("offlinePrimaryVertices"),

                        triggerResultsLabel = cms.InputTag("TriggerResults","","HLT"),

                        CentralitySrc    = cms.InputTag(""),
                        CentralityBinSrc = cms.InputTag(""),

                        #-- Reco Details
                        useBeamSpot = cms.bool(False),
                        useRapidity = cms.bool(True),

                        #--
                        maxAbsZ = cms.double(24.0),

                        centralityRanges = cms.vdouble(20,40,100),

                        applyCuts = cms.bool(False),
			selTightGlobalMuon = cms.bool(False),
                        SumETvariables = cms.bool(True),
                        OneMatchedHLTMu = cms.int32(-1),
                        storeSameSign = cms.bool(True),
                        AtLeastOneCand = cms.bool(False),

                        genealogyInfo = cms.bool(False),
                        removeSignalEvents = cms.untracked.bool(False),
                        removeTrueMuons = cms.untracked.bool(False),
                        checkTrigNames     = cms.bool(True),  # Whether to names of the triggers given in the config

                        muonLessPV = cms.bool(False),
                        useSVfinder = cms.bool(False),

                        #-- Gen Details
                        oniaPDG = cms.int32(443),
                        muonSel = cms.string("GlbGlb"),
                        isHI = cms.untracked.bool(True),
                        isPA = cms.untracked.bool(False),
                        isMC = cms.untracked.bool(False),
                        isPromptMC = cms.untracked.bool(True),
                        useEvtPlane = cms.untracked.bool(False),
                        genOnly     = cms.bool(False),  # fill only generated info

                        #-- Histogram configuration
                        combineCategories = cms.bool(False),
                        fillTree = cms.bool(True),
                        fillHistos = cms.bool(False),
                        fillSingleMuons = cms.bool(True),
                        histFileName = cms.string("Jpsi_Histos.root"),
                        dataSetName = cms.string("Jpsi_DataSet.root"),

                        #--
                        dblTriggerPathNames = cms.vstring(),
                        sglTriggerPathNames = cms.vstring(),
                        stageL1Trigger = cms.uint32(True)
                        )
