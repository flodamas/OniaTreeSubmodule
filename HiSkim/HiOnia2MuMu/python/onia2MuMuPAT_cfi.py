import FWCore.ParameterSet.Config as cms

onia2MuMuPAT = cms.EDProducer('HiOnia2MuMuPAT',
        muons                    = cms.InputTag("patMuonsWithTrigger"),
        beamSpotTag              = cms.InputTag("offlineBeamSpot"),
        primaryVertexTag         = cms.InputTag("offlinePrimaryVertices"),
        srcTracks                = cms.InputTag("generalTracks"),
        genParticles             = cms.InputTag("genParticles"),
        # At least one muon must pass this selection
        higherPuritySelection    = cms.string(""), ## No need to repeat lowerPuritySelection in there, already included
        # BOTH muons must pass this selection
        lowerPuritySelection     = cms.string("((isGlobalMuon && isTrackerMuon) || (innerTrack.isNonnull && genParticleRef(0).isNonnull)) && abs(innerTrack.dxy)<4 && abs(innerTrack.dz)<35"),
        dimuonSelection          = cms.string(""), ## The dimuon must pass this selection before vertexing
        LateDimuonSel            = cms.string(""), ## The dimuon must pass this selection before being written out
        addCommonVertex          = cms.bool(True), ## Embed the full reco::Vertex out of the common vertex fit
        addMuonlessPrimaryVertex = cms.bool(False), ## Embed the primary vertex re-made from all the tracks except the two muons
        resolvePileUpAmbiguity   = cms.bool(True), ## Order PVs by their vicinity to the J/psi vertex, not by sumPt
        onlySoftMuons            = cms.bool(False), ## Keep only the isSoftMuons (without highPurity) for the single muons + the di(tri)muon combinations
        flipJpsiDirection        = cms.int32(False), ## flip the Jpsi direction, before combining it with a third muon
        dimuonMassHypothesis     = cms.double(3.09609) ## dimuon mass hypothesis for KinematicConstrainedVertexFitter and lifetime estimates
)
