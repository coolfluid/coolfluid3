// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_NavierStokes_hpp
#define cf3_UFEM_NavierStokes_hpp

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/max.hpp>

#define BOOST_PROTO_MAX_ARITY 11
#ifdef BOOST_MPL_LIMIT_METAFUNCTION_ARITY
  #undef BOOST_MPL_LIMIT_METAFUNCTION_ARITY
#endif
#define BOOST_MPL_LIMIT_METAFUNCTION_ARITY 11

#include <boost/scoped_ptr.hpp>

#include "LibUFEM.hpp"
#include "LSSActionUnsteady.hpp"
#include "NavierStokesPhysics.hpp"

namespace cf3 {
  
  namespace solver { class ActionDirector; }
  
namespace UFEM {

/// solver for the unsteady incompressible Navier-Stokes equations
class UFEM_API NavierStokes : public LSSActionUnsteady
{
public: // functions

  /// Contructor
  /// @param name of the component
  NavierStokes ( const std::string& name );
  
  virtual ~NavierStokes();

  /// Get the class name
  static std::string type_name () { return "NavierStokes"; }

private:
  /// Create the solver structure, based on the choice of specialized code
  void trigger_assembly();

  /// Called when the initial condition manager is changed
  virtual void on_initial_conditions_set(InitialConditions& initial_conditions);

  /// Helper function to set the expression, taking into account the user's option to use specializations or not.
  /// Implemented in NavierStokesAssembly.hpp
  template<typename GenericElementsT, typename SpecializedElementsT>
  void set_assembly_expression(const std::string& action_name);
  
  /// Helper function to set the expression, taking into account the user's option to use specializations or not.
  /// Implemented in BoussinesqAssembly.hpp
  template<typename GenericElementsT, typename SpecializedElementsT>
  void set_boussinesq_assembly_expression(const std::string& action_name);

  /// Helper function to set the expression, taking into account the user's option to use specializations or not.
  /// Implemented in BoussinesqAssemblyExtended.hpp
  template<typename GenericElementsT, typename SpecializedElementsT>
  void set_boussinesq_assembly_extended_expression(const std::string& action_name);

  /// Helper functions to split the compilation over multiple units, to save memory. Each one is in a different cpp file.
  void set_triag_assembly(const bool use_specialization);
  void set_quad_assembly();
  void set_tetra_assembly(const bool use_specialization);
  void set_hexa_assembly();
  

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

  /// Temperature field
  FieldVariable<7, ScalarField> Temp;
  /// Temperature field
  FieldVariable<8, ScalarField> Temp_ref;


  /// Access to the physics
  PhysicsConstant u_ref;
  PhysicsConstant rho;
  PhysicsConstant nu;

  PhysicsConstant temp_ref;
  PhysicsConstant rho_ref;
  PhysicsConstant betha;
  PhysicsConstant cp_heat_capacity;
  PhysicsConstant kappa_heat_cond;
  PhysicsConstant g_acceleration;

  /// Storage of the stabilization coefficients
  Real tau_ps, tau_su, tau_bulk;
  
  Handle<solver::ActionDirector> m_assembly;
  Handle<solver::ActionDirector> m_update;
  Handle<common::Action> m_initial_conditions;
};

} // UFEM
} // cf3


#endif // cf3_UFEM_NavierStokes_hpp
