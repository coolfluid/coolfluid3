// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_AdjointCt_hpp
#define cf3_UFEM_AdjointCt_hpp

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
/// solver for the unsteady incompressible AdjointCt equations
class UFEM_API AdjointCt : public LSSActionUnsteady
{
public: // functions

  /// Contructor
  /// @param name of the component
  AdjointCt ( const std::string& name );

  virtual ~AdjointCt();

  /// Get the class name
  static std::string type_name () { return "AdjointCt"; }
  virtual void execute();


private:
  /// Create the solver structure, based on the choice of specialized code
  void trigger_assembly();

  // Update Ct list
  void trigger_ct();

  // Update a list
  void trigger_a();

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
  FieldVariable<1, ScalarField> epsilon;
  /// Pressure Adjointturb
  FieldVariable<2, ScalarField> q;
  /// Velocity Adjointturb
  FieldVariable<3, VectorField> U;
  /// Effective viscosity field
  FieldVariable<4, ScalarField> nu_eff;
  // Body force
  FieldVariable<5, VectorField> g;
  /// Temperature field
  FieldVariable<6, ScalarField> density_ratio;
  /// Adjoint turbulent kinetic energy
  FieldVariable<7, ScalarField> ka;
  /// Adjoint turbulence dissipation
  FieldVariable<8, ScalarField> epsilona;
  /// turbulent kinetic energy
  FieldVariable<9, ScalarField> k;

  /// Access to the physics
  PhysicsConstant rho;
  PhysicsConstant nu;

  /// Storage of the stabilization coefficients
  Real tau_ps, tau_su, tau_bulk;
  Real theta = 1.0;
  std::vector<Real> m_ct;
  std::vector<Real> m_a; // per-disk a
  Real m_th = 0.;
  Real m_U_mean_disk = 0.;
  Real m_area = 0.;
  bool m_updating = false;
  Real m_turbulence = 0.;
  Real m_c_epsilon_1 = 1.44;
  Real m_c_mu = 0.09;

  bool m_first_call = true;

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


#endif // cf3_UFEM_AdjointCt_hpp
