// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_NavierStokesExplicit_hpp
#define cf3_UFEM_NavierStokesExplicit_hpp

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/max.hpp>

#define BOOST_PROTO_MAX_ARITY 10
#ifdef BOOST_MPL_LIMIT_METAFUNCTION_ARITY
  #undef BOOST_MPL_LIMIT_METAFUNCTION_ARITY
#endif
#define BOOST_MPL_LIMIT_METAFUNCTION_ARITY 10

#include "solver/ActionDirector.hpp"

#include "LibUFEM.hpp"
#include "LSSActionUnsteady.hpp"
#include "NavierStokesPhysics.hpp"

namespace cf3 {

  namespace solver
  {
    namespace actions { class Iterate; }
    class Time;
  }

namespace UFEM {

  class InitialConditions;

/// solver for the unsteady incompressible Navier-Stokes equations
class UFEM_API NavierStokesExplicit : public solver::ActionDirector
{
public: // functions

  /// Contructor
  /// @param name of the component
  NavierStokesExplicit ( const std::string& name );

  virtual ~NavierStokesExplicit();

  /// Get the class name
  static std::string type_name () { return "NavierStokesExplicit"; }
  
  void execute();

private:
  /// Create the solver structure, based on the choice of specialized code
  void trigger_assembly();

  void trigger_time();
  void trigger_timestep();
  void trigger_initial_conditions();

  /// Called when the initial condition manager is changed
  virtual void on_initial_conditions_set(InitialConditions& initial_conditions);

  /// Helper function to set the inner loop expressions
  template<typename ElementsT>
  void set_velocity_assembly_expression(const std::string& base_name);
  template<typename ElementsT>
  void set_velocity_implicit_assembly_expression(const std::string& base_name);
  template<typename ElementsT>
  void set_pressure_matrix_assembly_expression(const std::string& base_name);
  template<typename ElementsT>
  void set_pressure_rhs_assembly_expression(const std::string& base_name);
  template<typename ElementsT, typename RHST>
  void set_pressure_gradient_assembly_expression(const std::string& base_name, RHST& rhs);

  /// Helper functions to split the compilation over multiple units, to save memory. Each one is in a different cpp file.
  void set_triag_u_assembly();
  void set_triag_p_rhs_assembly();
  void set_triag_p_mat_assembly();
  void set_triag_grad_p_assembly(const solver::actions::Proto::SystemRHS& rhs);
  void set_triag_grad_p_assembly(FieldVariable<3, VectorField>& rhs);
  void set_triag_implicit_u_assembly();
  void set_quad_u_assembly();
  void set_quad_p_rhs_assembly();
  void set_quad_p_mat_assembly();
  void set_quad_grad_p_assembly(const solver::actions::Proto::SystemRHS& rhs);
  void set_quad_grad_p_assembly(FieldVariable<3, VectorField>& rhs);
  void set_quad_implicit_u_assembly();
  void set_hexa_u_assembly();
  void set_hexa_p_rhs_assembly();
  void set_hexa_p_mat_assembly();
  void set_hexa_grad_p_assembly(const solver::actions::Proto::SystemRHS& rhs);
  void set_hexa_grad_p_assembly(FieldVariable<3, VectorField>& rhs);
  void set_hexa_implicit_u_assembly();
  void set_tetra_u_assembly();
  void set_tetra_p_rhs_assembly();
  void set_tetra_p_mat_assembly();
  void set_tetra_grad_p_assembly(const solver::actions::Proto::SystemRHS& rhs);
  void set_tetra_grad_p_assembly(FieldVariable<3, VectorField>& rhs);
  void set_tetra_implicit_u_assembly();

  virtual void on_regions_set();


  /// The velocity solution field
  FieldVariable<0, VectorField> u;
  /// The pressure solution field
  FieldVariable<1, ScalarField> p;
  /// The acceleration
  FieldVariable<2, VectorField> a;
  /// The residual vector
  FieldVariable<3, VectorField> R;
  /// Mass matrix diagonal
  FieldVariable<4, VectorField> M;
  /// Effective viscosity field
  FieldVariable<5, ScalarField> nu_eff;

  /// Iteration variables
  FieldVariable<5, ScalarField> p_dot;
  FieldVariable<7, VectorField> delta_a_star;
  FieldVariable<8, VectorField> delta_a;
  FieldVariable<9, ScalarField> delta_p;

  /// Velocity linearization
  FieldVariable<6, VectorField> u_adv;

  /// Access to the physics
  PhysicsConstant u_ref;
  PhysicsConstant rho;
  PhysicsConstant nu;

  /// Storage of the stabilization coefficients
  Real tau_ps, tau_su, tau_bulk;

  /// Explcit algorithm constants
  Real gamma_u, gamma_p;

  /// Maximum Peclet number for the current timestep
  Real alpha;

  /// Timings
  Real m_dt, m_inv_dt;

  Handle<solver::actions::Iterate> m_inner_loop;
  Handle<LSSActionUnsteady> m_pressure_lss;
  Handle<LSSActionUnsteady> m_velocity_lss;
  Handle<common::Action> m_viscosity_initial_condition;
  Handle<common::Action> m_iteration_initial_condition;
  Handle<common::Action> m_velocity_initial_condition;
  Handle<common::Action> m_pressure_initial_condition;
  Handle<solver::ActionDirector> m_pressure_matrix_assembly;

  bool m_recursing;

  common::Option::TriggerID m_time_trigger_id;
  Handle<solver::Time> time;
};

} // UFEM
} // cf3


#endif // cf3_UFEM_NavierStokesExplicit_hpp
