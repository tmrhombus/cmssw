/**
  \class    pat::ModifiedSuperCluster ModifiedSuperCluster.h "PhysicsTools/PatAlgos/interface/ModifiedSuperCluster.h"
  \brief    loop over electrons, modify superclusters
*/

#include "DataFormats/EgammaCandidates/interface/GsfElectron.h"
#include "DataFormats/PatCandidates/interface/Electron.h"

#include "PhysicsTools/PatAlgos/interface/ObjectModifier.h"

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/Common/interface/RefToPtr.h"
#include "DataFormats/Common/interface/Association.h"
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidate.h"
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidateFwd.h"
#include "DataFormats/PatCandidates/interface/PackedCandidate.h"
#include "CommonTools/UtilAlgos/interface/StringCutObjectSelector.h"
#include "RecoEcal/EgammaCoreTools/interface/EcalClusterLazyTools.h"
#include "FWCore/Utilities/interface/isFinite.h"

namespace pat {

  class ModifiedSuperCluster : public edm::EDProducer {
    public:
      explicit ModifiedSuperCluster(const edm::ParameterSet & iConfig);
      virtual ~ModifiedSuperCluster() { }

      virtual void produce(edm::Event & iEvent, const edm::EventSetup & iSetup);
      virtual void beginLuminosityBlock(const edm::LuminosityBlock&, const  edm::EventSetup&) override final;

    private:
      edm::EDGetTokenT<edm::View<pat::Electron> > src_;
      bool modifySuperCluster_;
      std::unique_ptr<pat::ObjectModifier<reco::CaloCluster> > superClusterModifier_;

  };

} // namespace

pat::ModifiedSuperCluster::ModifiedSuperCluster(const edm::ParameterSet & iConfig) :
    src_(consumes<edm::View<pat::Electron> >(iConfig.getParameter<edm::InputTag>("src"))),
    modifySuperCluster_(iConfig.getParameter<bool>("modifySuperClusters"))
    
{
    edm::ConsumesCollector sumes(consumesCollector());
    if( modifySuperCluster_ ) {
      const edm::ParameterSet& mod_config = iConfig.getParameter<edm::ParameterSet>("modifierConfig");
      superClusterModifier_.reset(new pat::ObjectModifier<reco::CaloCluster>(mod_config) );
      superClusterModifier_->setConsumes(sumes);
    } else {
      superClusterModifier_.reset(nullptr);
    }

    produces<std::vector<pat::Electron> >();
}

void 
pat::ModifiedSuperCluster::beginLuminosityBlock(const edm::LuminosityBlock&, const  edm::EventSetup& iSetup) {
}

void 
pat::ModifiedSuperCluster::produce(edm::Event & iEvent, const edm::EventSetup & iSetup) {
    using namespace edm;
    using namespace std;

    Handle<View<pat::Electron> > src;
    iEvent.getByToken(src_, src);

    auto_ptr<vector<pat::Electron> >  out(new vector<pat::Electron>());
    out->reserve(src->size());

    if( modifySuperCluster_ ) { superClusterModifier_->setEvent(iEvent); }
    if( modifySuperCluster_ ) { superClusterModifier_->setEventContent(iSetup); }

    std::vector<unsigned int> keys;
    for (View<pat::Electron>::const_iterator it = src->begin(), ed = src->end(); it != ed; ++it) {
        out->push_back(*it);
        pat::Electron & electron = out->back();

	reco::CaloCluster scluster = *electron.superCluster();
        if( modifySuperCluster_ ) { superClusterModifier_->modify(scluster); }

    }

    iEvent.put(out);
}

#include "FWCore/Framework/interface/MakerMacros.h"
using namespace pat;
DEFINE_FWK_MODULE(ModifiedSuperCluster);
