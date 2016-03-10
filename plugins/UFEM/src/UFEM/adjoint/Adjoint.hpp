// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_Adjoint_hpp
#define cf3_UFEM_Adjoint_hpp

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/max.hpp>

#define BOOST_PROTO_MAX_ARITY 11
#ifdef BOOST_MPL_LIMIT_METAFUNCTION_ARITY
  #undef BOOST_MPL_LIMIT_METAFUNCTION_ARITY
#endif
#define BOOST_MPL_LIMIT_METAFUNCTION_ARITY 11

#include <boost/scoped_ptr.hpp>

#include "LibUFEMAdjoint.hpp"
#include "../LSSActionUnsteady.hpp"
#include "../NavierStokesPhysics.hpp"
#include "../SUPG.hpp"

namespace cf3 {

  namespace solver { class ActionDirector; }

namespace UFEM {
namespace adjoint {
/// solver for the unsteady incompressible Adjoint equations
class UFEM_API Adjoint : public LSSActionUnsteady
{
public: // functions

  /// Contructor
  /// @param name of the component
  Adjoint ( const std::string& name );

  virtual ~Adjoint();

  /// Get the class name
  static std::string type_name () { return "Adjoint"; }

private:
  /// Create the solver structure, based on the choice of specialized code
  void trigger_assembly();

  ///On region set
  virtual void on_regions_set();

  /// Called when the initial condition manager is changed
  virtual void on_initial_conditions_set(InitialConditions& initial_conditions);

  /// Helper function to set the expression, taking into account the user's option to use specializations or not.
  /// Implemented in NavierStokesAssembly.hpp
  template<typename GenericElementsT, typename SpecializedElementsT>
  void set_assembly_expression(const std::string& action_name);

  /// Helper functions to split the compilation over multiple units, to save memory. Each one is in a different cpp file.
  void set_triag_assembly(const bool use_specialization);
  void set_quad_assembly();
  void set_tetra_assembly(const bool use_specialization);
  void set_hexa_assembly();
  void set_prism_assembly();


  /// The velocity solution field
  FieldVariable<0, VectorField> u;
  /// The pressure solution field
  FieldVariable<1, ScalarField> p;
  /// Pressure adjoint
  FieldVariable<2, ScalarField> q;
  /// Velocity adjoint
  FieldVariable<3, VectorField> U;
  /// The linearized advection velocity
  FieldVariable<4, VectorField> U_adv;
  /// Velocity at time n-1
  FieldVariable<5, VectorField> U1;
  /// Velocity at time n-2
  FieldVariable<6, VectorField> U2;
  /// Velocity at time n-3
  FieldVariable<7, VectorField> U3;
  /// Effective viscosity field
  FieldVariable<8, ScalarField> nu_eff;
  // Body force
  FieldVariable<9, VectorField> g;
  /// Temperature field
  FieldVariable<7, ScalarField> density_ratio;

  /// Access to the physics
  PhysicsConstant rho;
  PhysicsConstant nu;

  /// Storage of the stabilization coefficients
  Real tau_ps, tau_su, tau_bulk, theta = 0.5;
  std::vector<Real> ct;

  Handle<solver::ActionDirector> m_assembly;
  Handle<solver::ActionDirector> m_update;
  Handle<common::Action> m_initial_conditions;

  ComputeTau compute_tau;

  // Actuator disk regions
  std::vector<Handle<mesh::Region>> m_actuator_regions;
};
} // Adjoint
} // UFEM
} // cf3


#endif // cf3_UFEM_Adjoint_hpp
