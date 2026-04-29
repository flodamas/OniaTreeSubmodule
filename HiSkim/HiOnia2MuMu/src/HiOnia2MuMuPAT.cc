#include "HiSkim/HiOnia2MuMu/interface/HiOnia2MuMuPAT.h"

//Headers for the data items
#include <DataFormats/TrackReco/interface/TrackFwd.h>
#include <DataFormats/TrackReco/interface/Track.h>
#include <DataFormats/MuonReco/interface/MuonFwd.h>
#include <DataFormats/MuonReco/interface/Muon.h>
#include <DataFormats/Common/interface/View.h>
#include <DataFormats/HepMCCandidate/interface/GenParticle.h>
#include <DataFormats/PatCandidates/interface/Muon.h>
#include <DataFormats/VertexReco/interface/VertexFwd.h>

//Headers for services and tools
#include "RecoVertex/KalmanVertexFit/interface/KalmanVertexFitter.h"
#include "RecoVertex/PrimaryVertexProducer/interface/PrimaryVertexProducer.h"
#include "RecoVertex/VertexPrimitives/interface/TransientVertex.h"
#include "RecoVertex/KinematicFitPrimitives/interface/MultiTrackKinematicConstraint.h"
#include "RecoVertex/KinematicFit/interface/KinematicConstrainedVertexFitter.h"
#include "RecoVertex/VertexTools/interface/VertexDistanceXY.h"
#include "RecoVertex/VertexTools/interface/VertexDistance3D.h"
#include "RecoVertex/KinematicFit/interface/TwoTrackMassKinematicConstraint.h"
#include "RecoVertex/KinematicFitPrimitives/interface/KinematicParticleFactoryFromTransientTrack.h"
//#include "TMath.h"
#include "Math/VectorUtil.h"
#include "Math/DistFunc.h"

#include "TrackingTools/PatternTools/interface/TwoTrackMinimumDistance.h"
#include "TrackingTools/IPTools/interface/IPTools.h"
#include "TrackingTools/PatternTools/interface/ClosestApproachInRPhi.h"

HiOnia2MuMuPAT::HiOnia2MuMuPAT(const edm::ParameterSet &iConfig)
    : muonsToken_(consumes<edm::View<pat::Muon> >(iConfig.getParameter<edm::InputTag>("muons"))),
      thebeamspotToken_(consumes<reco::BeamSpot>(iConfig.getParameter<edm::InputTag>("beamSpotTag"))),
      thePVsToken_(consumes<reco::VertexCollection>(iConfig.getParameter<edm::InputTag>("primaryVertexTag"))),
      recoTracksToken_(consumes<reco::TrackCollection>(iConfig.getParameter<edm::InputTag>("srcTracks"))),
      theGenParticlesToken_(consumes<reco::GenParticleCollection>(iConfig.getParameter<edm::InputTag>("genParticles"))),
      magFieldToken_(esConsumes<MagneticField, IdealMagneticFieldRecord>()),
      trackBuilderToken_(esConsumes(edm::ESInputTag("", "TransientTrackBuilder"))),
      higherPuritySelection_(iConfig.getParameter<std::string>("higherPuritySelection")),
      lowerPuritySelection_(iConfig.getParameter<std::string>("lowerPuritySelection")),
      dimuonSelection_(
          iConfig.existsAs<std::string>("dimuonSelection") ? iConfig.getParameter<std::string>("dimuonSelection") : ""),

      LateDimuonSel_(iConfig.existsAs<std::string>("LateDimuonSel") ? iConfig.getParameter<std::string>("LateDimuonSel")
                                                                    : ""),
      addCommonVertex_(iConfig.getParameter<bool>("addCommonVertex")),
      addMuonlessPrimaryVertex_(iConfig.getParameter<bool>("addMuonlessPrimaryVertex")),
      resolveAmbiguity_(iConfig.getParameter<bool>("resolvePileUpAmbiguity")),
      onlySoftMuons_(iConfig.getParameter<bool>("onlySoftMuons")),
      flipJpsiDirection_(iConfig.getParameter<int>("flipJpsiDirection")),
      Converter_(converter::TrackToCandidate(iConfig, consumesCollector())),
      trackType_(iConfig.getParameter<int>("particleType")),
      trackMass_(iConfig.getParameter<double>("trackMassHypothesis")),
      dimuonMass_(iConfig.getParameter<double>("dimuonMassHypothesis")) {
  produces<pat::CompositeCandidateCollection>("");
};

//
// member functions
//

bool HiOnia2MuMuPAT::isSoftMuonBase(const pat::Muon *aMuon) {
  return (aMuon->isTrackerMuon() && aMuon->innerTrack()->hitPattern().trackerLayersWithMeasurement() > 5 &&
          aMuon->innerTrack()->hitPattern().pixelLayersWithMeasurement() > 0 &&
          std::abs(aMuon->innerTrack()->dxy(RefVtx)) < 0.3 && std::abs(aMuon->innerTrack()->dz(RefVtx)) < 20.);
}

//1: $z -> -z$ and $\phi -> \phi+\pi$ (mirror)
//2: $z -> -z$ and $\phi -> \phi+\pi/2$
//3: $z -> -z$
//4: $z -> -z$ and $\phi -> \phi-\pi/2$
//5: $\phi -> \phi+\pi/2$
//6: $\phi -> \phi+\pi$
//7: $\phi -> \phi-\pi/2$
const reco::TrackBase::Point HiOnia2MuMuPAT::rotatePoint(reco::TrackBase::Point PV,
                                                         reco::TrackBase::Point TrkPoint,
                                                         int flipJpsi) {
  float x = TrkPoint.x(), y = TrkPoint.y(), z = TrkPoint.z();
  float vx = PV.x(), vy = PV.y(), vz = PV.z();

  if (flipJpsi <= 4) {
    z = 2 * vz - z;
  }

  switch (flipJpsi) {
    case 1:
    case 6:
      x = 2 * vx -
          TrkPoint
              .x();  //change frame of reference to place the origin at the PV, rotate, then come back to (0,0,0) origin
      y = 2 * vy - TrkPoint.y();
      break;
    case 2:
    case 5:
      x = vx - (TrkPoint.y() - vy);
      y = vy + TrkPoint.x() - vx;
      break;
    case 4:
    case 7:
      x = vx + TrkPoint.y() - vy;
      y = vy - (TrkPoint.x() - vx);
  }
  return reco::TrackBase::Point(x, y, z);
};

const reco::TrackBase::Vector HiOnia2MuMuPAT::rotateMomentum(reco::Track trk, int flipJpsi) {
  float px = trk.px(), py = trk.py(), pz = trk.pz();

  if (flipJpsi <= 4) {
    pz = -trk.pz();
  }

  switch (flipJpsi) {
    case 1:
    case 6:
      px = -trk.px();
      py = -trk.py();
      break;
    case 2:
    case 5:
      px = -trk.py();
      py = trk.px();
      break;
    case 4:
    case 7:
      px = trk.py();
      py = -trk.px();
  }

  return reco::TrackBase::Vector(px, py, pz);
};

// ------------ method called to produce the data  ------------
void HiOnia2MuMuPAT::produce(edm::Event &iEvent, const edm::EventSetup &iSetup) {
  using namespace edm;
  using namespace std;
  using namespace reco;
  typedef Candidate::LorentzVector LorentzVector;


  vector<double> muMasses;
  muMasses.push_back(0.1056583715);
  muMasses.push_back(0.1056583715);

  std::unique_ptr<pat::CompositeCandidateCollection> oniaOutput(new pat::CompositeCandidateCollection);

  Vertex thePrimaryV;
  Vertex theBeamSpotV;

  const auto &bField = iSetup.getData(magFieldToken_);

  // get the stored reco BS, and copy its position in a Vertex object (theBeamSpotV)
  Handle<BeamSpot> theBeamSpot;
  iEvent.getByToken(thebeamspotToken_, theBeamSpot);
  BeamSpot bs = *theBeamSpot;
  theBeamSpotV = Vertex(bs.position(), bs.covariance3D());

  // get PV collection, if they are empty use theBeamSpot
  Handle<VertexCollection> priVtxs;
  iEvent.getByToken(thePVsToken_, priVtxs);
  if (priVtxs->begin() != priVtxs->end()) {
    thePrimaryV = Vertex(*(priVtxs->begin()));
  } else {
    thePrimaryV = Vertex(bs.position(), bs.covariance3D());
  }
  RefVtx = thePrimaryV.position();

  Handle<View<pat::Muon> > muons;
  iEvent.getByToken(muonsToken_, muons);

  const auto &theTTBuilder = iSetup.getHandle(trackBuilderToken_);
  KalmanVertexFitter vtxFitter(true);

  //For kinematic constrained fit
  KinematicParticleFactoryFromTransientTrack pFactory;
  ParticleMass jp_mass = dimuonMass_;
  KinematicConstrainedVertexFitter KCfitter;


  TrackCollection muonLess;  // track collection related to PV, minus the 2 muons (if muonLessPV option is activated)


  std::vector<pat::Muon> ourMuons;
  for (const auto& muon : *muons) {
    if (lowerPuritySelection_(muon) && (!onlySoftMuons_ || isSoftMuonBase(&(muon)))) {
      ourMuons.push_back(muon);
    }
  }
  int ourMuNb = ourMuons.size();
  //std::cout<<"number of soft muons = "<<ourMuNb<<std::endl;

  // Dimuon candidates only from muons
  for (int i = 0; i < ourMuNb; i++) {
    const pat::Muon &it = ourMuons[i];
    for (int j = i + 1; j < ourMuNb; j++) {
      bool goodMu1Mu2 = false;
      const pat::Muon &it2 = ourMuons[j];
      // one muon must pass tight quality
      if (!(higherPuritySelection_(it) || higherPuritySelection_(it2)))
        continue;
      if (!(it.track().isNonnull()) || !(it2.track().isNonnull()))
        continue;

      // --- some declarations ---
      std::map<std::string, int> userInt;
      std::map<std::string, float> userFloat;
      std::map<std::string, reco::Vertex> userVertex;
      std::map<std::string, reco::Track> userTrack;
      Vertex theOriginalPV;
      int flipJpsi = 0;  //loop iterator in case of flipJpsiDirection_>0
      TransientVertex myVertex;
      CachingVertex<5> VtxForInvMass;
      Measurement1D MassWErr;
      vector<TransientTrack> t_tks;
      float vChi2 = -100, vNDF = 1;

      pat::CompositeCandidate myCand;     // Default
      pat::CompositeCandidate myCandTmp;  // Default
      // ---- no explicit order defined ----
      myCand.addDaughter(it, "muon1");
      myCand.addDaughter(it2, "muon2");

      reco::Track muon1Trk = (*it.track());
      reco::Track muon2Trk = (*it2.track());
      LorentzVector mu1 = it.p4();
      LorentzVector mu2 = it2.p4();

      // ---- define and set candidate's 4momentum  ----
      LorentzVector jpsi = mu1 + mu2;
      myCand.setP4(jpsi);
      myCand.setCharge(it.charge() + it2.charge());


      // ---- build the dimuon secondary vertex ----
      t_tks.push_back(
          theTTBuilder->build(muon1Trk));  // pass the reco::Track, not  the reco::TrackRef (which can be transient)
      t_tks.push_back(theTTBuilder->build(muon2Trk));  // otherwise the vertex will have transient refs inside.

      VtxForInvMass = vtxFitter.vertex(t_tks);
      
      myVertex = vtxFitter.vertex(t_tks);

      MassWErr = Measurement1D(jpsi.M(), -9999.);
      if (bField.nominalValue() > 0) {
	      MassWErr = massCalculator.invariantMass(VtxForInvMass, muMasses);
      } else {
	      myVertex = TransientVertex();  // with no arguments it is invalid
      }

      userFloat["MassErr"] = MassWErr.error();

      if (myVertex.isValid()) {
        if (resolveAmbiguity_) {
          float minDz = 999999.;

          TwoTrackMinimumDistance ttmd;
          bool status = ttmd.calculate(
              GlobalTrajectoryParameters(
                  GlobalPoint(myVertex.position().x(), myVertex.position().y(), myVertex.position().z()),
                  GlobalVector(myCand.px(), myCand.py(), myCand.pz()),
                  TrackCharge(0),
                  &(bField)),
              GlobalTrajectoryParameters(GlobalPoint(bs.position().x(), bs.position().y(), bs.position().z()),
                                         GlobalVector(bs.dxdz(), bs.dydz(), 1.),
                                         TrackCharge(0),
                                         &(bField)));
          float extrapZ = -9E20;
          if (status)
            extrapZ = ttmd.points().first.z();

          for (VertexCollection::const_iterator itv = priVtxs->begin(), itvend = priVtxs->end(); itv != itvend; ++itv) {
            // only consider good vertices
            if (itv->isFake() || itv->tracksSize() < 2 || std::abs(itv->position().z()) > 25 || itv->position().Rho() > 2)
              continue;
            float deltaZ = std::abs(extrapZ - itv->position().z());
            if (deltaZ < minDz) {
              minDz = deltaZ;
              thePrimaryV = Vertex(*itv);
            }
          }
        }  //if resolve ambiguity

        theOriginalPV = thePrimaryV;

        // ---- apply the dimuon cut --- This selection is done only here because "resolvePileUpAmbiguity" info is needed for later Trimuon
        if (!dimuonSelection_(myCand)) {
          continue;
        }

        muonLess.clear();
        muonLess.reserve(thePrimaryV.tracksSize());
        if (addMuonlessPrimaryVertex_ && thePrimaryV.tracksSize() > 2) {
          // ---- Primary vertex matched to the dimuon, now refit it removing the two muons ----
          //edm::LogWarning("HiOnia2MuMuPAT_addMuonlessPrimaryVertex") << "If muonLessPV is turned on, ctau is calculated with muonLessPV only.\n" ;

          // I need to go back to the reco::Muon object, as the TrackRef in the pat::Muon can be an embedded ref.
          const reco::Muon *rmu1 = dynamic_cast<const reco::Muon *>(it.originalObject());
          const reco::Muon *rmu2 = dynamic_cast<const reco::Muon *>(it2.originalObject());
          if (thePrimaryV.hasRefittedTracks()) {
            // Need to go back to the original tracks before taking the key
            for (const auto &itRefittedTrack : thePrimaryV.refittedTracks()) {
              if (thePrimaryV.originalTrack(itRefittedTrack).key() == rmu1->track().key())
                continue;
              if (thePrimaryV.originalTrack(itRefittedTrack).key() == rmu2->track().key())
                continue;
              const reco::Track &recoTrack = *(thePrimaryV.originalTrack(itRefittedTrack));
              muonLess.push_back(recoTrack);
            }
          }  // PV has refitted tracks
          else {
            std::vector<reco::TrackBaseRef>::const_iterator itPVtrack = thePrimaryV.tracks_begin();
            for (; itPVtrack != thePrimaryV.tracks_end(); ++itPVtrack)
              if (itPVtrack->isNonnull()) {
                if (itPVtrack->key() == rmu1->track().key())
                  continue;
                if (itPVtrack->key() == rmu2->track().key())
                  continue;
                muonLess.push_back(**itPVtrack);
              }
          }  // take all tracks associated with the vtx

          if (muonLess.size() > 1 && muonLess.size() < thePrimaryV.tracksSize()) {
            // find the new vertex, from which the 2 muons were removed
            // need the transient tracks corresponding to the new track collection
            std::vector<reco::TransientTrack> t_tks_muonless;
            t_tks_muonless.reserve(muonLess.size());

            for (const auto &it : muonLess) {
              t_tks_muonless.push_back((*theTTBuilder).build(it));
              t_tks_muonless.back().setBeamSpot(bs);
            }
            std::unique_ptr<AdaptiveVertexFitter> theFitter(new AdaptiveVertexFitter());
            TransientVertex pvs = theFitter->vertex(t_tks_muonless, bs);  // if you want the beam constraint

            if (pvs.isValid()) {
              reco::Vertex muonLessPV = Vertex(pvs);
              thePrimaryV = muonLessPV;
            } else {
              edm::LogWarning("HiOnia2MuMuPAT_FailingToRefitMuonLessVtx")
                  << "TransientVertex re-fitted is not valid!! You got still the 'old vertex'"
                  << "\n";
            }
          } else {
            if (muonLess.size() == thePrimaryV.tracksSize()) {
              //edm::LogWarning("HiOnia2MuMuPAT_muonLessSizeORpvTrkSize") <<
              //"Still have the original PV: the refit was not done 'cose it is already muonless" << "\n";
            } else if (muonLess.size() <= 1) {
              edm::LogWarning("HiOnia2MuMuPAT_muonLessSizeORpvTrkSize")
                  << "Still have the original PV: the refit was not done 'cose there are not enough tracks to do the "
                     "refit without the muon tracks"
                  << "\n";
            } else {
              edm::LogWarning("HiOnia2MuMuPAT_muonLessSizeORpvTrkSize")
                  << "Still have the original PV: Something weird just happened, muonLess.size()=" << muonLess.size()
                  << " and thePrimaryV.tracksSize()=" << thePrimaryV.tracksSize() << " ."
                  << "\n";
            }
          }
        }  // ---- end refit vtx without the muon tracks ----

        // ---- count the number of high Purity tracks with pT > 500 MeV attached to the chosen vertex ----
        // this makes sense only in case of pp reconstruction
          double vertexWeight = -1., sumPTPV = -1.;
          int countTksOfPV = -1;

          EDConsumerBase::Labels thePVsLabel;
          EDConsumerBase::labelsForToken(thePVsToken_, thePVsLabel);
          if (thePVsLabel.module == (std::string)("offlinePrimaryVertices")) {
            const reco::Muon *rmu1 = dynamic_cast<const reco::Muon *>(it.originalObject());
            const reco::Muon *rmu2 = dynamic_cast<const reco::Muon *>(it2.originalObject());
            try {
              for (reco::Vertex::trackRef_iterator itVtx = theOriginalPV.tracks_begin();
                   itVtx != theOriginalPV.tracks_end();
                   itVtx++)
                if (itVtx->isNonnull()) {
                  const reco::Track &track = **itVtx;
                  if (!track.quality(reco::TrackBase::highPurity))
                    continue;
                  if (track.pt() < 0.5)
                    continue;  //reject all rejects from counting if less than 500 MeV

                  TransientTrack tt = theTTBuilder->build(track);
                  pair<bool, Measurement1D> tkPVdist = IPTools::absoluteImpactParameter3D(tt, theOriginalPV);

                  if (!tkPVdist.first)
                    continue;
                  if (tkPVdist.second.significance() > 3)
                    continue;
                  if (track.ptError() / track.pt() > 0.1)
                    continue;

                  // do not count the two muons
                  if (rmu1 != nullptr && rmu1->innerTrack().key() == itVtx->key())
                    continue;
                  if (rmu2 != nullptr && rmu2->innerTrack().key() == itVtx->key())
                    continue;

                  vertexWeight += theOriginalPV.trackWeight(*itVtx);
                  if (theOriginalPV.trackWeight(*itVtx) > 0.5) {
                    countTksOfPV++;
                    sumPTPV += track.pt();
                  }
                }
            } catch (std::exception &err) {
              std::cout << " Counting tracks from PV, fails! " << std::endl;
              return;
            }
          }
          userInt["countTksOfPV"] = countTksOfPV;
          userFloat["vertexWeight"] = (float)vertexWeight;
          userFloat["sumPTPV"] = (float)sumPTPV;
        
        // ---- end track counting ----
        userFloat["vNChi2"] = myVertex.normalisedChiSquared();
        userFloat["vProb"] = ROOT::Math::chisquared_cdf_c(myVertex.totalChiSquared(), myVertex.degreesOfFreedom());

        VertexDistanceXY vdistXY;
        VertexDistance3D vdistXYZ;

	      math::XYZPoint vtx(myVertex.position().x(), myVertex.position().y(), 0);
        math::XYZPoint pperp(jpsi.px(), jpsi.py(), 0);
        AlgebraicVector3 vpperp(pperp.x(), pperp.y(), 0.);

        math::XYZPoint vtx3D(myVertex.position().x(), myVertex.position().y(), myVertex.position().z());
        math::XYZPoint pxyz(jpsi.px(), jpsi.py(), jpsi.pz());
        AlgebraicVector3 vpxyz(pxyz.x(), pxyz.y(), pxyz.z());

        ///DCA
        TrajectoryStateClosestToPoint mu1TS = t_tks[0].impactPointTSCP();
        TrajectoryStateClosestToPoint mu2TS = t_tks[1].impactPointTSCP();
        float dca = 1E20;
        if (mu1TS.isValid() && mu2TS.isValid()) {
          ClosestApproachInRPhi cApp;
          cApp.calculate(mu1TS.theState(), mu2TS.theState());
          if (cApp.status())
            dca = cApp.distance();
        }
        userFloat["DCA"] = dca;
        ///end DCA

        if (addMuonlessPrimaryVertex_) {
          userVertex["muonlessPV"] = thePrimaryV;
          userVertex["PVwithmuons"] = theOriginalPV;
        } else {
          userVertex["PVwithmuons"] = thePrimaryV;
        }

        // lifetime using PV
	      math::XYZPoint pvtx(thePrimaryV.position().x(), thePrimaryV.position().y(), 0);
        auto vdiff = vtx - pvtx;
        double cosAlpha = vdiff.Dot(pperp) / (std::sqrt(vdiff.Perp2() * pperp.Perp2()));
        Measurement1D distXY = vdistXY.distance(Vertex(myVertex), thePrimaryV);
        double ctauPV = distXY.value() * cosAlpha * dimuonMass_ / std::sqrt(pperp.Perp2());
        GlobalError v1e = (Vertex(myVertex)).error();
        GlobalError v2e = thePrimaryV.error();
        AlgebraicSymMatrix33 vXYe = v1e.matrix() + v2e.matrix();
        double ctauErrPV = sqrt(ROOT::Math::Similarity(vpperp, vXYe)) * dimuonMass_ / (pperp.Perp2());

        userFloat["ppdlPV"] = ctauPV;
        userFloat["ppdlErrPV"] = ctauErrPV;
        userFloat["cosAlpha"] = cosAlpha;

        math::XYZPoint pvtx3D(thePrimaryV.position().x(), thePrimaryV.position().y(), thePrimaryV.position().z());
        auto vdiff3D = vtx3D - pvtx3D;
        double cosAlpha3D = vdiff3D.Dot(pxyz) / (std::sqrt(vdiff3D.Mag2() * pxyz.Mag2()));
        Measurement1D distXYZ = vdistXYZ.distance(Vertex(myVertex), thePrimaryV);
        double ctauPV3D = distXYZ.value() * cosAlpha3D * dimuonMass_ / std::sqrt(pxyz.Mag2());
        double ctauErrPV3D = sqrt(ROOT::Math::Similarity(vpxyz, vXYe)) * dimuonMass_ / (pxyz.Mag2());

        userFloat["ppdlPV3D"] = ctauPV3D;
        userFloat["ppdlErrPV3D"] = ctauErrPV3D;
        userFloat["cosAlpha3D"] = cosAlpha3D;

        if (addMuonlessPrimaryVertex_ ) {
          // 2D-lifetime using Original PV
          pvtx.SetXYZ(theOriginalPV.position().x(), theOriginalPV.position().y(), 0);
          vdiff = vtx - pvtx;
          double cosAlphaOrigPV = vdiff.Dot(pperp) / (std::sqrt(vdiff.Perp2() * pperp.Perp2()));
          distXY = vdistXY.distance(Vertex(myVertex), theOriginalPV);
          double ctauOrigPV = distXY.value() * cosAlphaOrigPV * dimuonMass_ / std::sqrt(pperp.Perp2());
          GlobalError v1eOrigPV = (Vertex(myVertex)).error();
          GlobalError v2eOrigPV = theOriginalPV.error();
          AlgebraicSymMatrix33 vXYeOrigPV = v1eOrigPV.matrix() + v2eOrigPV.matrix();
          double ctauErrOrigPV = sqrt(ROOT::Math::Similarity(vpperp, vXYeOrigPV)) * dimuonMass_ / (pperp.Perp2());

          userFloat["ppdlOrigPV"] = ctauOrigPV;
          userFloat["ppdlErrOrigPV"] = ctauErrOrigPV;

          // 3D-lifetime using Original PV
          pvtx3D.SetXYZ(theOriginalPV.position().x(), theOriginalPV.position().y(), theOriginalPV.position().z());
          vdiff3D = vtx3D - pvtx3D;
          double cosAlphaOrigPV3D = vdiff3D.Dot(pxyz) / (std::sqrt(vdiff3D.Mag2() * pxyz.Mag2()));
          distXYZ = vdistXYZ.distance(Vertex(myVertex), theOriginalPV);
          double ctauOrigPV3D = distXYZ.value() * cosAlphaOrigPV3D * dimuonMass_ / std::sqrt(pxyz.Mag2());
          double ctauErrOrigPV3D = sqrt(ROOT::Math::Similarity(vpxyz, vXYeOrigPV)) * dimuonMass_ / (pxyz.Mag2());

          userFloat["ppdlOrigPV3D"] = ctauOrigPV3D;
          userFloat["ppdlErrOrigPV3D"] = ctauErrOrigPV3D;
        } else {
          userFloat["ppdlOrigPV"] = ctauPV;
          userFloat["ppdlErrOrigPV"] = ctauErrPV;
          userFloat["ppdlOrigPV3D"] = ctauPV3D;
          userFloat["ppdlErrOrigPV3D"] = ctauErrPV3D;
        }

        // lifetime using BS
          pvtx.SetXYZ(theBeamSpotV.position().x(), theBeamSpotV.position().y(), 0);
          vdiff = vtx - pvtx;
          cosAlpha = vdiff.Dot(pperp) / (std::sqrt(vdiff.Perp2() * pperp.Perp2()));
          distXY = vdistXY.distance(Vertex(myVertex), theBeamSpotV);
          double ctauBS = distXY.value() * cosAlpha * dimuonMass_ / std::sqrt(pperp.Perp2());
          GlobalError v1eB = (Vertex(myVertex)).error();
          GlobalError v2eB = theBeamSpotV.error();
          AlgebraicSymMatrix33 vXYeB = v1eB.matrix() + v2eB.matrix();
          double ctauErrBS = sqrt(ROOT::Math::Similarity(vpperp, vXYeB)) * dimuonMass_ / (pperp.Perp2());

          userFloat["ppdlBS"] = ctauBS;
          userFloat["ppdlErrBS"] = ctauErrBS;
          pvtx3D.SetXYZ(theBeamSpotV.position().x(), theBeamSpotV.position().y(), theBeamSpotV.position().z());
          vdiff3D = vtx3D - pvtx3D;
          cosAlpha3D = vdiff3D.Dot(pxyz) / (std::sqrt(vdiff3D.Mag2() * pxyz.Mag2()));
          distXYZ = vdistXYZ.distance(Vertex(myVertex), theBeamSpotV);
          double ctauBS3D = distXYZ.value() * cosAlpha3D * dimuonMass_ / std::sqrt(pxyz.Mag2());
          double ctauErrBS3D = sqrt(ROOT::Math::Similarity(vpxyz, vXYeB)) * dimuonMass_ / (pxyz.Mag2());

          userFloat["ppdlBS3D"] = ctauBS3D;
          userFloat["ppdlErrBS3D"] = ctauErrBS3D;
        

        if (addCommonVertex_) {
          userVertex["commonVertex"] = Vertex(myVertex);
        }
      } else {
        userFloat["vNChi2"] = -1;
        userFloat["vProb"] = -1;
        userFloat["vertexWeight"] = -100;
        userFloat["sumPTPV"] = -100;
        userFloat["DCA"] = -10;
        userFloat["ppdlPV"] = -100;
        userFloat["ppdlErrPV"] = -100;
        userFloat["cosAlpha"] = -10;
        userFloat["ppdlBS"] = -100;
        userFloat["ppdlErrBS"] = -100;
        userFloat["ppdlOrigPV"] = -100;
        userFloat["ppdlErrOrigPV"] = -100;
        userFloat["ppdlPV3D"] = -100;
        userFloat["ppdlErrPV3D"] = -100;
        userFloat["cosAlpha3D"] = -10;
        userFloat["ppdlBS3D"] = -100;
        userFloat["ppdlErrBS3D"] = -100;
        userFloat["ppdlOrigPV3D"] = -100;
        userFloat["ppdlErrOrigPV3D"] = -100;

        userInt["countTksOfPV"] = -1;

        if (addCommonVertex_) {
          userVertex["commonVertex"] = Vertex();
        }
        if (addMuonlessPrimaryVertex_) {
          userVertex["muonlessPV"] = Vertex();
          userVertex["PVwithmuons"] = Vertex();
        } else {
          userVertex["PVwithmuons"] = Vertex();
        }
      }


      for (const auto &i : userFloat) {
        myCand.addUserFloat(i.first, i.second);
      }

      if (!LateDimuonSel_(myCand)) {
        continue;
      }
      goodMu1Mu2 = true;
      for (; flipJpsi < (1 + flipJpsiDirection_);
           flipJpsi++) {  //'int flipJpsi=0' must be declared before the 'goto' statements
        if (flipJpsiDirection_ > 0 && flipJpsi == 0)
          continue;

        myCandTmp = myCand;
        // --- Build the flipped tracks, change the vertex accordingly ---
        if (flipJpsiDirection_ > 0) {
          const reco::TrackBase::Point &refPoint1 = rotatePoint(
              thePrimaryV.position(), (it.track())->referencePoint(), flipJpsi);  //catch tracks of original muons
          const reco::TrackBase::Vector &Momentum1 = rotateMomentum(*it.track(), flipJpsi);
          muon1Trk = reco::Track(muon1Trk.chi2(),
                                 muon1Trk.ndof(),
                                 refPoint1,
                                 Momentum1,
                                 it.charge(),
                                 muon1Trk.covariance(),
                                 muon1Trk.originalAlgo());  //forget TrackQuality info here
          mu1 = LorentzVector(
              muon1Trk.px(), muon1Trk.py(), muon1Trk.pz(), sqrt(pow(muon1Trk.p(), 2) + pow(muMasses[0], 2)));

          const reco::TrackBase::Point &refPoint2 =
              rotatePoint(thePrimaryV.position(), (it2.track())->referencePoint(), flipJpsi);
          const reco::TrackBase::Vector &Momentum2 = rotateMomentum(*it2.track(), flipJpsi);
          muon2Trk = reco::Track(muon2Trk.chi2(),
                                 muon2Trk.ndof(),
                                 refPoint2,
                                 Momentum2,
                                 it2.charge(),
                                 muon2Trk.covariance(),
                                 muon2Trk.originalAlgo());
          mu2 = LorentzVector(
              muon2Trk.px(), muon2Trk.py(), muon2Trk.pz(), sqrt(pow(muon2Trk.p(), 2) + pow(muMasses[1], 2)));
          // cout<<"PV x, y,, z = "<<thePrimaryV.position().x()<<" "<<thePrimaryV.position().y()<<" "<<thePrimaryV.position().z()<<" "<<endl;
          // cout<<"old track x, y, z, px, py, pz = "<<(*it.track()).referencePoint().x()<<" "<<(*it.track()).referencePoint().y()<<" "<<(*it.track()).referencePoint().z()<<" "<<(*it.track()).px()<<" "<<(*it.track()).py()<<" "<<(*it.track()).pz()<<endl;
          // cout<<"new track x, y, z, px, py, pz = "<<muon1Trk.referencePoint().x()<<" "<<muon1Trk.referencePoint().y()<<" "<<muon1Trk.referencePoint().z()<<" "<<muon1Trk.px()<<" "<<muon1Trk.py()<<" "<<muon1Trk.pz()<<endl;

          jpsi = mu1 + mu2;
          myCandTmp.setP4(jpsi);
        }

        if ((flipJpsiDirection_ == 0) && goodMu1Mu2) {
          if (flipJpsiDirection_ > 0) {
            userTrack["muon1Track"] = muon1Trk;
            userTrack["muon2Track"] = muon2Trk;
            if (myVertex.isValid() && addCommonVertex_) {
              userVertex["commonVertex"] =
                  Vertex(reco::Vertex::Point(2 * thePrimaryV.position().x() - myVertex.position().x(),
                                             2 * thePrimaryV.position().y() - myVertex.position().y(),
                                             2 * thePrimaryV.position().z() - myVertex.position().z()),
                         userVertex["commonVertex"].error(),
                         vChi2,
                         vNDF,
                         2);
            }
            userInt["flipJpsi"] = flipJpsi;
            for (const auto &i : userTrack) {
              myCandTmp.addUserData(i.first, i.second);
            }
          }
          for (const auto &i : userInt) {
            myCandTmp.addUserInt(i.first, i.second);
          }
          for (const auto &i : userVertex) {
            myCandTmp.addUserData(i.first, i.second);
          }
          // ---- Push back output of this Jpsi candidate ----
          oniaOutput->push_back(myCandTmp);
        }

      }  //flipJpsi (always 0 when flipJpsiDirection_==0, i.e. the loop runs only once)
    }    //it2 muon
  }      //it muon

  //  std::sort(oniaOutput->begin(),oniaOutput->end(),pTComparator_);
  std::sort(oniaOutput->begin(), oniaOutput->end(), vPComparator_);
  iEvent.put(std::move(oniaOutput), "");

};

//define this as a plug-in
DEFINE_FWK_MODULE(HiOnia2MuMuPAT);
