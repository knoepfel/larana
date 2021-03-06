////////////////////////////////////////////////////////////////////////
//
// A Chi2 based particleID
//
// tjyang@fnal.gov
//
////////////////////////////////////////////////////////////////////////
#ifndef CHI2PIDALG_H
#define CHI2PIDALG_H

#include <string>

#include "fhiclcpp/fwd.h"
#include "canvas/Persistency/Common/Ptr.h"

class TProfile;

namespace anab {
  class Calorimetry;
  class ParticleID;
}

namespace pid {

  class Chi2PIDAlg {

  public:

    Chi2PIDAlg(fhicl::ParameterSet const& pset);

    void DoParticleID(art::Ptr<anab::Calorimetry> calo,
		      anab::ParticleID &pidOut);

  private:

    std::string fTemplateFile;
    bool        fUseMedian;
    //std::string fCalorimetryModuleLabel;
    std::string fROOTfile;

    TProfile *dedx_range_pro;   ///< proton template
    TProfile *dedx_range_ka;    ///< kaon template
    TProfile *dedx_range_pi;    ///< pion template
    TProfile *dedx_range_mu;    ///< muon template

  };//
}// namespace
#endif // CHI2PIDALG_H
