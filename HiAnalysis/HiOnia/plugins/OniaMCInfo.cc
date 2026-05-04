#include "HiAnalysis/HiOnia/interface/HiOniaAnalyzer.h"

reco::GenParticleRef HiOniaAnalyzer::findDaughterRef(reco::GenParticleRef GenParticleDaughter, int GenParticlePDG) {
  reco::GenParticleRef GenParticleTmp = GenParticleDaughter;
  bool foundFirstDaughter = false;

  for (int j = 0; j < 1000; ++j) {
    if (GenParticleTmp.isNonnull() && GenParticleTmp->status() > 0 && GenParticleTmp->status() < 1000 &&
        GenParticleTmp->numberOfDaughters() > 0) {
      if (GenParticleTmp->pdgId() == GenParticlePDG ||
          GenParticleTmp->daughterRef(0)->pdgId() ==
              GenParticlePDG)  //if oscillating B, can take two decays to return to pdgID(B parent)
      {
        GenParticleTmp = GenParticleTmp->daughterRef(0);
      } else if (!foundFirstDaughter)  //if Tmp is not a Bc, it means Tmp is a true daughter
      {
        foundFirstDaughter = true;
        GenParticlePDG = GenParticleTmp->pdgId();
      }
    } else
      break;
  }
  if (GenParticleTmp.isNonnull() && GenParticleTmp->status() > 0 && GenParticleTmp->status() < 1000 &&
      foundFirstDaughter) {  //(GenParticleTmp->pdgId()==GenParticlePDG)) {
    GenParticleDaughter = GenParticleTmp;
  }

  return GenParticleDaughter;
};

void HiOniaAnalyzer::fillGenInfo() {
  if (Gen_Dimuon_size >= NMaxDimuons) {
    std::cout << "Too many dimuons: " << Gen_Dimuon_size << std::endl;
    std::cout << "Maximum allowed: " << NMaxDimuons << std::endl;
    return;
  }

  if (Gen_Muon_size >= NMaxMuons) {
    std::cout << "Too many muons: " << Gen_Muon_size << std::endl;
    std::cout << "Maximum allowed: " << NMaxMuons << std::endl;
    return;
  }

  if (genInfo.isValid()) {
    if (genInfo->hasBinningValues())
      Gen_pthat = genInfo->binningValues()[0];
    Gen_weight = genInfo->weight();
  }

  if (collGenParticles.isValid()) {
    //Fill the single muons, before the dimuons (important)
    for (const auto& gen : *collGenParticles){

      if (abs(gen.pdgId()) == 13 && (gen.status() == 1)) {
        Gen_Muon_type[Gen_Muon_size] = _isPromptMC ? 0 : 1;  // prompt: 0, non-prompt: 1
        Gen_Muon_charge[Gen_Muon_size] = gen.charge();

        LorentzVector muonLV = gen.p4();
	      Gen_Muon_4mom.emplace_back(muonLV);
        Gen_Muon_4mom_pt.push_back(muonLV.Pt());
        Gen_Muon_4mom_eta.push_back(muonLV.Eta());
        Gen_Muon_4mom_phi.push_back(muonLV.Phi());
        Gen_Muon_4mom_m.push_back(muonLV.M());

        //Fill map of the muon indices. Use long int keys, to avoid rounding errors on a float key. Implies a precision of 10^-6
        mapGenMuonMomToIndex_[FloatToIntkey(muonLV.Pt())] = Gen_Muon_size;

        Gen_Muon_size++;
      }
    }

    for (const auto& gen : *collGenParticles){
      
      if (abs(gen.pdgId()) == _oniaPDG && (gen.status() == 2 || (abs(gen.pdgId()) == 23 && gen.status() == 62)) &&
          gen.numberOfDaughters() >= 2) {
        reco::GenParticleRef genMuon1 = findDaughterRef(gen.daughterRef(0), gen.pdgId());
        reco::GenParticleRef genMuon2 = findDaughterRef(gen.daughterRef(1), gen.pdgId());

        if (abs(genMuon1->pdgId()) == 13 && abs(genMuon2->pdgId()) == 13 && (genMuon1->status() == 1) &&
            (genMuon2->status() == 1)) {
          Gen_Dimuon_type[Gen_Dimuon_size] = _isPromptMC ? 0 : 1;  // prompt: 0, non-prompt: 1
          std::pair<std::vector<reco::GenParticleRef>, std::pair<float, float> > MCinfo = findGenMCInfo(gen);
          Gen_Dimuon_ctau[Gen_Dimuon_size] = 10.0 * MCinfo.second.first;
          Gen_Dimuon_ctau3D[Gen_Dimuon_size] = 10.0 * MCinfo.second.second;

          if (_genealogyInfo) {
            _Gen_Dimuon_MomAndTrkBro[Gen_Dimuon_size] = MCinfo.first;
            Gen_Dimuon_momId[Gen_Dimuon_size] = _Gen_Dimuon_MomAndTrkBro[Gen_Dimuon_size][0]->pdgId();
          }

          LorentzVector quarkoniumLV = gen.p4();
	        Gen_Dimuon_4mom.emplace_back(quarkoniumLV);
      	  Gen_Dimuon_4mom_pt.push_back(quarkoniumLV.Pt());
          Gen_Dimuon_4mom_eta.push_back(quarkoniumLV.Eta());
          Gen_Dimuon_4mom_y.push_back(quarkoniumLV.Rapidity());
          Gen_Dimuon_4mom_phi.push_back(quarkoniumLV.Phi());
          Gen_Dimuon_4mom_m.push_back(quarkoniumLV.M());

          float genMuonPtDiff = 0.0;
          if (genMuon1->charge() > genMuon2->charge()) {
            Gen_Dimuon_muonPlusIndex[Gen_Dimuon_size] = IndexOfThisMuon(genMuon1->pt(), true);
            Gen_Dimuon_muonMinusIndex[Gen_Dimuon_size] = IndexOfThisMuon(genMuon2->pt(), true);
            genMuonPtDiff = genMuon1->pt() - genMuon2->pt();
          } else {
            Gen_Dimuon_muonPlusIndex[Gen_Dimuon_size] = IndexOfThisMuon(genMuon2->pt(), true);
            Gen_Dimuon_muonMinusIndex[Gen_Dimuon_size] = IndexOfThisMuon(genMuon1->pt(), true);
            genMuonPtDiff = genMuon2->pt() - genMuon1->pt();
          }

          Gen_Dimuon_Muons_pTdiff.push_back(genMuonPtDiff);

          Gen_Dimuon_size++;
        }
      }
    }
  }
  return;
};

reco::GenParticleRef HiOniaAnalyzer::findMotherRef(reco::GenParticleRef GenParticleMother, int GenParticlePDG) {
  for (int i = 0; i < 1000; ++i) {
    if (GenParticleMother.isNonnull() && (GenParticleMother->pdgId() == GenParticlePDG) &&
        GenParticleMother->numberOfMothers() > 0) {
      GenParticleMother = GenParticleMother->motherRef();
    } else
      break;
  }
  return GenParticleMother;
};

std::vector<reco::GenParticleRef> HiOniaAnalyzer::GenBrothers(reco::GenParticleRef GenParticleMother, int GenDimuonPDG) {
  bool foundDimuon = false;
  std::vector<reco::GenParticleRef> res;

  if (!GenParticleMother.isNonnull())
    return res;

  for (int i = 0; i < (int)GenParticleMother->numberOfDaughters(); i++) {
    reco::GenParticleRef dau = findDaughterRef(GenParticleMother->daughterRef(i), GenParticleMother->pdgId());
    for (int l = 0; l < 100; l++) {  //avoid having a daughter of same pdgId
      if (!(dau.isNonnull() && dau->status() > 0 && dau->status() < 1000))
        break;
      if (dau->pdgId() == GenParticleMother->pdgId() && dau->numberOfDaughters() == 1)
        dau = findDaughterRef(dau->daughterRef(0), dau->pdgId());
      else
        break;
    }

    if (!(dau.isNonnull() && dau->status() > 0 && dau->status() < 1000))
      continue;

    if (isChargedTrack(dau->pdgId())) {
      res.push_back(dau);
    }
    if (dau->pdgId() == GenDimuonPDG) {
      foundDimuon = true;  //continue;
    }

    for (int j = 0; j < (int)dau->numberOfDaughters(); j++) {
      reco::GenParticleRef grandDau = findDaughterRef(dau->daughterRef(j), dau->pdgId());
      for (int l = 0; l < 100; l++) {  //avoid having a daughter of same pdgId
        if (!(grandDau.isNonnull() && grandDau->status() > 0 && grandDau->status() < 1000))
          break;
        if (grandDau->pdgId() == dau->pdgId() && grandDau->numberOfDaughters() == 1)
          grandDau = findDaughterRef(grandDau->daughterRef(0), grandDau->pdgId());
        else
          break;
      }

      if (!(grandDau.isNonnull() && grandDau->status() > 0 && grandDau->status() < 1000))
        continue;

      if (isChargedTrack(grandDau->pdgId())) {
        res.push_back(grandDau);
      }
      if (grandDau->pdgId() == GenDimuonPDG) {
        foundDimuon = true;  //continue;
      }

      for (int k = 0; k < (int)grandDau->numberOfDaughters(); k++) {
        reco::GenParticleRef ggrandDau = findDaughterRef(grandDau->daughterRef(k), grandDau->pdgId());
        for (int l = 0; l < 100; l++) {  //avoid having a daughter of same pdgId
          if (!(ggrandDau.isNonnull() && ggrandDau->status() > 0 && ggrandDau->status() < 1000))
            break;
          if (ggrandDau->pdgId() == grandDau->pdgId() && ggrandDau->numberOfDaughters() == 1)
            ggrandDau = findDaughterRef(ggrandDau->daughterRef(0), ggrandDau->pdgId());
          else
            break;
        }

        if (!(ggrandDau.isNonnull() && ggrandDau->status() > 0 && ggrandDau->status() < 1000))
          continue;
        if (isChargedTrack(ggrandDau->pdgId())) {
          res.push_back(ggrandDau);
        }
        if (ggrandDau->pdgId() == GenDimuonPDG) {
          foundDimuon = true;  //continue;
        }
      }
    }
  }

  if (!foundDimuon) {
    cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!! Incoherence in genealogy: Dimuon not found in the daughters!\n" << endl;
  }
  // if(!isAbHadron(GenParticleMother->pdgId())){
  //   cout<<"\n!!!!!!!!!!!!!!!!!!!!!!!!!!!! Dimuon ancestor is not a b-hadron! pdgID(mother of this ancestor) = "<<findMotherRef(GenParticleMother->motherRef() , GenParticleMother->pdgId())->pdgId()<<endl;
  // }

  return res;
};

std::pair<std::vector<reco::GenParticleRef>, std::pair<float, float> > HiOniaAnalyzer::findGenMCInfo(
    const reco::GenParticle& genDimuon) {
  float trueLife = -99.;
  float trueLife3D = -99.;
  std::vector<reco::GenParticleRef> DimuonBrothers;

  if (genDimuon.numberOfMothers() > 0) {
    math::XYZPoint trueVtxMom(0.0, 0.0, 0.0);

    math::XYZPoint trueVtx = genDimuon.vertex();
    
    bool aBhadron = false;
    reco::GenParticleRef Dimuonmom_final;
    reco::GenParticleRef Dimuonmom = findMotherRef(genDimuon.motherRef(), genDimuon.pdgId());

    if (Dimuonmom.isNull()) {
      std::pair<float, float> trueLifePair = std::make_pair(trueLife, trueLife3D);
      std::pair<std::vector<reco::GenParticleRef>, std::pair<float, float> > result =
          std::make_pair(DimuonBrothers, trueLifePair);
      return result;
    } else if (Dimuonmom->numberOfMothers() <= 0) {
      if (isAbHadron(Dimuonmom->pdgId())) {
        Dimuonmom_final = Dimuonmom;
        aBhadron = true;
      }
    }

    else {
      reco::GenParticleRef Dimuongrandmom = findMotherRef(Dimuonmom->motherRef(), Dimuonmom->pdgId());
      if (isAbHadron(Dimuonmom->pdgId())) {
        if (Dimuongrandmom.isNonnull() && isAMixedbHadron(Dimuonmom->pdgId(), Dimuongrandmom->pdgId())) {
          Dimuonmom_final = Dimuongrandmom;
        } else {
          Dimuonmom_final = Dimuonmom;
        }
        aBhadron = true;
      }

      else if (Dimuongrandmom.isNonnull() && isAbHadron(Dimuongrandmom->pdgId())) {
        if (Dimuongrandmom->numberOfMothers() <= 0) {
          Dimuonmom_final = Dimuongrandmom;
        } else {
          reco::GenParticleRef DimuonGrandgrandmom = findMotherRef(Dimuongrandmom->motherRef(), Dimuongrandmom->pdgId());
          if (DimuonGrandgrandmom.isNonnull() && isAMixedbHadron(Dimuongrandmom->pdgId(), DimuonGrandgrandmom->pdgId())) {
            Dimuonmom_final = DimuonGrandgrandmom;
          } else {
            Dimuonmom_final = Dimuongrandmom;
          }
        }
        aBhadron = true;
      }

      //This is to forcefully find the b-like mother of Dimuon
      else if (Dimuongrandmom.isNonnull() && Dimuongrandmom->numberOfMothers() > 0) {
        reco::GenParticleRef DimuonGrandgrandmom = findMotherRef(Dimuongrandmom->motherRef(), Dimuongrandmom->pdgId());
        if (DimuonGrandgrandmom.isNonnull() && isAbHadron(DimuonGrandgrandmom->pdgId())) {
          Dimuonmom_final = DimuonGrandgrandmom;
          aBhadron = true;
        }
      }
    }
    if (!aBhadron) {
      Dimuonmom_final = Dimuonmom;
    }

    if (Dimuonmom_final.isNonnull()) {
      trueVtxMom = Dimuonmom_final->vertex();
      if (_genealogyInfo) {
        DimuonBrothers = GenBrothers(Dimuonmom_final, genDimuon.pdgId());
      }
      DimuonBrothers.insert(DimuonBrothers.begin(), Dimuonmom_final);
    }

    auto vdiff = trueVtx - trueVtxMom;
    trueLife = std::sqrt(vdiff.Perp2()) * genDimuon.mass() / genDimuon.pt();
    trueLife3D = std::sqrt(vdiff.Mag2()) * genDimuon.mass() / genDimuon.p();
  }

  std::pair<float, float> trueLifePair = std::make_pair(trueLife, trueLife3D);
  std::pair<std::vector<reco::GenParticleRef>, std::pair<float, float> > result =
      std::make_pair(DimuonBrothers, trueLifePair);
  return result;
};

//Find the indices of the reconstructed dimuon matching each generated resonance (when the two daughter muons are reconstructed), and vice versa
void HiOniaAnalyzer::fillDimuonMatchingInfo() {
  for (int igen = 0; igen < Gen_Dimuon_size; igen++) {
    Gen_Dimuon_whichRec[igen] = -1;
    int Reco_muonPlusIndex =
        Gen_Muon_whichRec[Gen_Dimuon_muonPlusIndex[igen]];  //index of the reconstructed mupl associated to the generated mupl of Dimuon
    int Reco_muonMinusIndex =
        Gen_Muon_whichRec[Gen_Dimuon_muonMinusIndex[igen]];  //index of the reconstructed mumi associated to the generated mumi of Dimuon

    if ((Reco_muonPlusIndex >= 0) && (Reco_muonMinusIndex >= 0)) {  //Search for Reco_Dimuon only if both muons are reco
      for (int irec = 0; irec < Reco_Dimuon_size; irec++) {
        if (((Reco_muonPlusIndex == Reco_Dimuon_muonPlusIndex[irec]) &&
             (Reco_muonMinusIndex == Reco_Dimuon_muonMinusIndex[irec])) ||  //the charges might be wrong in reco
            ((Reco_muonPlusIndex == Reco_Dimuon_muonMinusIndex[irec]) && (Reco_muonMinusIndex == Reco_Dimuon_muonPlusIndex[irec]))) {
          Gen_Dimuon_whichRec[igen] = irec;
          break;
        }
      }

      if (Gen_Dimuon_whichRec[igen] == -1)
        Gen_Dimuon_whichRec[igen] = -2;  //Means the two muons were reconstructed, but the dimuon was not selected
    }
  }

  //Find the index of generated J/psi associated to a reco QQ
  for (int irec = 0; irec < Reco_Dimuon_size; irec++) {
    Reco_Dimuon_whichGen[irec] = -1;

    for (int igen = 0; igen < Gen_Dimuon_size; igen++) {
      if ((Gen_Dimuon_whichRec[igen] == irec)) {
        Reco_Dimuon_whichGen[irec] = igen;
        break;
      }
    }
  }
};
