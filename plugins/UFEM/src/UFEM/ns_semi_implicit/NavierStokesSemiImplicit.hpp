// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_NavierStokesSemiImplicit_hpp
#define cf3_UFEM_NavierStokesSemiImplicit_hpp

#include "solver/ActionDirector.hpp"
#include "solver/actions/Proto/ElementOperations.hpp"

#include "../LibUFEM.hpp"
#include "../LSSActionUnsteady.hpp"
#include "../NavierStokesPhysics.hpp"

#include "LSSVectorOps.hpp"

namespace cf3 {
namespace UFEM {

using solver::actions::Proto::SFOp;
using solver::actions::Proto::CustomSFOp;  

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

private:
  /// Set up matrix expressions
  template<typename T>
  void set_elements_expressions(const std::string& name);
  
  void set_elements_expressions_quad();
  void set_elements_expressions_triag();
  void set_elements_expressions_hexa();
  void set_elements_expressions_prism();
  void set_elements_expressions_tetra();
  
  /// Executed when the initial conditions are set
  void trigger_initial_conditions();
  
  void trigger_theta();
  void trigger_nb_iterations();
  void trigger_time();
  void trigger_timestep();
  
  virtual void on_regions_set();
  
  /// Variables
  /// The velocity solution field
  FieldVariable<0, VectorField> u;
  /// The pressure solution field
  FieldVariable<1, ScalarField> p;
  /// The linearized advection velocity
  FieldVariable<2, VectorField> u_adv;
  /// Velocity at time n-1
  FieldVariable<3, VectorField> u1;
  /// Velocity at time n-2
  FieldVariable<4, VectorField> u2;
  /// Velocity at time n-3
  FieldVariable<5, VectorField> u3;
  /// Effective viscosity field
  FieldVariable<6, ScalarField> nu_eff;
  
  SFOp< CustomSFOp<VectorLSSVector> > u_vec;
  SFOp< CustomSFOp<ScalarLSSVector> > p_vec;
  SFOp< CustomSFOp<VectorLSSVector> > a;
  SFOp< CustomSFOp<VectorLSSVector> > delta_a;
  SFOp< CustomSFOp<ScalarLSSVector> > delta_p;
  SFOp< CustomSFOp<ScalarLSSVector> > delta_p_sum;
  
  /// Access to the physics
  PhysicsConstant u_ref;
  PhysicsConstant nu;

  /// Storage of the stabilization coefficients
  Real tau_ps, tau_su, tau_bulk;
  
  /// Theta scheme parameter
  Real theta;
  
  /// Time step
  Real dt;
  
  /// LSS for the pressure
  Handle<LSSActionUnsteady> m_p_lss;
  /// LSS for the velocity
  Handle<LSSAction> m_u_lss;
  
  Handle<InitialConditions> m_initial_conditions;

  // Actions that handle different stages of assembly, used by the set_elements_expressions function
  Handle<common::Component> m_pressure_assembly;
  Handle<common::Component> m_mass_matrix_assembly;
  Handle<common::Component> m_velocity_assembly;
  Handle<common::Component> m_inner_loop;
  
  // Time component
  Handle<solver::Time> m_time;
};

} // UFEM
} // cf3


#endif // cf3_UFEM_NavierStokesSemiImplicit_hpp
