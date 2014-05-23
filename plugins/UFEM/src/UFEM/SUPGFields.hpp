// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_SUPGFields_hpp
#define cf3_UFEM_SUPGFields_hpp

#include "solver/Time.hpp"

#include "solver/actions/Proto/ProtoAction.hpp"

#include "SUPG.hpp"
#include "LibUFEM.hpp"

namespace cf3 {
  namespace solver { class Time; }
namespace UFEM {

/// Store the SUPG coefficient values in a field
class UFEM_API SUPGFields : public solver::actions::Proto::ProtoAction
{
public:
  /// Contructor
  /// @param name of the component
  SUPGFields ( const std::string& name );

  virtual ~SUPGFields();

  /// Get the class name
  static std::string type_name () { return "SUPGFields"; }

  virtual void execute();
  virtual void on_regions_set();
  
private:
  ComputeTau compute_tau;
  Real tau_ps, tau_su, tau_bu;
  
  void trigger_time();
  void trigger_timestep();

  Handle<solver::Time> m_time;
  Real m_dt;

  // Action to handle computation of the SUPG terms
  Handle<solver::actions::Proto::ProtoAction> m_supg_terms;

  Handle<solver::actions::Proto::ProtoAction> m_zero_field;
};

} // UFEM
} // cf3


#endif // cf3_UFEM_SUPGFields_hpp
