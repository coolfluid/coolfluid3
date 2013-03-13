// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_NavierStokesSemiImplicit_hpp
#define cf3_UFEM_NavierStokesSemiImplicit_hpp

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/max.hpp>

#include <boost/scoped_ptr.hpp>

#include "solver/ActionDirector.hpp"
#include "solver/actions/Iterate.hpp"

#include "../LibUFEM.hpp"
#include "../LSSActionUnsteady.hpp"
#include "../NavierStokesPhysics.hpp"

namespace cf3 {
namespace UFEM {

/// solver for the unsteady incompressible Navier-Stokes equations
class UFEM_API NavierStokesSemiImplicit : public solver::ActionDirector
{
public: // functions

  /// Contructor
  /// @param name of the component
  NavierStokesSemiImplicit ( const std::string& name );
  
  virtual ~NavierStokesSemiImplicit();

  /// Get the class name
  static std::string type_name () { return "NavierStokesSemiImplicit"; }

  void execute();

private:
  /// Create the solver structure, based on the choice of specialized code
  void trigger_assembly();

  /// Executed when the initial conditions are set
  void trigger_initial_conditions();

  /// Helper functions to set the expression, taking into account the user's option to use specializations or not.
  template<typename ElementsT>
  void set_pressure_matrix_assembly(LSSActionUnsteady& lss, const std::string& action_name);

  void set_pressure_matrix_assembly_quad(LSSActionUnsteady& lss);

  template<typename ElementsT>
  void set_pressure_rhs_assembly(LSSActionUnsteady& lss, const std::string& action_name);

  void set_pressure_rhs_assembly_quad(LSSActionUnsteady& lss);

  template<typename ElementsT>
  void set_pressure_gradient_apply(LSSActionUnsteady& lss, const std::string& action_name);

  void set_pressure_gradient_apply_quad(LSSActionUnsteady& lss);

  template<typename ElementsT>
  void set_velocity_matrix_assembly(LSSActionUnsteady& lss, const std::string& action_name);

  void set_velocity_matrix_assembly_quad(LSSActionUnsteady& lss);

  void on_regions_set();
  
  /// Variables
  FieldVariable<0, VectorField> u;
  FieldVariable<1, ScalarField> p;
  FieldVariable<2, ScalarField> nu_eff;
  FieldVariable<3, VectorField> u_adv;
  
  FieldVariable<4, VectorField> a;

  FieldVariable<5, VectorField> delta_a_star;
  FieldVariable<6, VectorField> delta_a;

  FieldVariable<7, ScalarField> dp;
  
  /// Access to the physics
  PhysicsConstant u_ref;
  PhysicsConstant nu;

  /// Storage of the stabilization coefficients
  Real tau_ps, tau_su, tau_bulk;
  
  Handle<solver::ActionDirector> m_pressure_matrix_assembly;
  Handle<solver::actions::Iterate> m_inner_loop;
  Handle<solver::ActionDirector> m_update;
  Handle<InitialConditions> m_initial_conditions;
  // These actions are disabled after the first execution
  std::vector<std::string> m_actions_to_disable;

  bool m_recursing;
};

} // UFEM
} // cf3


#endif // cf3_UFEM_NavierStokesSemiImplicit_hpp
