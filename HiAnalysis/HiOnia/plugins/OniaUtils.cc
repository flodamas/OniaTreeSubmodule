#include "HiAnalysis/HiOnia/interface/HiOniaAnalyzer.h"


//Find the indices of the reconstructed muon matching each generated muon, and vice versa
void HiOniaAnalyzer::fillMuMatchingInfo() {
  //initialize Gen_mu_whichRec
  for (int igen = 0; igen < Gen_mu_size; igen++) {
    Gen_mu_whichRec[igen] = -1;
  }

  //Find the index of generated muon associated to a reco muon, txs to Reco_mu_pTrue
  for (int irec = 0; irec < Reco_mu_size; irec++) {
    int foundGen = -1;
    if (Reco_mu_pTrue[irec] >= 0) {  //if pTrue=-1, then the reco muon is a fake
      for (int igen = 0; igen < Gen_mu_size; igen++) {
	
        auto genmuMom = Gen_mu_4mom.at(igen).P();
	
        if (std::abs(genmuMom - Reco_mu_pTrue[irec]) / Reco_mu_pTrue[irec] < 1e-6 &&
            Gen_mu_charge[igen] == Reco_mu_charge[irec]) {
          foundGen = igen;
          break;
        }
      }
    }
    Reco_mu_whichGen[irec] = foundGen;
    if (foundGen > -1)
      Gen_mu_whichRec[foundGen] = irec;
  }
};

pair<unsigned int, const pat::CompositeCandidate*> HiOniaAnalyzer::theBestQQ() {
  unsigned int theBestCat = 99;
  const pat::CompositeCandidate* theBestCand = new pat::CompositeCandidate();

  for (unsigned int i = 0; i < _thePassedCands.size(); i++) {
    if (_thePassedCats.at(i) < theBestCat) {
      theBestCat = _thePassedCats.at(i);
      theBestCand = _thePassedCands.at(i);
    }
  }
  pair<unsigned int, const pat::CompositeCandidate*> result = make_pair(theBestCat, theBestCand);
  return result;
};

void HiOniaAnalyzer::makeCuts(bool keepSameSign) {
  math::XYZPoint RefVtx_tmp = RefVtx;

  if (collJpsi.isValid()) {
    for (std::vector<pat::CompositeCandidate>::const_iterator it = collJpsi->begin(); it != collJpsi->end(); ++it) {
      const pat::CompositeCandidate* cand = &(*it);

      if (cand == nullptr) {
        std::cout << "ERROR: 'cand' pointer in makeCuts is NULL ! Return now" << std::endl;
        return;
      } else {
        const pat::Muon* muon1 = dynamic_cast<const pat::Muon*>(cand->daughter("muon1"));
        const pat::Muon* muon2 = dynamic_cast<const pat::Muon*>(cand->daughter("muon2"));

        if (muon1 == nullptr || muon2 == nullptr) {
          std::cout << "ERROR: 'muon1' or 'muon2' pointer in makeCuts is NULL ! Return now" << std::endl;
          return;
        } else {
          if (!keepSameSign && (muon1->charge() + muon2->charge() != 0))
            continue;

          if (!(_isHI) && _muonLessPrimaryVertex && cand->hasUserData("muonlessPV"))
            RefVtx = (*cand->userData<reco::Vertex>("muonlessPV")).position();
          else if (!_muonLessPrimaryVertex && cand->hasUserData("PVwithmuons"))
            RefVtx = (*cand->userData<reco::Vertex>("PVwithmuons")).position();
          else {
            std::cout << "HiOniaAnalyzer::makeCuts: no PV for muon pair stored ! Go to next candidate." << std::endl;
            continue;
          }

          if (std::abs(RefVtx.Z()) > _iConfig.getParameter<double>("maxAbsZ"))
            continue;

          if (std::abs(muon1->eta()) >= etaMax || std::abs(muon2->eta()) >= etaMax)
            continue;

          //Pass muon selection?
          bool muonSelFound = false;
          if (_muonSel == (std::string)("Glb")) {
            if (checkCuts(cand, muon1, muon2, &HiOniaAnalyzer::selGlobalMuon, &HiOniaAnalyzer::selGlobalMuon)) {
              _thePassedCats.push_back(Glb_Glb);
              _thePassedCands.push_back(cand);
              if (!_fillSingleMuons) {
                EtaOfWantedMuons.push_back(muon1->eta());
                EtaOfWantedMuons.push_back(muon2->eta());
              }
            }
            muonSelFound = true;
          }
          if (_muonSel == (std::string)("GlbTrk")) {
            if (checkCuts(cand, muon1, muon2, &HiOniaAnalyzer::selGlobalMuon, &HiOniaAnalyzer::selGlobalMuon)) {
              _thePassedCats.push_back(GlbTrk_GlbTrk);
              _thePassedCands.push_back(cand);
              if (!_fillSingleMuons) {
                EtaOfWantedMuons.push_back(muon1->eta());
                EtaOfWantedMuons.push_back(muon2->eta());
              }
            }
            muonSelFound = true;
          }
          if (_muonSel == (std::string)("Trk")) {
            if (checkCuts(cand, muon1, muon2, &HiOniaAnalyzer::selTrackerMuon, &HiOniaAnalyzer::selTrackerMuon)) {
              _thePassedCats.push_back(Trk_Trk);
              _thePassedCands.push_back(cand);
              if (!_fillSingleMuons) {
                EtaOfWantedMuons.push_back(muon1->eta());
                EtaOfWantedMuons.push_back(muon2->eta());
              }
            }
            muonSelFound = true;
          }
          if (_muonSel == (std::string)("GlbOrTrk")) {
            if (checkCuts(cand,
                          muon1,
                          muon2,
                          &HiOniaAnalyzer::selGlobalOrTrackerMuon,
                          &HiOniaAnalyzer::selGlobalOrTrackerMuon)) {
              _thePassedCats.push_back(GlbOrTrk_GlbOrTrk);
              _thePassedCands.push_back(cand);
              if (!_fillSingleMuons) {
                EtaOfWantedMuons.push_back(muon1->eta());
                EtaOfWantedMuons.push_back(muon2->eta());
              }
            }
            muonSelFound = true;
          }
          if (_muonSel == (std::string)("All")) {
              _thePassedCats.push_back(All_All);
              _thePassedCands.push_back(cand);
              if (!_fillSingleMuons) {
                EtaOfWantedMuons.push_back(muon1->eta());
                EtaOfWantedMuons.push_back(muon2->eta());
              }
            
            muonSelFound = true;
          }
          if (!muonSelFound) {
            std::cout << "[HiOniaAnalyzer::makeCuts] --- The muon selection: " << _muonSel
                      << " is invalid. The supported options are: All, Glb, GlbTrk, GlbOrTrk, Trk"
                      << std::endl;
          }
        }
      }
    }
  }
  RefVtx = RefVtx_tmp;
  return;
};

bool HiOniaAnalyzer::checkCuts(const pat::CompositeCandidate* cand,
                               const pat::Muon* muon1,
                               const pat::Muon* muon2,
                               bool (HiOniaAnalyzer::*callFunc1)(const pat::Muon*),
                               bool (HiOniaAnalyzer::*callFunc2)(const pat::Muon*)) {
  std::string lastFilter = _OneMatchedHLTMu >= 0 ? filterNameMap.at(theTriggerNames[_OneMatchedHLTMu]) : "";
  if ((((this->*callFunc1)(muon1) && (this->*callFunc2)(muon2)) ||
       ((this->*callFunc1)(muon2) && (this->*callFunc2)(muon1))) &&
      (!_applycuts || true) &&  //Add hard-coded cuts here if desired
      ((_OneMatchedHLTMu == -1) || !muon1->triggerObjectMatchesByFilter(lastFilter).empty() ||
       !muon2->triggerObjectMatchesByFilter(lastFilter).empty()))
    return true;
  else
    return false;
};

int HiOniaAnalyzer::IndexOfThisMuon(const float muonPt, bool isGen) {
  const auto& mapMuIdx = (isGen ? mapGenMuonMomToIndex_ : mapMuonMomToIndex_);
  const long int& muPt = FloatToIntkey(muonPt);

  if (mapMuIdx.count(muPt) == 0)
    return -1;
  else
    return mapMuIdx.at(muPt);
};

int HiOniaAnalyzer::IndexOfThisTrack(const float trackPt, bool isGen) {
  const auto& mapTrkIdx = (isGen ? mapTrkMomToIndex_ : mapTrkMomToIndex_);
  const long int& trkPt = FloatToIntkey(trackPt);

  if (mapTrkIdx.count(trkPt) == 0)
    return -1;
  else
    return mapTrkIdx.at(trkPt);
};

int HiOniaAnalyzer::IndexOfThisJpsi(int mu1_idx, int mu2_idx, int flipJpsi) {
  int GoodIndex = -1;
  for (int iJpsi = 0; iJpsi < Reco_QQ_size; iJpsi++) {
    if (((Reco_QQ_mumi_idx[iJpsi] == mu1_idx && Reco_QQ_mupl_idx[iJpsi] == mu2_idx) ||
         (Reco_QQ_mumi_idx[iJpsi] == mu2_idx && Reco_QQ_mupl_idx[iJpsi] == mu1_idx)) &&
        (flipJpsi == Reco_QQ_flipJpsi[iJpsi])) {
      GoodIndex = iJpsi;
      break;
    }
  }
  return GoodIndex;
};

Short_t HiOniaAnalyzer::MuInSV(LorentzVector v1, LorentzVector v2, LorentzVector v3) {
  int nMuInSV = 0;
  for (std::vector<reco::Vertex>::const_iterator vt = SVs->begin(); vt != SVs->end(); ++vt) {
    const reco::Vertex* vtx = &(*vt);
    int nTrksInSV = 0;
    for (reco::Vertex::trackRef_iterator it = vtx->tracks_begin(); it != vtx->tracks_end(); ++it) {
      auto trkPt = (*it)->pt();
      auto trkEta = (*it)->eta();
      if ((std::abs(trkPt - v1.Pt()) < 1e-3 && std::abs(trkEta - v1.Eta()) < 1e-4) ||
          (std::abs(trkPt - v2.Pt()) < 1e-3 && std::abs(trkEta - v2.Eta()) < 1e-4) ||
          (std::abs(trkPt - v3.Pt()) < 1e-3 && std::abs(trkEta - v3.Eta()) < 1e-4)) {
        nTrksInSV += 1;
      }
    }
    if (nTrksInSV > nMuInSV)
      nMuInSV = nTrksInSV;
  }
  return nMuInSV;
};


void HiOniaAnalyzer::hltReport(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
  std::map<std::string, bool> mapTriggernameToTriggerFired;
  std::map<std::string, unsigned int> mapTriggernameToHLTbit;

  for (std::vector<std::string>::const_iterator it = theTriggerNames.begin(); it != theTriggerNames.end(); ++it) {
    mapTriggernameToTriggerFired[*it] = false;
    mapTriggernameToHLTbit[*it] = 1000;
  }

  // HLTConfigProvider
  if (hltConfigInit) {
    //! Use HLTConfigProvider
    const unsigned int n = hltConfig.size();
    for (std::map<std::string, unsigned int>::iterator it = mapTriggernameToHLTbit.begin();
         it != mapTriggernameToHLTbit.end();
         it++) {
      unsigned int triggerIndex = hltConfig.triggerIndex(triggerNameMap.at(it->first));
      if (it->first == "NoTrigger")
        continue;
      if (triggerIndex >= n) {
        if (_checkTrigNames)
          std::cout << "[HiOniaAnalyzer::hltReport] --- TriggerName " << it->first << " not available in config!"
                    << std::endl;
      } else {
        it->second = triggerIndex;
        //      std::cout << "[HiOniaAnalyzer::hltReport] --- TriggerName " << it->first << " available in config!" << std::endl;
      }
    }
  }

  // Get Trigger Results
  iEvent.getByToken(_tagTriggerResultsToken, collTriggerResults);
  if (collTriggerResults.isValid() && (collTriggerResults->size() == hltConfig.size())) {
    //    std::cout << "[HiOniaAnalyzer::hltReport] --- J/psi TriggerResults IS valid in current event" << std::endl;

    // loop over Trigger Results to check if paths was fired
    for (std::vector<std::string>::iterator itHLTNames = theTriggerNames.begin(); itHLTNames != theTriggerNames.end();
         itHLTNames++) {
      const std::string triggerPathName = *itHLTNames;
      if (mapTriggernameToHLTbit[triggerPathName] < 1000) {
        if (collTriggerResults->accept(mapTriggernameToHLTbit[triggerPathName])) {
          mapTriggerNameToIntFired_[triggerPathName] = 3;
        }
        if (_isMC) {
          mapTriggerNameToPrescaleFac_[triggerPathName] = 1;
        } else {
          //-------prescale factor------------
          if (hltPrescaleInit && hltPrescaleProvider.prescaleSet(iEvent, iSetup) >= 0) {
            auto const detailedPrescaleInfo = hltPrescaleProvider.prescaleValuesInDetail<double, double>(
                iEvent, iSetup, triggerNameMap.at(triggerPathName));
            //std::pair<std::vector<std::pair<std::string,int> >,int> detailedPrescaleInfo = hltPrescaleProvider.prescaleValuesInDetail(iEvent, iSetup, triggerPathName);
            //get HLT prescale info from hltPrescaleProvider
            const int hltPrescale = detailedPrescaleInfo.second;
            //get L1 prescale info from hltPrescaleProvider
            int l1Prescale = -1;
            if (detailedPrescaleInfo.first.size() == 1) {
              l1Prescale = detailedPrescaleInfo.first.at(0).second;
            } else if (detailedPrescaleInfo.first.size() > 1) {
              l1Prescale =
                  1;  // Means it is a complex l1 seed, and for us it is only Mu3 OR Mu5, both of them unprescaled at L1
            } else if (_checkTrigNames) {
              std::cout << "[HiOniaAnalyzer::hltReport] --- L1 prescale was NOT found for TriggerName "
                        << triggerPathName << " , default L1 prescale value set to 1 " << std::endl;
            }
            //compute the total prescale = HLT prescale * L1 prescale
            mapTriggerNameToPrescaleFac_[triggerPathName] = hltPrescale * l1Prescale;
          }
        }
      }
    }
  } else
    std::cout << "[HiOniaAnalyzer::hltReport] --- TriggerResults NOT valid in current event" << std::endl;

  return;
};

int HiOniaAnalyzer::muonIDmask(const pat::Muon* muon) {
  int mask = 0;
  int type;
  for (type = muon::All; type <= muon::RPCMuLoose; type++) {
    if (muon->hasUserInt(Form("muonID_%d", type)) ? muon->userInt(Form("muonID_%d", type))
                                                  : muon::isGoodMuon(*muon, muon::SelectionType(type)))
      mask = mask | (int)pow(2, type);
  }

  return mask;
};

long int HiOniaAnalyzer::FloatToIntkey(float v) {
  float vres = std::abs(v);
  while (vres > 0.1)
    vres = vres / 10;                  //Assume argument v is always above 0.1, true for abs(Pt)
  return (long int)(10000000 * vres);  // Precision 10^-6 (i.e. 7-1) on the comparison
};
