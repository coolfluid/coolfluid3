// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_Polydisperse_hpp
#define cf3_UFEM_Polydisperse_hpp

#include "solver/Action.hpp"
#include "solver/ActionDirector.hpp"

#include "../InitialConditions.hpp"
#include "../LSSAction.hpp"

#include "LibUFEMParticles.hpp"

namespace cf3 {
namespace UFEM {
namespace particles {

/// Polydisperse flow modeling
class Polydisperse : public solver::Action
{
public: // functions

  /// Contructor
  /// @param name of the component
  Polydisperse ( const std::string& name );
  
  virtual ~Polydisperse();

  /// Get the class name
  static std::string type_name () { return "Polydisperse"; }

  virtual void execute();
  
private:
  virtual void on_regions_set();
  void trigger_nb_phases();
  void trigger_initial_conditions();
  
  Uint m_nb_phases;

  Handle<InitialConditions> m_initial_conditions;

  Handle<solver::ActionDirector> m_compute_velocities;
  Handle<InitialConditions> m_ic_actions;

  std::vector<std::string> m_concentration_tags;
  std::vector<std::string> m_concentration_variables;
  std::vector<std::string> m_weighted_volume_tags;
  std::vector<std::string> m_weighted_volume_variables;
  std::vector<std::string> m_tau_variables;
  std::vector<std::string> m_velocity_variables;
  std::vector<std::string> m_concentration_src_variables;
  std::vector<std::string> m_weighted_volume_src_variables;
  std::vector<std::string> m_gradient_tags;
  
  Handle<LSSAction> m_concentration_solver;
  Handle<common::Group> m_boundary_conditions;

  Real m_reference_volume;
};

} // particles
} // UFEM
} // cf3


#endif // cf3_UFEM_Polydisperse_hpp
