// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_RelaxationTime_hpp
#define cf3_UFEM_RelaxationTime_hpp

#include "math/Consts.hpp"

#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"

#include "LibUFEMParticles.hpp"

namespace cf3 {
namespace UFEM {
namespace particles {

// Functor to compute the squared particle radius
struct SquaredParticleRadius : solver::actions::Proto::FunctionBase
{
  typedef Real result_type;
  
  Real operator()(const Real c, const Real zeta)
  {
    const Real r2 = ::pow(3./(4.*math::Consts::pi())*zeta/c*m_reference_volume, 2./3.);
    if(!std::isfinite(r2))
      return 0.;
    
    return r2;
  }
  
  Real m_reference_volume;
};
  
/// Compute the particle relaxation time
class RelaxationTime : public solver::actions::Proto::ProtoAction
{
public: // functions

  /// Contructor
  /// @param name of the component
  RelaxationTime ( const std::string& name );
  
  virtual ~RelaxationTime();

  /// Get the class name
  static std::string type_name () { return "RelaxationTime"; }

private:
  virtual void on_regions_set();

  SquaredParticleRadius r2;
};

} // particles
} // UFEM
} // cf3


#endif // cf3_UFEM_RelaxationTime_hpp
