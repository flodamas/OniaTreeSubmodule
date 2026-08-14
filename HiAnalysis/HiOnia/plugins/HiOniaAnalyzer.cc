#include "HiAnalysis/HiOnia/interface/HiOniaAnalyzer.h"

HiOniaAnalyzer::HiOniaAnalyzer(const edm::ParameterSet& iConfig)
    : _patMuonToken(consumes<pat::MuonCollection>(iConfig.getParameter<edm::InputTag>("srcMuon"))),
      _patMuonNoTrigToken(consumes<pat::MuonCollection>(iConfig.getParameter<edm::InputTag>("srcMuonNoTrig"))),
      _patDimuonToken(consumes<pat::CompositeCandidateCollection>(iConfig.getParameter<edm::InputTag>("srcDimuon"))),
      //_recoTracksToken(consumes<reco::TrackCollection>(iConfig.getParameter<edm::InputTag>("srcTracks"))),
      _genParticleToken(consumes<reco::GenParticleCollection>(iConfig.getParameter<edm::InputTag>("genParticles"))),
      _genInfoToken(consumes<GenEventInfoProduct>(edm::InputTag("generator"))),
      _thePVsToken(consumes<reco::VertexCollection>(iConfig.getParameter<edm::InputTag>("primaryVertexTag"))),
      _SVToken(
          consumes<reco::VertexCollection>(iConfig.getParameter<edm::InputTag>("srcSV"))),  //consumes<edm::View<VTX>>
      _tagTriggerResultsToken(
          consumes<edm::TriggerResults>(iConfig.getParameter<edm::InputTag>("triggerResultsLabel"))),
      _centralityTagToken(consumes<reco::Centrality>(iConfig.getParameter<edm::InputTag>("CentralitySrc"))),
      _centralityBinTagToken(consumes<int>(iConfig.getParameter<edm::InputTag>("CentralityBinSrc"))),
      _evtPlaneTagToken(consumes<reco::EvtPlaneCollection>(iConfig.getParameter<edm::InputTag>("EvtPlane"))),
      _histfilename(iConfig.getParameter<std::string>("histFileName")),
      _datasetname(iConfig.getParameter<std::string>("dataSetName")),
      _muonSel(iConfig.getParameter<std::string>("muonSel")),
      _centralityranges(iConfig.getParameter<std::vector<double> >("centralityRanges")),
      _dblTriggerPathNames(iConfig.getParameter<std::vector<string> >("dblTriggerPathNames")),
      _sglTriggerPathNames(iConfig.getParameter<std::vector<string> >("sglTriggerPathNames")),
      _applycuts(iConfig.getParameter<bool>("applyCuts")),
      _SumETvariables(iConfig.getParameter<bool>("SumETvariables")),
      _selTightGlobalMuon(iConfig.getParameter<bool>("selTightGlobalMuon")),
      _muonLessPrimaryVertex(iConfig.getParameter<bool>("muonLessPV")),
      _useSVfinder(iConfig.getParameter<bool>("useSVfinder")),
      _useBS(iConfig.getParameter<bool>("useBeamSpot")),
      _useRapidity(iConfig.getParameter<bool>("useRapidity")),
      _removeSignal(iConfig.getUntrackedParameter<bool>("removeSignalEvents", false)),
      _removeMuons(iConfig.getUntrackedParameter<bool>("removeTrueMuons", false)),
      _storeSs(iConfig.getParameter<bool>("storeSameSign")),
      _AtLeastOneCand(iConfig.getParameter<bool>("AtLeastOneCand")),
      _combineCategories(iConfig.getParameter<bool>("combineCategories")),
      _fillTree(iConfig.getParameter<bool>("fillTree")),
      _fillHistos(iConfig.getParameter<bool>("fillHistos")),
      _fillSingleMuons(iConfig.getParameter<bool>("fillSingleMuons")),
      _isHI(iConfig.getUntrackedParameter<bool>("isHI", false)),
      _isPA(iConfig.getUntrackedParameter<bool>("isPA", true)),
      _isMC(iConfig.getUntrackedParameter<bool>("isMC", false)),
      _isPromptMC(iConfig.getUntrackedParameter<bool>("isPromptMC", true)),
      _useEvtPlane(iConfig.getUntrackedParameter<bool>("useEvtPlane", false)),
      _genealogyInfo(iConfig.getParameter<bool>("genealogyInfo")),
      _oniaPDG(iConfig.getParameter<int>("oniaPDG")),
      _OneMatchedHLTMu(iConfig.getParameter<int>("OneMatchedHLTMu")),
      _checkTrigNames(iConfig.getParameter<bool>("checkTrigNames")),
      _genOnly(iConfig.getParameter<bool>("genOnly")),
      _addMuonIsolation(iConfig.getParameter<bool>("addMuonIsolation")),
      hltPrescaleProvider(iConfig, consumesCollector(), *this),
      _iConfig(iConfig) {
  usesResource(TFileService::kSharedResource);


  //now do whatever initialization is needed
  nEvents = 0;
  passedCandidates = 0;


  std::stringstream centLabel;
  for (unsigned int iCent = 0; iCent < _centralityranges.size(); ++iCent) {
    if (iCent == 0)
      centLabel << "00" << _centralityranges.at(iCent);
    else
      centLabel << _centralityranges.at(iCent - 1) << _centralityranges.at(iCent);

    theCentralities.push_back(centLabel.str());
    centLabel.str("");
  }
  theCentralities.push_back("MinBias");

  theSign.push_back("pm");
  if (_storeSs) {
    theSign.push_back("pp");
    theSign.push_back("mm");
  }

  NTRIGGERS_DBL = _dblTriggerPathNames.size();
  NTRIGGERS = NTRIGGERS_DBL + _sglTriggerPathNames.size() + 1;  // + 1 for "NoTrigger"
  std::cout << "NTRIGGERS_DBL = " << NTRIGGERS_DBL << "\t NTRIGGERS_SGL = " << _sglTriggerPathNames.size()
            << "\t NTRIGGERS = " << NTRIGGERS << std::endl;
  nTrig = NTRIGGERS - 1;

  isTriggerMatched[0] = true;  // first entry 'hardcoded' true to accept "all" events
  theTriggerNames.push_back("NoTrigger");

  for (unsigned int iTr = 1; iTr < NTRIGGERS; ++iTr) {
    isTriggerMatched[iTr] = false;

    if (iTr <= NTRIGGERS_DBL) {
      theTriggerNames.push_back(_dblTriggerPathNames.at(iTr - 1));
    } else {
      theTriggerNames.push_back(_sglTriggerPathNames.at(iTr - NTRIGGERS_DBL - 1));
    }
    std::cout << " Trigger " << iTr << "\t" << theTriggerNames[iTr] << std::endl;
  }

  if (_OneMatchedHLTMu >= (int)NTRIGGERS) {
    std::cout
        << "WARNING: the _OneMatchedHLTMu parameter is asking for a wrong trigger number. No matching will be done."
        << std::endl;
    _OneMatchedHLTMu = -1;
  }
  if (_OneMatchedHLTMu > -1)
    std::cout << " Will keep only dimuons (trimuons) that have one (two) daughters matched to "
              << theTriggerNames[_OneMatchedHLTMu] << " filter." << std::endl;


  //DimuonMassMin = 2.6;
  //DimuonMassMax = 3.5;

  //DimuonPtMin = _ptbinranges[0];
  //std::cout << "Pt min = " << DimuonPtMin << std::endl;
  //DimuonPtMax = _ptbinranges[_ptbinranges.size() - 1];
  //std::cout << "Pt max = " << DimuonPtMax << std::endl;

  //DimuonRapMin = _etabinranges[0];
  //std::cout << "Rap min = " << DimuonRapMin << std::endl;
  //DimuonRapMax = _etabinranges[_etabinranges.size() - 1];
  //std::cout << "Rap max = " << DimuonRapMax << std::endl;

  for (std::vector<std::string>::iterator it = theTriggerNames.begin(); it != theTriggerNames.end(); ++it) {
    mapTriggerNameToIntFired_[*it] = -9999;
    mapTriggerNameToPrescaleFac_[*it] = -1;
  }
};

HiOniaAnalyzer::~HiOniaAnalyzer() {
  // do anything here that needs to be done at destruction time
  // (e.g. close files, deallocate resources etc.)
  
};

void HiOniaAnalyzer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
  //   using namespace edm;
  InitEvent();
  nEvents++;
  hStats->Fill(BIN_nEvents);

  runNb = iEvent.id().run();
  eventNb = iEvent.id().event();
  lumiSection = iEvent.luminosityBlock();


  edm::Handle<reco::VertexCollection> privtxs;
  iEvent.getByToken(_thePVsToken, privtxs);
  reco::VertexCollection::const_iterator privtx;

  edm::Handle<reco::Centrality> centrality;
  edm::Handle<int> cbin_;

  if(_genOnly) goto genOnly;


  if (privtxs.isValid()) {
    nPV = privtxs->size();

    if (privtxs->begin() != privtxs->end()) {
      privtx = privtxs->begin();
      RefVtx = privtx->position();
      RefVtx_xError = privtx->xError();
      RefVtx_yError = privtx->yError();
      RefVtx_zError = privtx->zError();
    } else {
      RefVtx.SetXYZ(0., 0., 0.);
      RefVtx_xError = 0.0;
      RefVtx_yError = 0.0;
      RefVtx_zError = 0.0;
    }

    zVtx = RefVtx.Z();

    hZVtx->Fill(zVtx);
    hPileUp->Fill(nPV);
  } else {
    std::cout << "ERROR: privtxs is NULL or not isValid ! Return now" << std::endl;
    return;
  }

  this->hltReport(iEvent, iSetup);

  for (unsigned int iTr = 1; iTr < theTriggerNames.size(); iTr++) {
    if (mapTriggerNameToIntFired_[theTriggerNames.at(iTr)] == 3) {
      HLTriggers += pow(2, iTr - 1);
      hStats->Fill(iTr);  // event info
    }
    trigPrescale[iTr - 1] = mapTriggerNameToPrescaleFac_[theTriggerNames.at(iTr)];
  }

  
  if (_isHI || _isPA) {
    iEvent.getByToken(_centralityTagToken, centrality);
    iEvent.getByToken(_centralityBinTagToken, cbin_);
  }
  if (centrality.isValid() && cbin_.isValid()) {
    centBin = *cbin_;
    hCent->Fill(centBin);

    for (unsigned int iCent = 0; iCent < _centralityranges.size(); ++iCent) {
      if ((_isHI && centBin < _centralityranges.at(iCent) / 0.5) || (_isPA && centBin < _centralityranges.at(iCent))) {
        theCentralityBin = iCent;
        break;
      }
    }

    Npix = (Short_t)centrality->multiplicityPixel();
    NpixelTracks = (Short_t)centrality->NpixelTracks();
    Ntracks = (Short_t)centrality->Ntracks();
    NtracksPtCut = centrality->NtracksPtCut();
    NtracksEtaCut = centrality->NtracksEtaCut();
    NtracksEtaPtCut = centrality->NtracksEtaPtCut();
    

    if (_SumETvariables) {
      SumET_HF = centrality->EtHFtowerSum();
      SumET_HFplus = centrality->EtHFtowerSumPlus();
      SumET_HFminus = centrality->EtHFtowerSumMinus();
      SumET_HFplusEta4 = centrality->EtHFtruncatedPlus();
      SumET_HFminusEta4 = centrality->EtHFtruncatedMinus();

      SumET_HFhit = centrality->EtHFhitSum();
      SumET_HFhitPlus = centrality->EtHFhitSumPlus();
      SumET_HFhitMinus = centrality->EtHFhitSumMinus();

      SumET_ZDC = centrality->zdcSum();
      SumET_ZDCplus = centrality->zdcSumPlus();
      SumET_ZDCminus = centrality->zdcSumMinus();

      SumET_EEplus = centrality->EtEESumPlus();
      SumET_EEminus = centrality->EtEESumMinus();
      SumET_EE = centrality->EtEESum();
      SumET_EB = centrality->EtEBSum();
      SumET_ET = centrality->EtMidRapiditySum();
    }
  } else {
    centBin = 0;
    theCentralityBin = 0;

    Npix = 0;
    NpixelTracks = 0;
    Ntracks = 0;
    NtracksPtCut = 0;
    NtracksEtaCut = 0;
    NtracksEtaPtCut = 0;

    SumET_HF = 0;
    SumET_HFplus = 0;
    SumET_HFminus = 0;
    SumET_HFplusEta4 = 0;
    SumET_HFminusEta4 = 0;

    SumET_HFhit = 0;
    SumET_HFhitPlus = 0;
    SumET_HFhitMinus = 0;

    SumET_ZDC = 0;
    SumET_ZDCplus = 0;
    SumET_ZDCminus = 0;
    SumET_EEplus = 0;
    SumET_EEminus = 0;
    SumET_EE = 0;
    SumET_EB = 0;
    SumET_ET = 0;
  }

  if ((_isHI || _isPA) && _useEvtPlane) {
    nEP = 0;
    edm::Handle<reco::EvtPlaneCollection> flatEvtPlanes;
    iEvent.getByToken(_evtPlaneTagToken, flatEvtPlanes);
    if (flatEvtPlanes.isValid()) {
      for (reco::EvtPlaneCollection::const_iterator rp = flatEvtPlanes->begin(); rp != flatEvtPlanes->end(); rp++) {
        rpAng_origin[nEP] = rp->angle(0);   // Using Event Plane Level 0 -> w/o recentering and w/o flattening. 
        rpSin_origin[nEP] = rp->sumSin(0);  // Using Event Plane Level 0 -> w/o recentering and w/o flattening. 
        rpCos_origin[nEP] = rp->sumCos(0);  // Using Event Plane Level 0 -> w/o recentering and w/o flattening.
        rpAng[nEP] = rp->angle(2);   // Using Event Plane Level 2 -> Includes recentering and flattening.
        rpSin[nEP] = rp->sumSin(2);  // Using Event Plane Level 2 -> Includes recentering and flattening.
        rpCos[nEP] = rp->sumCos(2);  // Using Event Plane Level 2 -> Includes recentering and flattening.
        nEP++;
      }
    } else if (!_isMC) {
      std::cout << "Warning! Can't get flattened hiEvtPlane product!" << std::endl;
    }
  }

  iEvent.getByToken(_patDimuonToken, collDimuon);
  iEvent.getByToken(_patMuonToken, collMuon);
  iEvent.getByToken(_patMuonNoTrigToken, collMuonNoTrig);

  if (_useSVfinder)
    iEvent.getByToken(_SVToken, SVs);

  // APPLY CUTS
  this->makeCuts(_storeSs);
  

  if (_fillSingleMuons || !_AtLeastOneCand  || !_isMC) {  //not storing the mu reconstructed info if we do a trimuon MC and there is no reco trimuon
    //_fillSingleMuons is checked within the fillRecoMuons function: the info on the wanted muons was stored in the makeCuts function
    this->fillRecoMuons(theCentralityBin);

  }

  this->fillRecoHistos();

 genOnly:
  if (_isMC) {
    //GEN info
    iEvent.getByToken(_genParticleToken, collGenParticles);
    iEvent.getByToken(_genInfoToken, genInfo);
    this->fillGenInfo();

    //MC MATCHING info
    this->fillMuMatchingInfo();  //Needs to be done after fillGenInfo, and the filling of reco muons collections
    this->fillDimuonMatchingInfo();  //Needs to be done after fillMuMatchingInfo
    
  }

  //keeping events with at least ONE CANDIDATE when asked
  bool oneGoodCand = !_AtLeastOneCand;  //if !_AtLeastOneCand, pass in all cases
  if (_AtLeastOneCand) {
    if (Reco_Dimuon_size > 0)
      oneGoodCand = true;
  }

  // ---- Fill the tree with this event only if AtLeastOneCand=false OR if there is at least one dimuon candidate in the event (or at least one trimuon cand if doTrimuons=true) ----
  if (_fillTree && oneGoodCand)
    myTree->Fill();

  return;
};

void HiOniaAnalyzer::fillRecoHistos() {
  
    
    for (unsigned int count = 0; count < _thePassedCands.size(); count++) {
        const pat::CompositeCandidate* aDimuonCand = _thePassedCands.at(count);

        this->checkTriggers(aDimuonCand);
        if (_fillTree)
          this->fillTreeDimuon(count);

        for (unsigned int iTr = 0; iTr < NTRIGGERS; ++iTr) {
          if (isTriggerMatched[iTr]) {
            this->fillRecoDimuon(count, theTriggerNames.at(iTr), theCentralities.at(theCentralityBin));
          }
        }
    }

  return;
};

void HiOniaAnalyzer::fillTreeMuon(const pat::Muon* muon, int iType, ULong64_t trigBits) {
  if (Reco_Muon_size >= NMaxMuons) {
    std::cout << "Too many muons: " << Reco_Muon_size << std::endl;
    std::cout << "Maximum allowed: " << NMaxMuons << std::endl;
    return;
  }

  if (muon != nullptr) {
    Reco_Muon_charge[Reco_Muon_size] = muon->charge();
    Reco_Muon_type[Reco_Muon_size] = iType;

    LorentzVector vMuon = muon->p4();
    Reco_Muon_4mom.emplace_back(vMuon);
    
    Reco_Muon_4mom_pt.push_back(muon->pt());
    Reco_Muon_4mom_eta.push_back(muon->eta());
    Reco_Muon_4mom_phi.push_back(muon->phi());
    Reco_Muon_4mom_m.push_back(vMuon.mass());
    
    Reco_Muon_L1_4mom_eta.push_back(muon->hasUserFloat("l1Eta") ? muon->userFloat("l1Eta") : -99);
    Reco_Muon_L1_4mom_phi.push_back(muon->hasUserFloat("l1Phi") ? muon->userFloat("l1Phi") : -99);

    //Fill map of the muon indices. Use long int keys, to avoid rounding errors on a float key. Implies a precision of 10^-6
    mapMuonMomToIndex_[FloatToIntkey(muon->pt())] = Reco_Muon_size;

    Reco_Muon_trig[Reco_Muon_size] = trigBits;

    reco::TrackRef iTrack = muon->innerTrack();
    //reco::TrackRef bestTrack = muon->muonBestTrack();

    Reco_Muon_InTightAcc[Reco_Muon_size] = isInAcceptance(vMuon.eta(), vMuon.pt(), "GLB");
    Reco_Muon_InLooseAcc[Reco_Muon_size] = isInAcceptance(vMuon.eta(), vMuon.pt(), "GLBSOFT");
    Reco_Muon_SelectionType[Reco_Muon_size] = muonIDmask(muon);
    Reco_Muon_nStationsMatched[Reco_Muon_size] = muon->numberOfMatchedStations();
    Reco_Muon_isPF[Reco_Muon_size] = muon->isPFMuon();
    Reco_Muon_isTracker[Reco_Muon_size] = muon->isTrackerMuon();
    Reco_Muon_isGlobal[Reco_Muon_size] = muon->isGlobalMuon();

    // Muon selection IDs available from PAT selectors at https://github.com/cms-sw/cmssw/blob/e38ffb66b72775681a950c7019dd04133df699b9/DataFormats/MuonReco/interface/Muon.h#L202
    // see also Muon POG's recommendations for Run 3 https://muon-wiki.docs.cern.ch/guidelines/recommendations/

    // cut-based
    Reco_Muon_isSoftCutBased[Reco_Muon_size] = muon->passed(reco::Muon::SoftCutBasedId);
    Reco_Muon_isHybridSoft[Reco_Muon_size] = isHybridSoftMuon(muon);
    Reco_Muon_isLooseCutBased[Reco_Muon_size] = muon->passed(reco::Muon::CutBasedIdLoose);
    Reco_Muon_isMediumCutBased[Reco_Muon_size] = muon->passed(reco::Muon::CutBasedIdMedium);
    Reco_Muon_isTightCutBased[Reco_Muon_size] = muon->passed(reco::Muon::CutBasedIdTight);

    // MVA-based
    Reco_Muon_softMVAValue[Reco_Muon_size] = muon->softMvaRun3Value(); // for heavy-flavor low-pT muon selection (MUO-24-001)
    Reco_Muon_muonMVAValue[Reco_Muon_size] = muon->mvaIDValue(); // for pT > 10 GeV prompt muon production (MUO-22-001)

    Reco_Muon_candType[Reco_Muon_size] = (Short_t)(muon->hasUserInt("candType")) ? (muon->userInt("candType")) : (-1);

    Reco_Muon_TMOneStaTight[Reco_Muon_size] = muon::isGoodMuon(*muon, muon::TMOneStationTight);

    Reco_Muon_localChi2[Reco_Muon_size] = muon->combinedQuality().chi2LocalPosition;
    Reco_Muon_kink[Reco_Muon_size] = muon->combinedQuality().trkKink;
    Reco_Muon_segmentComp[Reco_Muon_size] = muon->segmentCompatibility(reco::Muon::SegmentAndTrackArbitration);

    //Reco_Muon_normChi2_bestTracker[Reco_Muon_size] = bestTrack->normalizedChi2();

    if (iTrack.isNonnull() && iTrack.isAvailable()) {
        Reco_Muon_highPurity[Reco_Muon_size] = iTrack->quality(reco::TrackBase::highPurity);
        Reco_Muon_nTrkHits[Reco_Muon_size] = iTrack->found();
        Reco_Muon_normChi2_inner[Reco_Muon_size] =
            (muon->hasUserFloat("trackChi2") ? muon->userFloat("trackChi2") : iTrack->normalizedChi2());
        Reco_Muon_nPixValHits[Reco_Muon_size] = iTrack->hitPattern().numberOfValidPixelHits();
        Reco_Muon_nPixWMea[Reco_Muon_size] = iTrack->hitPattern().pixelLayersWithMeasurement();
        Reco_Muon_nTrkWMea[Reco_Muon_size] = iTrack->hitPattern().trackerLayersWithMeasurement();
        Reco_Muon_dxy[Reco_Muon_size] = iTrack->dxy(RefVtx);
        Reco_Muon_dxyErr[Reco_Muon_size] = iTrack->dxyError();
        Reco_Muon_dz[Reco_Muon_size] = iTrack->dz(RefVtx);
        Reco_Muon_dzErr[Reco_Muon_size] = iTrack->dzError();
        Reco_Muon_pt_inner[Reco_Muon_size] = iTrack->pt();
        Reco_Muon_ptErr_inner.push_back(iTrack->ptError());
        Reco_Muon_validFraction[Reco_Muon_size] = iTrack->validFraction();
    } else {
        Reco_Muon_highPurity[Reco_Muon_size] = false;
        Reco_Muon_nTrkHits[Reco_Muon_size] = -1;
        Reco_Muon_normChi2_inner[Reco_Muon_size] = 999.f;
        Reco_Muon_nPixValHits[Reco_Muon_size] = -1;
        Reco_Muon_nPixWMea[Reco_Muon_size] = -1;
        Reco_Muon_nTrkWMea[Reco_Muon_size] = -1;
        Reco_Muon_dxy[Reco_Muon_size] = -999.f;
        Reco_Muon_dxyErr[Reco_Muon_size] = -999.f;
        Reco_Muon_dz[Reco_Muon_size] = -999.f;
        Reco_Muon_dzErr[Reco_Muon_size] = -999.f;
        Reco_Muon_pt_inner[Reco_Muon_size] = -1.f;
        Reco_Muon_ptErr_inner.push_back(-1.f);
        Reco_Muon_validFraction[Reco_Muon_size] = -1.f;
        
    }

    if (muon->isGlobalMuon()) {
        reco::TrackRef gTrack = muon->globalTrack();
        Reco_Muon_nMuValHits[Reco_Muon_size] = gTrack->hitPattern().numberOfValidMuonHits();
        Reco_Muon_normChi2_global[Reco_Muon_size] = gTrack->normalizedChi2();
        //Reco_Muon_pt_global[Reco_Muon_size] = gTrack->pt();
        //Reco_Muon_ptErr_global[Reco_Muon_size] = gTrack->ptError();
    } else {
        Reco_Muon_nMuValHits[Reco_Muon_size] = -1;
        Reco_Muon_normChi2_global[Reco_Muon_size] = 999;
        //Reco_Muon_pt_global[Reco_Muon_size] = -1;
        //Reco_Muon_ptErr_global[Reco_Muon_size] = -1;
    }
    

    if (_isMC) {
      Reco_Muon_pTrue[Reco_Muon_size] =
          ((muon->genParticleRef()).isNonnull()) ? ((float)(muon->genParticleRef())->p()) : (-1);
      if (_genealogyInfo) {
        Reco_Muon_simExtType[Reco_Muon_size] = muon->simExtType();
      }
    }
  } else {
    std::cout << "ERROR: 'muon' pointer in fillTreeMuon is NULL ! Return now" << std::endl;
    return;
  }

  // Isolation variables
  if (_addMuonIsolation){
    
    Reco_Muon_passesPFIsoLoose.push_back(muon->passed(reco::Muon::PFIsoLoose));
    Reco_Muon_passesPFIsoMedium.push_back(muon->passed(reco::Muon::PFIsoMedium));
    Reco_Muon_passesPFIsoTight.push_back(muon->passed(reco::Muon::PFIsoTight));
    Reco_Muon_passesPFIsoVeryTight.push_back(muon->passed(reco::Muon::PFIsoVeryTight));

    Reco_Muon_isoTrackSumPt.push_back(muon->isolationR03().sumPt);

    Reco_Muon_passesMultiIsoMedium.push_back(muon->passed(reco::Muon::MultiIsoMedium));

    Reco_Muon_HIMVAIso.push_back(muon->hasUserFloat("hiMVAIso") ? muon->userFloat("hiMVAIso") : -99);
    for (auto& w : Reco_Muon_HIMVAIsoWPs)
      w.second.push_back(muon->hasUserInt("hiMVAIso"+w.first) && muon->userInt("hiMVAIso"+w.first)>0);
  }

  Reco_Muon_size++;
  return;
};

void HiOniaAnalyzer::fillTreeDimuon(int count) {
  if (Reco_Dimuon_size >= NMaxDimuons) {
    std::cout << "Too many dimuons: " << Reco_Dimuon_size << std::endl;
    std::cout << "Maximum allowed: " << NMaxDimuons << std::endl;
    return;
  }

  const pat::CompositeCandidate* aDimuonCandidate = _thePassedCands.at(count);

  if (aDimuonCandidate != nullptr) {
    const pat::Muon* muon1 = dynamic_cast<const pat::Muon*>(aDimuonCandidate->daughter("muon1"));
    const pat::Muon* muon2 = dynamic_cast<const pat::Muon*>(aDimuonCandidate->daughter("muon2"));

    ULong64_t trigBits = 0;
    for (unsigned int iTr = 1; iTr < NTRIGGERS; ++iTr) {
      if (isTriggerMatched[iTr]) {
        trigBits += pow(2, iTr - 1);
      }
    }

    if (muon1 == nullptr || muon2 == nullptr) {
      std::cout << "ERROR: 'muon1' or 'muon2' pointer in fillTreeDimuon is NULL ! Return now" << std::endl;
      return;
    } else {
      Reco_Dimuon_sign[Reco_Dimuon_size] = muon1->charge() + muon2->charge();
      Reco_Dimuon_type[Reco_Dimuon_size] = _thePassedCats.at(count);

      Reco_Dimuon_trig[Reco_Dimuon_size] = trigBits;

      if (!(_isHI) && _muonLessPrimaryVertex && aDimuonCandidate->hasUserData("muonlessPV")) {
        RefVtx = (*aDimuonCandidate->userData<reco::Vertex>("muonlessPV")).position();
        RefVtx_xError = (*aDimuonCandidate->userData<reco::Vertex>("muonlessPV")).xError();
        RefVtx_yError = (*aDimuonCandidate->userData<reco::Vertex>("muonlessPV")).yError();
        RefVtx_zError = (*aDimuonCandidate->userData<reco::Vertex>("muonlessPV")).zError();
      } else if (!_muonLessPrimaryVertex && aDimuonCandidate->hasUserData("PVwithmuons")) {
        RefVtx = (*aDimuonCandidate->userData<reco::Vertex>("PVwithmuons")).position();
        RefVtx_xError = (*aDimuonCandidate->userData<reco::Vertex>("PVwithmuons")).xError();
        RefVtx_yError = (*aDimuonCandidate->userData<reco::Vertex>("PVwithmuons")).yError();
        RefVtx_zError = (*aDimuonCandidate->userData<reco::Vertex>("PVwithmuons")).zError();
      } else {
        cout << "HiOniaAnalyzer::fillTreeDimuon: no PV for muon pair stored" << endl;
        return;
      }


      Reco_Dimuon_vtx_xpos.emplace_back(RefVtx.X());
      Reco_Dimuon_vtx_ypos.emplace_back(RefVtx.Y());
      Reco_Dimuon_vtx_zpos.emplace_back(RefVtx.Z());
      
      LorentzVector vMuon1 = muon1->p4();
      LorentzVector vMuon2 = muon2->p4();

      reco::Track iTrack_mupl, iTrack_mumi, mu1Trk, mu2Trk;
      if (aDimuonCandidate->hasUserData("muon1Track") && aDimuonCandidate->hasUserData("muon2Track")) {
        mu1Trk = *(aDimuonCandidate->userData<reco::Track>("muon1Track"));
        mu2Trk = *(aDimuonCandidate->userData<reco::Track>("muon2Track"));
      }


      if ((muon1->innerTrack()).isNull() || (muon2->innerTrack()).isNull()) {
        std::cout << "ERROR: 'iTrack_mupl' or 'iTrack_mumi' pointer in fillTreeDimuon is NULL ! Return now" << std::endl;
        return;
      }

      float muonPtDiff = 0;

      if (muon1->charge() > muon2->charge()) {
        Reco_Dimuon_muonPlusIndex[Reco_Dimuon_size] = IndexOfThisMuon(muon1->pt());  //needs the non-flipped muon momentum
        Reco_Dimuon_muonMinusIndex[Reco_Dimuon_size] = IndexOfThisMuon(muon2->pt());

        muonPtDiff = muon1->pt() - muon2->pt();

        iTrack_mupl = mu1Trk;
        iTrack_mumi = mu2Trk;
	  	  
	      Reco_Dimuon_mupl_4mom_pt.push_back(mu1Trk.pt());
        Reco_Dimuon_mupl_4mom_eta.push_back(mu1Trk.eta());
        Reco_Dimuon_mupl_4mom_phi.push_back(mu1Trk.phi());
        Reco_Dimuon_mupl_4mom_m.push_back(vMuon1.mass());


        Reco_Dimuon_mumi_4mom_pt.push_back(mu2Trk.pt());
        Reco_Dimuon_mumi_4mom_eta.push_back(mu2Trk.eta());
        Reco_Dimuon_mumi_4mom_phi.push_back(mu2Trk.phi());
        Reco_Dimuon_mumi_4mom_m.push_back(vMuon2.mass());

        if (_muonLessPrimaryVertex) {
          iTrack_mupl = *(muon1->innerTrack());
          iTrack_mumi = *(muon2->innerTrack());
        }

      } else {
        Reco_Dimuon_muonPlusIndex[Reco_Dimuon_size] = IndexOfThisMuon(muon2->pt());  //needs the non-flipped muon momentum
        Reco_Dimuon_muonMinusIndex[Reco_Dimuon_size] = IndexOfThisMuon(muon1->pt());

        muonPtDiff = muon2->pt() - muon1->pt();

        iTrack_mupl = mu2Trk;
        iTrack_mumi = mu1Trk;

        Reco_Dimuon_mumi_4mom_pt.push_back(mu1Trk.pt());
        Reco_Dimuon_mumi_4mom_eta.push_back(mu1Trk.eta());
        Reco_Dimuon_mumi_4mom_phi.push_back(mu1Trk.phi());
        Reco_Dimuon_mumi_4mom_m.push_back(vMuon1.mass());
	  	  
        Reco_Dimuon_mupl_4mom_pt.push_back(mu2Trk.pt());
        Reco_Dimuon_mupl_4mom_eta.push_back(mu2Trk.eta());
        Reco_Dimuon_mupl_4mom_phi.push_back(mu2Trk.phi());
        Reco_Dimuon_mupl_4mom_m.push_back(vMuon2.mass());
        
        if (_muonLessPrimaryVertex) {
          iTrack_mupl = *(muon2->innerTrack());
          iTrack_mumi = *(muon1->innerTrack());
        }
      }


      LorentzVector dimuonLV = aDimuonCandidate->p4();
      Reco_Dimuon_4mom_pt.push_back(dimuonLV.Pt());
      Reco_Dimuon_4mom_eta.push_back(dimuonLV.Eta());
      Reco_Dimuon_4mom_y.push_back(dimuonLV.Rapidity());
      Reco_Dimuon_4mom_phi.push_back(dimuonLV.Phi());
      Reco_Dimuon_4mom_m.push_back(dimuonLV.M());

      Reco_Dimuon_muonPtDiff.push_back(muonPtDiff);
      Reco_Dimuon_muonPtRelDiff.push_back(muonPtDiff / (muon1->pt() + muon2->pt()));

      if (_useBS) {
        if (aDimuonCandidate->hasUserFloat("ppdlBS")) {
          Reco_Dimuon_ctau[Reco_Dimuon_size] = 10.0 * aDimuonCandidate->userFloat("ppdlBS");
        } else {
          Reco_Dimuon_ctau[Reco_Dimuon_size] = -100;
          std::cout << "Warning: User Float ppdlBS was not found" << std::endl;
        }
        if (aDimuonCandidate->hasUserFloat("ppdlErrBS")) {
          Reco_Dimuon_ctauErr[Reco_Dimuon_size] = 10.0 * aDimuonCandidate->userFloat("ppdlErrBS");
        } else {
          Reco_Dimuon_ctauErr[Reco_Dimuon_size] = -100;
          std::cout << "Warning: User Float ppdlErrBS was not found" << std::endl;
        }
        if (aDimuonCandidate->hasUserFloat("ppdlBS3D")) {
          Reco_Dimuon_ctau3D[Reco_Dimuon_size] = 10.0 * aDimuonCandidate->userFloat("ppdlBS3D");
        } else {
          Reco_Dimuon_ctau3D[Reco_Dimuon_size] = -100;
          std::cout << "Warning: User Float ppdlBS3D was not found" << std::endl;
        }
        if (aDimuonCandidate->hasUserFloat("ppdlErrBS3D")) {
          Reco_Dimuon_ctauErr3D[Reco_Dimuon_size] = 10.0 * aDimuonCandidate->userFloat("ppdlErrBS3D");
        } else {
          Reco_Dimuon_ctauErr3D[Reco_Dimuon_size] = -100;
          std::cout << "Warning: User Float ppdlErrBS3D was not found" << std::endl;
        }
      } else {
        if (aDimuonCandidate->hasUserFloat("ppdlPV")) {
          Reco_Dimuon_ctau[Reco_Dimuon_size] = 10.0 * aDimuonCandidate->userFloat("ppdlPV");
        } else {
          Reco_Dimuon_ctau[Reco_Dimuon_size] = -100;
          std::cout << "Warning: User Float ppdlPV was not found" << std::endl;
        }
        if (aDimuonCandidate->hasUserFloat("ppdlErrPV")) {
          Reco_Dimuon_ctauErr[Reco_Dimuon_size] = 10.0 * aDimuonCandidate->userFloat("ppdlErrPV");
        } else {
          Reco_Dimuon_ctauErr[Reco_Dimuon_size] = -100;
          std::cout << "Warning: User Float ppdlErrPV was not found" << std::endl;
        }
        if (aDimuonCandidate->hasUserFloat("ppdlPV3D")) {
          Reco_Dimuon_ctau3D[Reco_Dimuon_size] = 10.0 * aDimuonCandidate->userFloat("ppdlPV3D");
        } else {
          Reco_Dimuon_ctau3D[Reco_Dimuon_size] = -100;
          std::cout << "Warning: User Float ppdlPV3D was not found" << std::endl;
        }
        if (aDimuonCandidate->hasUserFloat("ppdlErrPV3D")) {
          Reco_Dimuon_ctauErr3D[Reco_Dimuon_size] = 10.0 * aDimuonCandidate->userFloat("ppdlErrPV3D");
        } else {
          Reco_Dimuon_ctau3D[Reco_Dimuon_size] = -100;
          std::cout << "Warning: User Float ppdlErrPV3D was not found" << std::endl;
        }
        if (aDimuonCandidate->hasUserFloat("cosAlpha")) {
          Reco_Dimuon_cosAlpha[Reco_Dimuon_size] = aDimuonCandidate->userFloat("cosAlpha");
        } else {
          Reco_Dimuon_cosAlpha[Reco_Dimuon_size] = -10;
          std::cout << "Warning: User Float cosAlpha was not found" << std::endl;
        }
        if (aDimuonCandidate->hasUserFloat("cosAlpha3D")) {
          Reco_Dimuon_cosAlpha3D[Reco_Dimuon_size] = aDimuonCandidate->userFloat("cosAlpha3D");
        } else {
          Reco_Dimuon_cosAlpha3D[Reco_Dimuon_size] = -10;
          std::cout << "Warning: User Float cosAlpha3D was not found" << std::endl;
        }
      }
      if (aDimuonCandidate->hasUserFloat("vProb")) {
        Reco_Dimuon_VtxProb[Reco_Dimuon_size] = aDimuonCandidate->userFloat("vProb");
      } else {
        Reco_Dimuon_VtxProb[Reco_Dimuon_size] = -1;
        std::cout << "Warning: User Float vProb was not found" << std::endl;
      }
      if (aDimuonCandidate->hasUserFloat("DCA")) {
        Reco_Dimuon_dca[Reco_Dimuon_size] = aDimuonCandidate->userFloat("DCA");
      } else {
        Reco_Dimuon_dca[Reco_Dimuon_size] = -10;
        std::cout << "Warning: User Float DCA was not found" << std::endl;
      }
      if (aDimuonCandidate->hasUserFloat("MassErr")) {
        Reco_Dimuon_MassErr[Reco_Dimuon_size] = aDimuonCandidate->userFloat("MassErr");
      } else {
        Reco_Dimuon_MassErr[Reco_Dimuon_size] = -10;
        std::cout << "Warning: User Float MassErr was not found" << std::endl;
      }

    }
  } else {
    std::cout << "ERROR: 'aDimuonCand' pointer in fillTreeDimuon is NULL ! Return now" << std::endl;
    return;
  }

  Reco_Dimuon_size++;
  return;
};

void HiOniaAnalyzer::fillRecoDimuon(int count, std::string trigName, std::string centName) {
  pat::CompositeCandidate* aDimuonCand = _thePassedCands.at(count)->clone();

  if (aDimuonCand == nullptr) {
    std::cout << "ERROR: 'aDimuonCand' pointer in fillTreeDimuon is NULL ! Return now" << std::endl;
    return;
  }
  aDimuonCand->addUserInt("centBin", centBin);
  const pat::Muon* muon1 = dynamic_cast<const pat::Muon*>(aDimuonCand->daughter("muon1"));
  const pat::Muon* muon2 = dynamic_cast<const pat::Muon*>(aDimuonCand->daughter("muon2"));

  if (muon1 == nullptr || muon2 == nullptr) {
    std::cout << "ERROR: 'muon1' or 'muon2' pointer in fillTreeDimuon is NULL ! Return now" << std::endl;
    return;
  }
  int iSign = muon1->charge() + muon2->charge();
  if (iSign != 0) {
    (iSign == 2) ? (iSign = 1) : (iSign = 2);
  }

  std::string theLabel = trigName + "_" + centName + "_" + theSign.at(iSign);


  delete aDimuonCand;
  return;
};


void HiOniaAnalyzer::checkTriggers(const pat::CompositeCandidate* aDimuonCand) {
  if (aDimuonCand == nullptr) {
    std::cout << "ERROR: 'aDimuonCand' pointer in checkTriggers is NULL ! Return now" << std::endl;
    return;
  }

  const pat::Muon* muon1 = dynamic_cast<const pat::Muon*>(aDimuonCand->daughter("muon1"));
  const pat::Muon* muon2 = dynamic_cast<const pat::Muon*>(aDimuonCand->daughter("muon2"));

  if (muon1 == nullptr || muon2 == nullptr) {
    std::cout << "ERROR: 'muon1' or 'muon2' pointer in checkTriggers is NULL ! Return now" << std::endl;
    return;
  }

  // Trigger passed
  for (unsigned int iTr = 1; iTr < NTRIGGERS; ++iTr) {
    const auto& lastFilter = filterNameMap.at(theTriggerNames[iTr]);
    const auto& mu1HLTMatchesFilter = muon1->triggerObjectMatchesByFilter(lastFilter);
    const auto& mu2HLTMatchesFilter = muon2->triggerObjectMatchesByFilter(lastFilter);

    bool pass1 = !mu1HLTMatchesFilter.empty();
    bool pass2 = !mu2HLTMatchesFilter.empty();

    if (iTr > NTRIGGERS_DBL) {  // single triggers here
      isTriggerMatched[iTr] = pass1 || pass2;
    } else {  // double triggers here
      isTriggerMatched[iTr] = pass1 && pass2;
    }
  }

  for (unsigned int iTr = 1; iTr < NTRIGGERS; ++iTr) {
    if (isTriggerMatched[iTr]) {
      // since we have bins for event info, let's try to fill here the trigger info for each pair
      // also if there are several pairs matched to the same kind of trigger
      hStats->Fill(iTr + NTRIGGERS);  // pair info
    }
  }
  return;
};

void HiOniaAnalyzer::InitEvent() {
  for (unsigned int iTr = 1; iTr < NTRIGGERS; ++iTr) {
    alreadyFilled[iTr] = false;
  }
  HLTriggers = 0;
  nEP = 0;

  _thePassedCats.clear();
  _thePassedCands.clear();

  Reco_Dimuon_size = 0;
  Reco_Muon_size = 0;

  Reco_Dimuon_4mom_pt.clear();
  Reco_Dimuon_4mom_eta.clear();
  Reco_Dimuon_4mom_y.clear();
  Reco_Dimuon_4mom_phi.clear();
  Reco_Dimuon_4mom_m.clear();
  Reco_Dimuon_muonPtDiff.clear();
  Reco_Dimuon_muonPtRelDiff.clear();

  Reco_Dimuon_mupl_4mom_pt.clear();
  Reco_Dimuon_mupl_4mom_eta.clear();
  Reco_Dimuon_mupl_4mom_phi.clear();
  Reco_Dimuon_mupl_4mom_m.clear();

  Reco_Dimuon_mumi_4mom_pt.clear();
  Reco_Dimuon_mumi_4mom_eta.clear();
  Reco_Dimuon_mumi_4mom_phi.clear();
  Reco_Dimuon_mumi_4mom_m.clear();
  Reco_Dimuon_vtx_xpos.clear();
  Reco_Dimuon_vtx_ypos.clear();
  Reco_Dimuon_vtx_zpos.clear();
  
  Reco_Muon_4mom.clear();
  Reco_Muon_4mom_pt.clear();
  Reco_Muon_ptErr_inner.clear();

  Reco_Muon_4mom_eta.clear();
  Reco_Muon_4mom_phi.clear();
  Reco_Muon_4mom_m.clear();
  Reco_Muon_L1_4mom_eta.clear();
  Reco_Muon_L1_4mom_phi.clear();


  if (_isMC) {
    Gen_Dimuon_4mom.clear();
    Gen_Dimuon_4mom_pt.clear();
    Gen_Dimuon_4mom_eta.clear();
    Gen_Dimuon_4mom_y.clear();
    Gen_Dimuon_4mom_phi.clear();
    Gen_Dimuon_4mom_m.clear();
    Gen_Dimuon_muonPtDiff.clear();
    Gen_Dimuon_muonPtRelDiff.clear();

    Gen_Muon_4mom.clear();
    Gen_Muon_4mom_pt.clear();
    Gen_Muon_4mom_eta.clear();
    Gen_Muon_4mom_phi.clear();
    Gen_Muon_4mom_m.clear();
    Gen_Dimuon_size = 0;
    Gen_Muon_size = 0;

    Gen_weight = -1.;
    Gen_pthat = -1.;

    mapGenMuonMomToIndex_.clear();
  }

  mapMuonMomToIndex_.clear();

  if (_addMuonIsolation){
    
    Reco_Muon_passesPFIsoLoose.clear();
    Reco_Muon_passesPFIsoMedium.clear();
    Reco_Muon_passesPFIsoTight.clear();
    Reco_Muon_passesPFIsoVeryTight.clear();

    Reco_Muon_isoTrackSumPt.clear();
    Reco_Muon_passesMultiIsoMedium.clear();

    if (_isHI){
      Reco_Muon_HIMVAIso.clear();
      for (auto& w : Reco_Muon_HIMVAIsoWPs)
        w.second.clear();
    }
    
  }

  for (std::map<std::string, int>::iterator clearIt = mapTriggerNameToIntFired_.begin();
       clearIt != mapTriggerNameToIntFired_.end();
       clearIt++) {
    clearIt->second = 0;
  }
  for (std::map<std::string, int>::iterator clearIt = mapTriggerNameToPrescaleFac_.begin();
       clearIt != mapTriggerNameToPrescaleFac_.end();
       clearIt++) {
    clearIt->second = -1;
  }

  return;
};


void HiOniaAnalyzer::fillRecoMuons(int iCent) {

  if (collMuon.isValid()) {
    for (vector<pat::Muon>::const_iterator it = collMuon->begin(); it != collMuon->end(); ++it) {
      const pat::Muon* muon = &(*it);
      if (muon == nullptr) {
        std::cout << "ERROR: 'muon' pointer in fillRecoMuons is NULL ! Return now" << std::endl;
        return;
      }

      std::string theLabel = theTriggerNames.at(0) + "_" + theCentralities.at(iCent);


      muType = -99;
      if (_muonSel == (std::string)("Glb") && selGlobalMuon(muon))
        muType = Glb;
      if (_muonSel == (std::string)("Tight") && selTightMuon(muon))
        muType = Tight;
      if (_muonSel == (std::string)("GlbTrk") && selGlobalMuon(muon))
        muType = GlbTrk;
      if (_muonSel == (std::string)("Trk") && selTrackerMuon(muon))
        muType = Trk;
      if (_muonSel == (std::string)("GlbOrTrk") && selGlobalOrTrackerMuon(muon))
        muType = GlbOrTrk;
      if (_muonSel == (std::string)("All"))
        muType = All;

      ULong64_t trigBits = 0;
      for (unsigned int iTr = 1; iTr < NTRIGGERS; ++iTr) {
        const pat::TriggerObjectStandAloneCollection muHLTMatchesFilter =
              muon->triggerObjectMatchesByFilter(filterNameMap.at(theTriggerNames[iTr]));

        // apparently matching by path gives false positives so we use matching by filter for all triggers for which we know the filter name
        if (!muHLTMatchesFilter.empty()) {
          std::string theLabel = theTriggerNames.at(iTr) + "_" + theCentralities.at(iCent);

          trigBits += pow(2, iTr - 1);
            
        }
      }
      if (_fillTree)
        this->fillTreeMuon(muon, muType, trigBits);
      
    }
  }

  return;
};

void HiOniaAnalyzer::InitTree() {

  //myTree = new TTree("myTree","My TTree of dimuons");
  myTree = fs->make<TTree>("DimuonTree", "My TTree of dimuons");

  myTree->Branch("eventNb", &eventNb, "eventNb/i");
  if(_genOnly) goto genOnly2;
  if (!_isMC) {
    myTree->Branch("runNb", &runNb, "runNb/i");
    myTree->Branch("LS", &lumiSection, "LS/i");
  }
  myTree->Branch("zVtx", &zVtx, "zVtx/F");
  myTree->Branch("nPV", &nPV, "nPV/S");
  if (_isHI || _isPA) {
    myTree->Branch("Centrality", &centBin, "Centrality/I");
    //myTree->Branch("Npix", &Npix, "Npix/S");
    myTree->Branch("NpixelTracks", &NpixelTracks, "NpixelTracks/S");
  }

  //myTree->Branch("nTrig", &nTrig, "nTrig/I");
  myTree->Branch("trigPrescale", trigPrescale, Form("trigPrescale[%d]/I", nTrig));
  myTree->Branch("HLTriggers", &HLTriggers, "HLTriggers/l");

  if ((_isHI || _isPA) && _SumETvariables) {
    myTree->Branch("SumET_HF", &SumET_HF, "SumET_HF/F");
    myTree->Branch("SumET_HFplus", &SumET_HFplus, "SumET_HFplus/F");
    myTree->Branch("SumET_HFminus", &SumET_HFminus, "SumET_HFminus/F");
    myTree->Branch("SumET_HFplusEta4", &SumET_HFplusEta4, "SumET_HFplusEta4/F");
    myTree->Branch("SumET_HFminusEta4", &SumET_HFminusEta4, "SumET_HFminusEta4/F");
    myTree->Branch("SumET_ET", &SumET_ET, "SumET_ET/F");
    myTree->Branch("SumET_EE", &SumET_EE, "SumET_EE/F");
    myTree->Branch("SumET_EB", &SumET_EB, "SumET_EB/F");
    //myTree->Branch("SumET_EEplus", &SumET_EEplus, "SumET_EEplus/F");
    //myTree->Branch("SumET_EEminus", &SumET_EEminus, "SumET_EEminus/F");
    //myTree->Branch("SumET_ZDC", &SumET_ZDC, "SumET_ZDC/F");
    //myTree->Branch("SumET_ZDCplus", &SumET_ZDCplus, "SumET_ZDCplus/F");
    //myTree->Branch("SumET_ZDCminus", &SumET_ZDCminus, "SumET_ZDCminus/F");
  }

  if ((_isHI || _isPA) && _useEvtPlane) {
    myTree->Branch("nEP", &nEP, "nEP/I");
    myTree->Branch("rpAng", &rpAng, "rpAng[nEP]/F");
    myTree->Branch("rpSin", &rpSin, "rpSin[nEP]/F");
    myTree->Branch("rpCos", &rpCos, "rpCos[nEP]/F");
    myTree->Branch("rpAng_origin", &rpAng_origin, "rpAng_origin[nEP]/F");
    myTree->Branch("rpSin_origin", &rpSin_origin, "rpSin_origin[nEP]/F");
    myTree->Branch("rpCos_origin", &rpCos_origin, "rpCos_origin[nEP]/F");
  }


  myTree->Branch("Reco_Dimuon_size", &Reco_Dimuon_size, "Reco_Dimuon_size/S");
  //myTree->Branch("Reco_Dimuon_type", Reco_Dimuon_type, "Reco_Dimuon_type[Reco_Dimuon_size]/S");
  myTree->Branch("Reco_Dimuon_sign", Reco_Dimuon_sign, "Reco_Dimuon_sign[Reco_Dimuon_size]/S");

  myTree->Branch("Reco_Dimuon_pt", &Reco_Dimuon_4mom_pt, 32000, 0);
  myTree->Branch("Reco_Dimuon_eta", &Reco_Dimuon_4mom_eta, 32000, 0);
  myTree->Branch("Reco_Dimuon_rapidity", &Reco_Dimuon_4mom_y, 32000, 0);
  myTree->Branch("Reco_Dimuon_phi", &Reco_Dimuon_4mom_phi, 32000, 0);
  myTree->Branch("Reco_Dimuon_invMass", &Reco_Dimuon_4mom_m, 32000, 0);
  myTree->Branch("Reco_Dimuon_muonPtDiff", &Reco_Dimuon_muonPtDiff, 32000, 0);
  myTree->Branch("Reco_Dimuon_muonPtRelDiff", &Reco_Dimuon_muonPtRelDiff, 32000, 0);
    
  myTree->Branch("Reco_Dimuon_muonPlusIndex", Reco_Dimuon_muonPlusIndex, "Reco_Dimuon_muonPlusIndex[Reco_Dimuon_size]/S");
  myTree->Branch("Reco_Dimuon_muonMinusIndex", Reco_Dimuon_muonMinusIndex, "Reco_Dimuon_muonMinusIndex[Reco_Dimuon_size]/S");

  myTree->Branch("Reco_Dimuon_trig", Reco_Dimuon_trig, "Reco_Dimuon_trig[Reco_Dimuon_size]/l");
  myTree->Branch("Reco_Dimuon_ctau", Reco_Dimuon_ctau, "Reco_Dimuon_ctau[Reco_Dimuon_size]/F");
  myTree->Branch("Reco_Dimuon_ctauErr", Reco_Dimuon_ctauErr, "Reco_Dimuon_ctauErr[Reco_Dimuon_size]/F");
  myTree->Branch("Reco_Dimuon_cosAlpha", Reco_Dimuon_cosAlpha, "Reco_Dimuon_cosAlpha[Reco_Dimuon_size]/F");
  myTree->Branch("Reco_Dimuon_ctau3D", Reco_Dimuon_ctau3D, "Reco_Dimuon_ctau3D[Reco_Dimuon_size]/F");
  myTree->Branch("Reco_Dimuon_ctauErr3D", Reco_Dimuon_ctauErr3D, "Reco_Dimuon_ctauErr3D[Reco_Dimuon_size]/F");
  myTree->Branch("Reco_Dimuon_cosAlpha3D", Reco_Dimuon_cosAlpha3D, "Reco_Dimuon_cosAlpha3D[Reco_Dimuon_size]/F");

  if (_isMC) {
    myTree->Branch("Reco_Dimuon_whichGen", Reco_Dimuon_whichGen, "Reco_Dimuon_whichGen[Reco_Dimuon_size]/S");
  }
    
  myTree->Branch("Reco_Dimuon_vtxProb", Reco_Dimuon_VtxProb, "Reco_Dimuon_vtxProb[Reco_Dimuon_size]/F");
  myTree->Branch("Reco_Dimuon_dca", Reco_Dimuon_dca, "Reco_Dimuon_dca[Reco_Dimuon_size]/F");
  myTree->Branch("Reco_Dimuon_invMassErr", Reco_Dimuon_MassErr, "Reco_Dimuon_MassErr[Reco_Dimuon_size]/F");

  myTree->Branch("Reco_Dimuon_vtx_xpos", &Reco_Dimuon_vtx_xpos, 32000, 0);
  myTree->Branch("Reco_Dimuon_vtx_ypos", &Reco_Dimuon_vtx_ypos, 32000, 0);
  myTree->Branch("Reco_Dimuon_vtx_zpos", &Reco_Dimuon_vtx_zpos, 32000, 0);
      
  if ( _muonLessPrimaryVertex) {
    myTree->Branch("Reco_Dimuon_mupl_dxy_muonlessVtx", Reco_Dimuon_mupl_dxy, "Reco_Dimuon_mupl_dxy_muonlessVtx[Reco_Dimuon_size]/F");
    myTree->Branch("Reco_Dimuon_mumi_dxy_muonlessVtx", Reco_Dimuon_mumi_dxy, "Reco_Dimuon_mumi_dxy_muonlessVtx[Reco_Dimuon_size]/F");
    myTree->Branch("Reco_Dimuon_mupl_dz_muonlessVtx", Reco_Dimuon_mupl_dz, "Reco_Dimuon_mupl_dz_muonlessVtx[Reco_Dimuon_size]/F");
    myTree->Branch("Reco_Dimuon_mumi_dz_muonlessVtx", Reco_Dimuon_mumi_dz, "Reco_Dimuon_mumi_dz_muonlessVtx[Reco_Dimuon_size]/F");
  }


  myTree->Branch("Reco_Muon_size", &Reco_Muon_size, "Reco_Muon_size/S");
  //myTree->Branch("Reco_Muon_type", Reco_Muon_type, "Reco_Muon_type[Reco_Muon_size]/S");
  if (_isMC) {
    myTree->Branch("Reco_Muon_whichGen", Reco_Muon_whichGen, "Reco_Muon_whichGen[Reco_Muon_size]/S");
  }
  //myTree->Branch("Reco_Muon_SelectionType", Reco_Muon_SelectionType, "Reco_Muon_SelectionType[Reco_Muon_size]/I");
  myTree->Branch("Reco_Muon_pt", &Reco_Muon_4mom_pt, 32000, 0);
  myTree->Branch("Reco_Muon_ptErrTrk", &Reco_Muon_ptErr_inner, 32000, 0);

  myTree->Branch("Reco_Muon_eta", &Reco_Muon_4mom_eta, 32000, 0);
  myTree->Branch("Reco_Muon_phi", &Reco_Muon_4mom_phi, 32000, 0);
  myTree->Branch("Reco_Muon_mass", &Reco_Muon_4mom_m, 32000, 0);
  myTree->Branch("Reco_Muon_etaL1", &Reco_Muon_L1_4mom_eta, 32000, 0);
  myTree->Branch("Reco_Muon_phiL1", &Reco_Muon_L1_4mom_phi, 32000, 0);
  
  myTree->Branch("Reco_Muon_trig", Reco_Muon_trig, "Reco_Muon_trig[Reco_Muon_size]/l");

  
  myTree->Branch("Reco_Muon_isPF", Reco_Muon_isPF, "Reco_Muon_isPF[Reco_Muon_size]/O");
  myTree->Branch("Reco_Muon_isTracker", Reco_Muon_isTracker, "Reco_Muon_isTracker[Reco_Muon_size]/O");
  myTree->Branch("Reco_Muon_isGlobal", Reco_Muon_isGlobal, "Reco_Muon_isGlobal[Reco_Muon_size]/O");
  myTree->Branch("Reco_Muon_isSoftCutBased", Reco_Muon_isSoftCutBased, "Reco_Muon_isSoftCutBased[Reco_Muon_size]/O");
  myTree->Branch("Reco_Muon_isHybridSoft", Reco_Muon_isHybridSoft, "Reco_Muon_isHybridSoft[Reco_Muon_size]/O");
  myTree->Branch("Reco_Muon_isLooseCutBased", Reco_Muon_isLooseCutBased, "Reco_Muon_isLooseCutBased[Reco_Muon_size]/O");
  myTree->Branch("Reco_Muon_isMediumCutBased", Reco_Muon_isMediumCutBased, "Reco_Muon_isMediumCutBased[Reco_Muon_size]/O");
  myTree->Branch("Reco_Muon_isTightCutBased", Reco_Muon_isTightCutBased, "Reco_Muon_isTightCutBased[Reco_Muon_size]/O");

  myTree->Branch("Reco_Muon_softMVAValue", Reco_Muon_softMVAValue, "Reco_Muon_softMVAValue[Reco_Muon_size]/F");
  myTree->Branch("Reco_Muon_muonMVAValue", Reco_Muon_muonMVAValue, "Reco_Muon_muonMVAValue[Reco_Muon_size]/F");

  // muon isolation variables
  if (_addMuonIsolation){
    myTree->Branch("Reco_Muon_passesPFIsoLoose", &Reco_Muon_passesPFIsoLoose);
    myTree->Branch("Reco_Muon_passesPFIsoMedium", &Reco_Muon_passesPFIsoMedium);
    myTree->Branch("Reco_Muon_passesPFIsoTight", &Reco_Muon_passesPFIsoTight);
    myTree->Branch("Reco_Muon_passesPFIsoVeryTight", &Reco_Muon_passesPFIsoVeryTight);

    myTree->Branch("Reco_Muon_isoTrackSumPt", &Reco_Muon_isoTrackSumPt);

    myTree->Branch("Reco_Muon_passesMultiIsoMedium", &Reco_Muon_passesMultiIsoMedium);
    

    if (_isHI){
      myTree->Branch("Reco_Muon_HIMVAIso", &Reco_Muon_HIMVAIso);
      for (auto& w : Reco_Muon_HIMVAIsoWPs)
        myTree->Branch(("Reco_Muon_HIMVAIso"+w.first).c_str(), &(w.second));
    }
  }
  
  
  //myTree->Branch("Reco_Muon_InTightAcc", Reco_Muon_InTightAcc, "Reco_Muon_InTightAcc[Reco_Muon_size]/O");
  //myTree->Branch("Reco_Muon_InLooseAcc", Reco_Muon_InLooseAcc, "Reco_Muon_InLooseAcc[Reco_Muon_size]/O");
  //myTree->Branch("Reco_Muon_isHighPurity", Reco_Muon_highPurity, "Reco_Muon_highPurity[Reco_Muon_size]/O");
  //myTree->Branch("Reco_Muon_TMOneStaTight", Reco_Muon_TMOneStaTight, "Reco_Muon_TMOneStaTight[Reco_Muon_size]/O");
  // myTree->Branch("Reco_Muon_TrkMuArb", Reco_Muon_TrkMuArb,   "Reco_Muon_TrkMuArb[Reco_Muon_size]/O");

    //myTree->Branch("Reco_Muon_candType", Reco_Muon_candType, "Reco_Muon_candType[Reco_Muon_size]/S");
    //myTree->Branch("Reco_Muon_nPixValHits", Reco_Muon_nPixValHits, "Reco_Muon_nPixValHits[Reco_Muon_size]/I");
    //myTree->Branch("Reco_Muon_nMuValHits", Reco_Muon_nMuValHits, "Reco_Muon_nMuValHits[Reco_Muon_size]/I");
    //myTree->Branch("Reco_Muon_nTrkHits", Reco_Muon_nTrkHits, "Reco_Muon_nTrkHits[Reco_Muon_size]/I");
    //myTree->Branch("Reco_Muon_segmentComp", Reco_Muon_segmentComp, "Reco_Muon_segmentComp[Reco_Muon_size]/F");
    //myTree->Branch("Reco_Muon_kink", Reco_Muon_kink, "Reco_Muon_kink[Reco_Muon_size]/F");
    //myTree->Branch("Reco_Muon_localChi2", Reco_Muon_localChi2, "Reco_Muon_localChi2[Reco_Muon_size]/F");
    //myTree->Branch("Reco_Muon_validFraction", Reco_Muon_validFraction, "Reco_Muon_validFraction[Reco_Muon_size]/F");
  //myTree->Branch("Reco_Muon_normChi2_bestTracker", Reco_Muon_normChi2_bestTracker, "Reco_Muon_normChi2_bestTracker[Reco_Muon_size]/F");
  //myTree->Branch("Reco_Muon_normChi2_inner", Reco_Muon_normChi2_inner, "Reco_Muon_normChi2_inner[Reco_Muon_size]/F");
    //myTree->Branch("Reco_Muon_normChi2_global", Reco_Muon_normChi2_global, "Reco_Muon_normChi2_global[Reco_Muon_size]/F");
  //myTree->Branch("Reco_Muon_nPixWMea", Reco_Muon_nPixWMea, "Reco_Muon_nPixWMea[Reco_Muon_size]/I");
  //myTree->Branch("Reco_Muon_nTrkWMea", Reco_Muon_nTrkWMea, "Reco_Muon_nTrkWMea[Reco_Muon_size]/I");
  //myTree->Branch("Reco_Muon_nStationsMatched", Reco_Muon_nStationsMatched, "Reco_Muon_nStationsMatched[Reco_Muon_size]/I");
    //myTree->Branch("Reco_Muon_dxy", Reco_Muon_dxy, "Reco_Muon_dxy[Reco_Muon_size]/F");
    //myTree->Branch("Reco_Muon_dxyErr", Reco_Muon_dxyErr, "Reco_Muon_dxyErr[Reco_Muon_size]/F");
    //myTree->Branch("Reco_Muon_dz", Reco_Muon_dz, "Reco_Muon_dz[Reco_Muon_size]/F");
    //myTree->Branch("Reco_Muon_dzErr", Reco_Muon_dzErr, "Reco_Muon_dzErr[Reco_Muon_size]/F");
    // myTree->Branch("Reco_Muon_pt_inner",Reco_Muon_pt_inner, "Reco_Muon_pt_inner[Reco_Muon_size]/F");
    // myTree->Branch("Reco_Muon_pt_global",Reco_Muon_pt_global, "Reco_Muon_pt_global[Reco_Muon_size]/F");
    // myTree->Branch("Reco_Muon_ptErr_global",Reco_Muon_ptErr_global, "Reco_Muon_ptErr_global[Reco_Muon_size]/F");
  


genOnly2: 
  if (_isMC) {
    if (_genealogyInfo) {
      //myTree->Branch("Reco_Muon_simExtType", Reco_Muon_simExtType, "Reco_Muon_simExtType[Reco_Muon_size]/I");
    }
    
  myTree->Branch("Gen_weight", &Gen_weight, "Gen_weight/F");
  myTree->Branch("Gen_pthat", &Gen_pthat, "Gen_pthat/F");

  myTree->Branch("Gen_Dimuon_size", &Gen_Dimuon_size, "Gen_Dimuon_size/S");
    //myTree->Branch("Gen_Dimuon_type",      Gen_Dimuon_type,    "Gen_Dimuon_type[Gen_Dimuon_size]/S");
	myTree->Branch("Gen_Dimuon_pt", &Gen_Dimuon_4mom_pt, 32000, 0);
	myTree->Branch("Gen_Dimuon_eta", &Gen_Dimuon_4mom_eta, 32000, 0);
	myTree->Branch("Gen_Dimuon_rapidity", &Gen_Dimuon_4mom_y, 32000, 0);
	myTree->Branch("Gen_Dimuon_phi", &Gen_Dimuon_4mom_phi, 32000, 0);
	myTree->Branch("Gen_Dimuon_mass", &Gen_Dimuon_4mom_m, 32000, 0);

      
    myTree->Branch("Gen_Dimuon_ctau", Gen_Dimuon_ctau, "Gen_Dimuon_ctau[Gen_Dimuon_size]/F");
    myTree->Branch("Gen_Dimuon_ctau3D", Gen_Dimuon_ctau3D, "Gen_Dimuon_ctau3D[Gen_Dimuon_size]/F");
    myTree->Branch("Gen_Dimuon_muonPlusIndex", Gen_Dimuon_muonPlusIndex, "Gen_Dimuon_muonPlusIndex[Gen_Dimuon_size]/S");
    myTree->Branch("Gen_Dimuon_muonMinusIndex", Gen_Dimuon_muonMinusIndex, "Gen_Dimuon_muonMinusIndex[Gen_Dimuon_size]/S");
    myTree->Branch("Gen_Dimuon_muonPtDiff", &Gen_Dimuon_muonPtDiff, 32000, 0);
    myTree->Branch("Gen_Dimuon_muonPtRelDiff", &Gen_Dimuon_muonPtRelDiff, 32000, 0);


    myTree->Branch("Gen_Dimuon_whichRec", Gen_Dimuon_whichRec, "Gen_Dimuon_whichRec[Gen_Dimuon_size]/S");
    if (_genealogyInfo) {
      myTree->Branch("Gen_Dimuon_momId", Gen_Dimuon_momId, "Gen_Dimuon_momId[Gen_Dimuon_size]/I");
    }

    myTree->Branch("Gen_Muon_size", &Gen_Muon_size, "Gen_Muon_size/S");
    //myTree->Branch("Gen_Muon_type",   Gen_Muon_type,   "Gen_Muon_type[Gen_Muon_size]/S");
    myTree->Branch("Gen_Muon_charge", Gen_Muon_charge, "Gen_Muon_charge[Gen_Muon_size]/S");
    
    myTree->Branch("Gen_Muon_pt", &Gen_Muon_4mom_pt, 32000, 0);
    myTree->Branch("Gen_Muon_eta", &Gen_Muon_4mom_eta, 32000, 0);
    myTree->Branch("Gen_Muon_phi", &Gen_Muon_4mom_phi, 32000, 0);
    myTree->Branch("Gen_Muon_mass", &Gen_Muon_4mom_m, 32000, 0);
    
    myTree->Branch("Gen_Muon_whichRec", Gen_Muon_whichRec, "Gen_Muon_whichRec[Gen_Muon_size]/S");
  }

  return;
};

// ------------ method called once each job just before starting event loop  ------------
void HiOniaAnalyzer::beginJob() {
  InitTree();

  // book histos

  hStats = fs->make<TH1F>("hStats", "hStats;;Number of Events", 2 * NTRIGGERS + 1, 0, 2 * NTRIGGERS + 1);
  hStats->GetXaxis()->SetBinLabel(1, "All");
  for (int i = 2; i < (int)theTriggerNames.size() + 1; ++i) {
    hStats->GetXaxis()->SetBinLabel(i, theTriggerNames.at(i - 1).c_str());              // event info
    hStats->GetXaxis()->SetBinLabel(i + NTRIGGERS, theTriggerNames.at(i - 1).c_str());  // muon pair info
  }
  hStats->Sumw2();

  hCent = fs->make<TH1F>("hCent", "hCent;centrality bin;Number of Events", 200, 0, 200);
  hCent->Sumw2();

  hPileUp = fs->make<TH1F>("hPileUp", "Number of Primary Vertices;n_{PV};counts", 50, 0, 50);
  hPileUp->Sumw2();

  hZVtx = fs->make<TH1F>("hZVtx", "Primary z-vertex distribution;z_{vtx} [cm];counts", 120, -30, 30);
  hZVtx->Sumw2();

  return;
};

void HiOniaAnalyzer::beginRun(const edm::Run& iRun, const edm::EventSetup& iSetup) {
  //init HLTConfigProvider

  EDConsumerBase::Labels labelTriggerResults;
  EDConsumerBase::labelsForToken(_tagTriggerResultsToken, labelTriggerResults);
  const std::string pro = labelTriggerResults.process;

  bool changed = true;
  hltConfigInit = hltConfig.init(iRun, iSetup, pro, changed);

  changed = true;
  hltPrescaleInit = hltPrescaleProvider.init(iRun, iSetup, pro, changed);

  //extract trigger path names
  for (const auto& pathLabel : theTriggerNames)
    triggerNameMap[pathLabel] = "";

  for (const auto& hltPath : hltConfig.triggerNames())
    if (hltPath.rfind("HLT_", 0) == 0)
      for (const auto& pathLabel : theTriggerNames)
        if (hltPath != "NoString" && TString(hltPath).Contains(TRegexp(TString(pathLabel)))) {
          triggerNameMap.at(pathLabel) = hltPath;
          break;
        }

  //extract last filter names
  for (const auto& p : triggerNameMap) {
    filterNameMap[p.first] = "";
    if (p.second.empty())
      continue;
    const auto& m = hltConfig.moduleLabels(hltConfig.triggerIndex(p.second));
    for (int j = m.size() - 1; j >= 0; j--)
      if ( (m[j].rfind("hltL", 0) == 0 && m[j].rfind("Filtered") != std::string::npos) || (m[j].rfind("hltL1s", 0) == 0) ) {
        filterNameMap.at(p.first) = m[j];
        break;
      }
  }
  return;
};

// ------------ method called once each job just after ending the event loop  ------------
void HiOniaAnalyzer::endJob() {
  std::cout << "Total number of events = " << nEvents << std::endl;
  //std::cout << "Total number of passed candidates = " << passedCandidates << std::endl;
  return;
};

//define this as a plug-in
DEFINE_FWK_MODULE(HiOniaAnalyzer);