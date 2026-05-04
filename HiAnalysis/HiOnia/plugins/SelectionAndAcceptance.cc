#include "HiAnalysis/HiOnia/interface/HiOniaAnalyzer.h"

bool HiOniaAnalyzer::isInAcceptance(const float eta, const float pt, std::string muonType) {

  const float absEta = std::abs(eta);

  if (absEta > 2.4)
    return false;

  if (muonType == (std::string)("TIGHT")) {
    return pt > 6.0;
  }

  if (muonType == (std::string)("GLB")) {
    return ((absEta < 1.2 && pt >= 3.5) || (1.2 <= absEta && absEta < 2.1 &&
                                           pt >= 5.47 - 1.89 * absEta) ||
                                          (2.1 <= absEta && pt >= 1.5));
  } else if (muonType == (std::string)("TRK") || muonType == (std::string)("TRKSOFT")) {
    return ((absEta < 1.1 && pt >= 3.3) ||
             (1.1 <= absEta && absEta < 1.3 && pt >= 13.2 - 9.0 * absEta) ||
             (1.3 <= absEta && pt >= 0.8 && pt >= 3.02 - 1.17 * absEta));
  } else if (muonType == (std::string)("GLBSOFT")) {
    return ((absEta < 0.3 && pt >= 3.4) ||
         (absEta > 0.3 && absEta < 1.1 && pt >= 3.3) ||
         (absEta > 1.1 && absEta < 1.4 && pt >= 7.7 - 4.0 * absEta) ||
         (absEta > 1.4 && absEta < 1.55 && pt >= 2.1) ||
         (absEta > 1.55 && absEta < 2.2 && pt >= 4.25 - 1.39 * absEta) ||
         (absEta > 2.2 && pt >= 1.2));
  } else
    std::cout << "ERROR: Incorrect Muon Type" << std::endl;

  return false;
};

bool HiOniaAnalyzer::isSoftMuonBase(const pat::Muon* aMuon) {
  return (aMuon->isTrackerMuon() && aMuon->innerTrack()->hitPattern().trackerLayersWithMeasurement() > 5 &&
          aMuon->innerTrack()->hitPattern().pixelLayersWithMeasurement() > 0 &&
          abs(aMuon->innerTrack()->dxy(RefVtx)) < 0.3 && abs(aMuon->innerTrack()->dz(RefVtx)) < 20.);
};

bool HiOniaAnalyzer::isHybridSoftMuon(const pat::Muon* aMuon) {
  return (isSoftMuonBase(aMuon) && aMuon->isGlobalMuon());
};

bool HiOniaAnalyzer::selGlobalMuon(const pat::Muon* aMuon) {
  if (!aMuon->isGlobalMuon())
    return false;

  if (_muonSel == (std::string)("GlbTrk") && !aMuon->isTrackerMuon())
    return false;

  if (!_applycuts)
    return true;

  bool isInAcc = isInAcceptance(aMuon->eta(), aMuon->pt(), (std::string)("GLB"));
  bool isGood = (_selTightGlobalMuon ? aMuon->passed(reco::Muon::CutBasedIdTight) : isSoftMuonBase(aMuon));

  return (isInAcc && isGood);
};

bool HiOniaAnalyzer::selTrackerMuon(const pat::Muon* aMuon) {
  if (!aMuon->isTrackerMuon())
    return false;

  if (!_applycuts)
    return true;

  bool isInAcc = isInAcceptance(aMuon->eta(), aMuon->pt(), (std::string)("TRK"));
  bool isGood = isSoftMuonBase(aMuon);

  return (isInAcc && isGood);
};

bool HiOniaAnalyzer::selGlobalOrTrackerMuon(const pat::Muon* aMuon) {
  if (!aMuon->isGlobalMuon() && !aMuon->isTrackerMuon())
    return false;

  if (!_applycuts)
    return true;

  bool isInAcc = isInAcceptance(aMuon->eta(), aMuon->pt(), (std::string)("TRK"));
  bool isGood = isSoftMuonBase(aMuon);

  return (isInAcc && isGood);
};

bool HiOniaAnalyzer::selTrk(const reco::TrackRef aTrk) {
  if (!(aTrk->qualityByName("highPurity") && aTrk->ptError() / aTrk->pt() < 0.1))
    return false;

  if (!_applycuts)
    return true;

  bool isInAcc =
      aTrk->pt() > 1.2 &&
      abs(aTrk->eta()) <
          2.4;  //(aTrk->pt())>0.2 && fabs(aTrk->eta())<2.4 && aTrk->ptError()/aTrk->pt()<0.1 && fabs(aTrk->dxy(RefVtx))<0.35 && fabs(aTrk->dz(RefVtx))<20; //keep margin in dxy and dz, if the RefVtx is not the good one due to muonlessPV

  return (isInAcc);
};

bool HiOniaAnalyzer::selTightMuon(const pat::Muon* aMuon) {
  if (!aMuon->isGlobalMuon())
    return false;

  if (!_applycuts)
    return true;

  bool isInAcc = isInAcceptance(aMuon->eta(), aMuon->pt(), (std::string)("TIGHT"));
  bool isGood = aMuon->passed(reco::Muon::CutBasedIdTight);

  return (isInAcc && isGood);
};

bool HiOniaAnalyzer::isAbHadron(int pdgID) {
  return (abs(pdgID) == 511 || abs(pdgID) == 521 || abs(pdgID) == 531 || abs(pdgID) == 5122 || abs(pdgID) == 541);
};

bool HiOniaAnalyzer::isNeutrino(int pdgID) { return (abs(pdgID) == 14 || abs(pdgID) == 16 || abs(pdgID) == 18); };

bool HiOniaAnalyzer::isChargedTrack(int pdgId) {
  return ((abs(pdgId) == 211) || (abs(pdgId) == 321) || (abs(pdgId) == 2212) || (abs(pdgId) == 11) ||
          (abs(pdgId) == 13));
};

bool HiOniaAnalyzer::isAMixedbHadron(int pdgID, int momPdgID) {
  if ((abs(pdgID) == 511 && abs(momPdgID) == 511 && pdgID * momPdgID < 0) ||
      (abs(pdgID) == 531 && abs(momPdgID) == 531 && pdgID * momPdgID < 0))
    return true;
  return false;
};
