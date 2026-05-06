#ifndef __HIONIA__
#define __HIONIA__


// system include files
#include <memory>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <utility>

#include <TTree.h>
#include <TRegexp.h>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"
#include "DataFormats/Candidate/interface/CompositeCandidate.h"
#include <DataFormats/RecoCandidate/interface/RecoChargedCandidate.h>
#include "DataFormats/Common/interface/TriggerResults.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "DataFormats/BeamSpot/interface/BeamSpot.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"

#include "HLTrigger/HLTcore/interface/HLTPrescaleProvider.h"
#include "FWCore/Framework/interface/ConsumesCollector.h"
#include "HLTrigger/HLTcore/interface/HLTConfigProvider.h"

#include "DataFormats/HeavyIonEvent/interface/Centrality.h"
#include "DataFormats/HeavyIonEvent/interface/EvtPlane.h"
#include "SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h"

#include "DataFormats/Math/interface/LorentzVector.h"
#include "DataFormats/Math/interface/deltaR.h"

#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"

//
// class declaration
//
using LorentzVector = math::XYZTLorentzVector;

class HiOniaAnalyzer : public edm::one::EDAnalyzer<edm::one::SharedResources, edm::one::WatchRuns> {
public:
  explicit HiOniaAnalyzer(const edm::ParameterSet&);
  ~HiOniaAnalyzer() override;

private:
  void beginJob() override;
  void analyze(const edm::Event&, const edm::EventSetup&) override;
  void endJob() override;

  void InitEvent();
  void InitTree();

  void makeCuts(bool keepSameSign);
  bool checkCuts(const pat::CompositeCandidate* cand,
                 const pat::Muon* muon1,
                 const pat::Muon* muon2,
                 bool (HiOniaAnalyzer::*callFunc1)(const pat::Muon*),
                 bool (HiOniaAnalyzer::*callFunc2)(const pat::Muon*));

  reco::GenParticleRef findDaughterRef(reco::GenParticleRef GenParticleDaughter, int GenParticlePDG);
  int IndexOfThisMuon(const float pt, bool isGen = false);
  int IndexOfThisDimuon(int mu1_idx, int mu2_idx);
  void fillGenInfo();
  void fillMuMatchingInfo();
  void fillDimuonMatchingInfo();
  bool isAbHadron(int pdgID);
  bool isNeutrino(int pdgID);
  bool isAMixedbHadron(int pdgID, int momPdgID);
  bool isChargedTrack(int pdgId);
  std::vector<reco::GenParticleRef> GenBrothers(reco::GenParticleRef GenParticleMother, int GenDimuonPDG);
  reco::GenParticleRef findMotherRef(reco::GenParticleRef GenParticleMother, int GenParticlePDG);
  std::pair<std::vector<reco::GenParticleRef>, std::pair<float, float> > findGenMCInfo(const reco::GenParticle& genDimuon);

  void fillRecoMuons(int theCentralityBin);
  bool isInAcceptance(const float eta, const float pt, std::string muonType);

  bool isSoftMuonBase(const pat::Muon* aMuon);
  bool isHybridSoftMuon(const pat::Muon* aMuon);
  Short_t MuInSV(LorentzVector v1, LorentzVector v2, LorentzVector v3);


  pair<unsigned int, const pat::CompositeCandidate*> theBestQQ();
  double CorrectMass(const reco::Muon& mu1, const reco::Muon& mu2, int mode);

  bool selGlobalMuon(const pat::Muon* aMuon);
  bool selTightMuon(const pat::Muon* aMuon);
  bool selTrackerMuon(const pat::Muon* aMuon);
  bool selGlobalOrTrackerMuon(const pat::Muon* aMuon);

  void fillRecoHistos();
  void fillRecoDimuon(int count, std::string trigName, std::string centName);

  void fillTreeMuon(const pat::Muon* muon, int iType, ULong64_t trigBits);
  void fillTreeDimuon(int count);

  void checkTriggers(const pat::CompositeCandidate* aDimuonCand);
  void hltReport(const edm::Event& iEvent, const edm::EventSetup& iSetup);

  long int FloatToIntkey(float v);
  void beginRun(const edm::Run&, const edm::EventSetup&) override;
  void endRun(const edm::Run&, const edm::EventSetup&) override{};

  int muonIDmask(const pat::Muon* muon);

  // ----------member data ---------------------------
  enum StatBins { BIN_nEvents = 0 };

  enum dimuonCategories {
    GlbTrk_GlbTrk = 0,
    Glb_Glb = 1,
    Trk_Trk = 2,
    GlbOrTrk_GlbOrTrk = 3,
    Tight_Tight = 4,
    All_All = 5
  };

  enum muonCategories { GlbTrk = 0, Trk = 1, Glb = 2, GlbOrTrk = 3, Tight = 4, All = 5 };

  std::vector<std::string> theCentralities;
  std::vector<std::string> theTriggerNames;
  std::vector<std::string> theSign;
  std::map<std::string, std::string> triggerNameMap;
  std::map<std::string, std::string> filterNameMap;

  HLTConfigProvider hltConfig;
  bool hltConfigInit;


  // TFileService
  edm::Service<TFileService> fs;

  // // TFile
  // TFile* fOut;

  // TTree
  TTree* myTree;

  std::vector<LorentzVector> Gen_Muon_4mom;
  std::vector<LorentzVector> Gen_Dimuon_4mom;
  std::vector<LorentzVector> Reco_Muon_4mom;

  std::vector<float> Reco_Dimuon_vtx_xpos;
  std::vector<float> Reco_Dimuon_vtx_ypos;
  std::vector<float> Reco_Dimuon_vtx_zpos;

  
  std::vector<float> Reco_Muon_4mom_pt;
  std::vector<float> Reco_Muon_ptErr_inner;
  std::vector<float> Reco_Dimuon_4mom_pt;
  std::vector<float> Reco_Dimuon_mumi_4mom_pt;
  std::vector<float> Reco_Dimuon_mupl_4mom_pt;
  std::vector<float> Reco_Dimuon_muonPtDiff;
  std::vector<float> Reco_Dimuon_muonPtRelDiff;


  std::vector<float> Gen_Muon_4mom_pt;
  std::vector<float> Gen_Dimuon_4mom_pt;
  std::vector<float> Gen_Dimuon_muonPtDiff;
  std::vector<float> Gen_Dimuon_muonPtRelDiff;

  std::vector<float> Reco_Muon_4mom_eta;
  std::vector<float> Reco_Muon_L1_4mom_eta;
  std::vector<float> Reco_Dimuon_4mom_eta;
  std::vector<float> Reco_Dimuon_mumi_4mom_eta;
  std::vector<float> Reco_Dimuon_mupl_4mom_eta;

  std::vector<float> Gen_Muon_4mom_eta;
  std::vector<float> Gen_Dimuon_4mom_eta;

  std::vector<float> Gen_Dimuon_4mom_y;
  std::vector<float> Reco_Dimuon_4mom_y;

  std::vector<float> Reco_Muon_4mom_phi;
  std::vector<float> Reco_Muon_L1_4mom_phi;
  std::vector<float> Reco_Dimuon_4mom_phi;
  std::vector<float> Reco_Dimuon_mumi_4mom_phi;
  std::vector<float> Reco_Dimuon_mupl_4mom_phi;

  std::vector<float> Gen_Muon_4mom_phi;
  std::vector<float> Gen_Dimuon_4mom_phi;

  std::vector<float> Reco_Muon_4mom_m;
  std::vector<float> Reco_Muon_L1_4mom_m;
  std::vector<float> Reco_Dimuon_4mom_m;
  std::vector<float> Reco_Dimuon_mumi_4mom_m;
  std::vector<float> Reco_Dimuon_mupl_4mom_m;

  std::vector<float> Gen_Muon_4mom_m;
  std::vector<float> Gen_Dimuon_4mom_m;

  static const int NMaxDimuons = 1000;
  static const int NMaxMuons = 1000;

  float Gen_weight;  // generator weight
  float Gen_pthat;   // ptHat scale of generated hard scattering

  Short_t Gen_Dimuon_size;               // number of generated Onia
  Short_t Gen_Dimuon_type[NMaxDimuons];  // Onia type: prompt, non-prompt, unmatched
  float Gen_Dimuon_ctau[NMaxDimuons];    // ctau: flight time
  float Gen_Dimuon_ctau3D[NMaxDimuons];  // ctau3D: 3D flight time
  int Gen_Dimuon_momId
      [NMaxDimuons];  // PDG ID of the generated mother of the Gen QQ, going back far enough in the geneaology to find a potential B mother
  float Gen_Dimuon_momPt[NMaxDimuons];       // Pt of mother particle of 2 muons
  Short_t Gen_Dimuon_muonPlusIndex[NMaxDimuons];  // index of the muon plus from Dimuon, in the full list of muons
  Short_t Gen_Dimuon_muonMinusIndex[NMaxDimuons];  // index of the muon minus from Dimuon, in the full list of muons
  Short_t Gen_Dimuon_whichRec
      [NMaxDimuons];  // index of the reconstructed Dimuon that was matched with this gen Dimuon. Is -1 if one of the 2 muons from Dimuon was not reconstructed. Is -2 if the two muons were reconstructed, but the dimuon was not selected

  Short_t Gen_Muon_size;                 // number of generated muons
  Short_t Gen_Muon_charge[NMaxMuons];  // muon charge
  Short_t Gen_Muon_type[NMaxMuons];    // muon type: prompt, non-prompt, unmatched
  Short_t Gen_Muon_whichRec
      [NMaxMuons];  // index of the reconstructed muon that was matched with this gen muon. Is -1 if the muon was not reconstructed
  float Gen_Muon_MatchDeltaR[NMaxMuons];  // deltaR between reco and gen matched muons


  Short_t Reco_Dimuon_size;                   // Number of reconstructed dimuons
  Short_t Reco_Dimuon_type[NMaxDimuons];      // Dimuon category: GG, GT, TT
  Short_t Reco_Dimuon_sign[NMaxDimuons];      /* Mu Mu combinations sign:
                             0 = +/- (signal)
                             1 = +/+
                             2 = -/- 
                          */
  Short_t Reco_Dimuon_muonPlusIndex[NMaxDimuons];  // index of the muon plus from Dimuon, in the full list of muons
  Short_t Reco_Dimuon_muonMinusIndex[NMaxDimuons];  // index of the muon minus from Dimuon, in the full list of muons
  Short_t Reco_Dimuon_whichGen
      [NMaxDimuons];  // index of the generated Dimuon that was matched with this rec Dimuon. Is -1 if one of the 2 muons from Dimuon was not reconstructed
  ULong64_t Reco_Dimuon_trig[NMaxDimuons];  // Vector of trigger bits matched to the Onia
  float Reco_Dimuon_VtxProb[NMaxDimuons];   // chi2 probability of vertex fitting
  float Reco_Dimuon_ctau[NMaxDimuons];      // ctau: flight time
  float Reco_Dimuon_ctauErr[NMaxDimuons];   // error on ctau
  float Reco_Dimuon_cosAlpha
      [NMaxDimuons];  // cosine of angle between momentum of Dimuon and direction of PV--displaced vertex segment (in XY plane)
  float Reco_Dimuon_ctau3D[NMaxDimuons];     // ctau: flight time in 3D
  float Reco_Dimuon_ctauErr3D[NMaxDimuons];  // error on ctau in 3D
  float Reco_Dimuon_cosAlpha3D
      [NMaxDimuons];  // cosine of angle between momentum of Dimuon and direction of PV--displaced vertex segment (3D)
  float Reco_Dimuon_dca[NMaxDimuons];
  float Reco_Dimuon_MassErr[NMaxDimuons];


  float Reco_Dimuon_mupl_dxy[NMaxDimuons];  // dxy for plus inner track muons
  float Reco_Dimuon_mumi_dxy[NMaxDimuons];  // dxy for minus inner track muons
  float Reco_Dimuon_mupl_dz[NMaxDimuons];   // dz for plus inner track muons
  float Reco_Dimuon_mumi_dz[NMaxDimuons];   // dz for minus inner track muons

  Short_t Reco_Muon_size;  // Number of reconstructed muons
  int Reco_Muon_SelectionType[NMaxMuons];
  ULong64_t Reco_Muon_trig[NMaxMuons];  // Vector of trigger bits matched to the muon
  Short_t Reco_Muon_charge[NMaxMuons];  // Vector of charge of muons
  Short_t Reco_Muon_type[NMaxMuons];    // Vector of type of muon (global=0, tracker=1, calo=2)
  Short_t Reco_Muon_whichGen
      [NMaxMuons];  // index of the generated muon that was matched with this reco muon. Is -1 if the muon is not associated with a generated muon (fake, or very bad resolution)

  bool Reco_Muon_highPurity[NMaxMuons];     // Vector of high purity flag
  bool Reco_Muon_TrkMuArb[NMaxMuons];       // Vector of TrackerMuonArbitrated
  bool Reco_Muon_TMOneStaTight[NMaxMuons];  // Vector of TMOneStationTight
  Short_t Reco_Muon_candType
      [NMaxMuons];  // candidate type of muon. 0 (or not present): muon collection, 1: packedPFCandidate, 2: lostTrack collection
  bool Reco_Muon_isPF[NMaxMuons];  // Vector of isParticleFlow muon
  bool Reco_Muon_isTracker[NMaxMuons];
  bool Reco_Muon_isGlobal[NMaxMuons];
  bool Reco_Muon_isSoftCutBased[NMaxMuons];
  bool Reco_Muon_isHybridSoft[NMaxMuons];
  bool Reco_Muon_isLooseCutBased[NMaxMuons];
  bool Reco_Muon_isMediumCutBased[NMaxMuons];
  bool Reco_Muon_isTightCutBased[NMaxMuons];

  float Reco_Muon_softMVAValue[NMaxMuons];
  float Reco_Muon_muonMVAValue[NMaxMuons]; // https://muon-wiki.docs.cern.ch/guidelines/recommendations/#muon-mva

  // muon isolation

  // from muon POG's recommendations https://muon-wiki.docs.cern.ch/guidelines/recommendations/#muon-isolation
  
  std::vector<bool> Reco_Muon_passesPFIsoLoose;
  std::vector<bool> Reco_Muon_passesPFIsoMedium;
  std::vector<bool> Reco_Muon_passesPFIsoTight;
  std::vector<bool> Reco_Muon_passesPFIsoVeryTight;

  std::vector<float> Reco_Muon_isoTrackSumPt;

  std::vector<bool> Reco_Muon_passesMultiIsoMedium;
  
  // HI specific
  std::vector<float> Reco_Muon_HIMVAIso;
  std::map<std::string, std::vector<bool>> Reco_Muon_HIMVAIsoWPs{{{"WP95",{}}, {"WP90",{}}, {"WP85",{}}, {"WP80",{}}}};

  bool Reco_Muon_InTightAcc[NMaxMuons];  // Is in the tight acceptance for global muons
  bool Reco_Muon_InLooseAcc[NMaxMuons];  // Is in the loose acceptance for global muons

  int Reco_Muon_nPixValHits[NMaxMuons];      // Number of valid pixel hits in sta muons
  int Reco_Muon_nMuValHits[NMaxMuons];       // Number of valid muon hits in sta muons
  int Reco_Muon_nTrkHits[NMaxMuons];         // track hits global muons
  int Reco_Muon_nPixWMea[NMaxMuons];         // pixel layers with measurement for inner track muons
  int Reco_Muon_nTrkWMea[NMaxMuons];         // track layers with measurement for inner track muons
  int Reco_Muon_nStationsMatched[NMaxMuons];  // number of stations matched for inner track muons
  float Reco_Muon_segmentComp[NMaxMuons];
  float Reco_Muon_kink[NMaxMuons];
  float Reco_Muon_localChi2[NMaxMuons];
  float Reco_Muon_normChi2_bestTracker[NMaxMuons];
  float Reco_Muon_normChi2_inner[NMaxMuons];   // chi2/ndof for inner track muons
  float Reco_Muon_normChi2_global[NMaxMuons];  // chi2/ndof for global muons
  float Reco_Muon_dxy[NMaxMuons];              // dxy for inner track muons
  float Reco_Muon_dxyErr[NMaxMuons];           // dxy error for inner track muons
  float Reco_Muon_dz[NMaxMuons];               // dz for inner track muons
  float Reco_Muon_dzErr[NMaxMuons];            // dz error for inner track muons
  float Reco_Muon_pt_inner[NMaxMuons];         // pT for inner track muons
  float Reco_Muon_pt_global[NMaxMuons];        // pT for global muons
  float Reco_Muon_ptErr_global[NMaxMuons];     // pT error for global muons
  float Reco_Muon_pTrue[NMaxMuons];  // P of the associated generated muon, used to match the Reco_Muon with the Gen_Muon
  float Reco_Muon_validFraction[NMaxMuons];
  int Reco_Muon_simExtType[NMaxMuons];  //

  Short_t muType;  // type of muon (GlbTrk=0, Trk=1, Glb=2, none=-1)

  // histos
  TH1F* hGoodMuonsNoTrig = nullptr;
  TH1F* hGoodMuons = nullptr;

  // event counters
  TH1F* hStats = nullptr;

  // centrality
  TH1F* hCent = nullptr;

  // number of primary vertices
  TH1F* hPileUp = nullptr;

  // z vertex distribution
  TH1F* hZVtx = nullptr;

  // centrality
  int centBin;
  int theCentralityBin;

  Short_t Npix, NpixelTracks, Ntracks;
  int NtracksPtCut, NtracksEtaCut, NtracksEtaPtCut;
  float SumET_HF, SumET_HFplus, SumET_HFminus, SumET_HFplusEta4, SumET_HFminusEta4, SumET_HFhit, SumET_HFhitPlus,
      SumET_HFhitMinus, SumET_EB, SumET_ET, SumET_EE, SumET_EEplus, SumET_EEminus, SumET_ZDC, SumET_ZDCplus,
      SumET_ZDCminus;

  // Event Plane variables
  int nEP;  // number of event planes
  //float *hiEvtPlane;
  float rpAng[50];
  float rpCos[50];
  float rpSin[50];
  float rpAng_origin[50];
  float rpCos_origin[50];
  float rpSin_origin[50];

  // handles
  edm::Handle<pat::CompositeCandidateCollection> collDimuon;
  edm::Handle<pat::MuonCollection> collMuon;
  edm::Handle<pat::MuonCollection> collMuonNoTrig;
  edm::Handle<reco::VertexCollection> SVs;

  edm::Handle<reco::GenParticleCollection> collGenParticles;
  edm::Handle<GenEventInfoProduct> genInfo;

  edm::Handle<edm::TriggerResults> collTriggerResults;

  // data members
  edm::EDGetTokenT<pat::MuonCollection> _patMuonToken;
  edm::EDGetTokenT<pat::MuonCollection> _patMuonNoTrigToken;
  edm::EDGetTokenT<pat::CompositeCandidateCollection> _patDimuonToken;
  edm::EDGetTokenT<reco::GenParticleCollection> _genParticleToken;
  edm::EDGetTokenT<GenEventInfoProduct> _genInfoToken;
  edm::EDGetTokenT<reco::VertexCollection> _thePVsToken;
  edm::EDGetTokenT<reco::VertexCollection> _SVToken;
  edm::EDGetTokenT<edm::TriggerResults> _tagTriggerResultsToken;
  edm::EDGetTokenT<reco::Centrality> _centralityTagToken;
  edm::EDGetTokenT<int> _centralityBinTagToken;
  edm::EDGetTokenT<reco::EvtPlaneCollection> _evtPlaneTagToken;
  std::string _histfilename;
  std::string _datasetname;
  std::string _muonSel;

  std::vector<double> _centralityranges;
  std::vector<string> _dblTriggerPathNames;
  std::vector<string> _sglTriggerPathNames;

  bool _applycuts;
  bool _SumETvariables;
  bool _selTightGlobalMuon;
  bool _muonLessPrimaryVertex;
  bool _useSVfinder;
  bool _useBS;
  bool _useRapidity;
  bool _removeSignal;
  bool _removeMuons;
  bool _storeSs;
  bool _AtLeastOneCand;
  bool _combineCategories;
  bool _fillTree;
  bool _fillHistos;
  bool _fillSingleMuons;
  bool _isHI;
  bool _isPA;
  bool _isMC;
  bool _isPromptMC;
  bool _useEvtPlane;
  bool _genealogyInfo;

  int _oniaPDG;
  int _OneMatchedHLTMu;
  bool _checkTrigNames;
  bool _genOnly;

  bool _addMuonIsolation;

  std::vector<unsigned int> _thePassedCats;
  std::vector<const pat::CompositeCandidate*> _thePassedCands;

  std::vector<reco::GenParticleRef> _Gen_Dimuon_MomAndTrkBro[NMaxDimuons];

  // number of events
  unsigned int nEvents;
  unsigned int passedCandidates;

  unsigned int runNb;
  unsigned int eventNb;
  unsigned int lumiSection;

  math::XYZPoint RefVtx;
  float RefVtx_xError;
  float RefVtx_yError;
  float RefVtx_zError;
  float zVtx;
  Short_t nPV;

  // Trigger stuff
  // PUT HERE THE *LAST FILTERS* OF THE BITS YOU LIKE
  static const unsigned int sNTRIGGERS = 65;
  unsigned int NTRIGGERS;
  unsigned int NTRIGGERS_DBL;
  unsigned int nTrig;

  // MC 8E29
  bool isTriggerMatched[sNTRIGGERS];
  bool alreadyFilled[sNTRIGGERS];
  ULong64_t HLTriggers;
  int trigPrescale[sNTRIGGERS];

  std::map<std::string, int> mapTriggerNameToIntFired_;
  std::map<std::string, int> mapTriggerNameToPrescaleFac_;
  std::map<long int, int> mapMuonMomToIndex_;
  std::map<long int, int> mapGenMuonMomToIndex_;

  HLTPrescaleProvider hltPrescaleProvider;
  bool hltPrescaleInit;

  const edm::ParameterSet _iConfig;
};

#endif
