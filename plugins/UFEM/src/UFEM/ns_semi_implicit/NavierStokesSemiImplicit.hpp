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
class UFEM_API NavierStokesSemiImplicit : public LSSActionUnsteady
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
  /// Executed when the initial conditions are set
  void on_initial_conditions_set(InitialConditions& initial_conditions);
  
  virtual void on_regions_set();

  /// Helper functions to set the expression, taking into account the user's option to use specializations or not.
  template<typename ElementsT>
  void set_matrix_assembly(LSSAction& rhs_lss, LSSAction& t_lss, const std::string& action_name);

  void set_matrix_assembly_quad(LSSAction& rhs_lss, LSSAction &t_lss);
  
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
  
  /// Access to the physics
  PhysicsConstant u_ref;
  PhysicsConstant nu;

  /// Storage of the stabilization coefficients
  Real tau_ps, tau_su, tau_bulk;
  
  /// Theta scheme parameter
  Real theta;
    
  Handle<math::VariablesDescriptor> m_variables_descriptor;
  
  // This LSS stores a matrix that is used to construct the RHS vector
  Handle<LSSAction> m_rhs_lss;
  Handle<LSSAction> m_t_lss;
};

} // UFEM
} // cf3


#endif // cf3_UFEM_NavierStokesSemiImplicit_hpp
