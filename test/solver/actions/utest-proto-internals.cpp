// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for proto operators"

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/max.hpp>

#include <boost/foreach.hpp>
#include <boost/test/unit_test.hpp>

#include "solver/Model.hpp"
#include "solver/Solver.hpp"

#include "solver/actions/Proto/ElementLooper.hpp"
#include "solver/actions/Proto/Expression.hpp"
#include "solver/actions/Proto/Functions.hpp"
#include "solver/actions/Proto/NodeLooper.hpp"
#include "solver/actions/Proto/Terminals.hpp"

#include "common/Core.hpp"
#include "common/Log.hpp"

#include "math/MatrixTypes.hpp"

#include "mesh/Domain.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Region.hpp"
#include "mesh/Elements.hpp"
#include "mesh/MeshReader.hpp"
#include "mesh/ElementData.hpp"
#include "mesh/FieldManager.hpp"
#include "mesh/Dictionary.hpp"

#include "mesh/Integrators/Gauss.hpp"
#include "mesh/ElementTypes.hpp"

#include "physics/PhysModel.hpp"

#include "Tools/MeshGeneration/MeshGeneration.hpp"
#include "Tools/Testing/TimedTestFixture.hpp"
#include "Tools/Testing/ProfiledTestFixture.hpp"

using namespace cf3;
using namespace cf3::solver;
using namespace cf3::solver::actions;
using namespace cf3::solver::actions::Proto;
using namespace cf3::mesh;
using namespace cf3::common;

using namespace cf3::math::Consts;

/// Check close, for testing purposes
inline void check_close(const RealMatrix2& a, const RealMatrix2& b, const Real threshold)
{
  for(Uint i = 0; i != a.rows(); ++i)
    for(Uint j = 0; j != a.cols(); ++j)
      BOOST_CHECK_CLOSE(a(i,j), b(i,j), threshold);
}

static boost::proto::terminal< void(*)(const RealMatrix2&, const RealMatrix2&, Real) >::type const _check_close = {&check_close};

////////////////////////////////////////////////////

/// List of all supported shapefunctions that allow high order integration
typedef boost::mpl::vector3< LagrangeP1::Line1D,
                             LagrangeP1::Quad2D,
                             LagrangeP1::Hexa3D
> HigherIntegrationElements;

typedef boost::mpl::vector5< LagrangeP1::Line1D,
                             LagrangeP1::Triag2D,
                             LagrangeP1::Quad2D,
                             LagrangeP1::Hexa3D,
                             LagrangeP1::Tetra3D
> VolumeTypes;

BOOST_AUTO_TEST_SUITE( ProtoOperatorsSuite )

template<typename T>
void print_type(T)
{
  std::cout << demangle(typeid(T).name()) << std::endl;
}

template<typename T>
void print_type()
{
  std::cout << demangle(typeid(T).name()) << std::endl;
}

template<typename ExprT>
void print_variables(const ExprT&)
{
  print_type<typename ExpressionProperties<ExprT>::VariablesT>();
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( EquationVars )
{
  static const boost::proto::terminal< RestrictToElementTypeTag< boost::mpl::vector0<> > >::type for_generic_elements = {};
  
  Real tau_ps, tau_su, tau_bulk;
  FieldVariable<0, VectorField> u("Velocity", "navier_stokes_u_solution");
  FieldVariable<1, ScalarField> p("Pressure", "navier_stokes_p_solution");
  FieldVariable<2, VectorField> u_adv("AdvectionVelocity", "linearized_velocity");
  FieldVariable<3, VectorField> u1("AdvectionVelocity1", "linearized_velocity");
  FieldVariable<4, VectorField> u2("AdvectionVelocity2", "linearized_velocity");
  FieldVariable<5, VectorField> u3("AdvectionVelocity3", "linearized_velocity");
  FieldVariable<6, ScalarField> nu_eff("EffectiveViscosity", "navier_stokes_viscosity");
  PhysicsConstant u_ref("reference_velocity");
  PhysicsConstant rho("density");
  PhysicsConstant nu("kinematic_viscosity");
  
  print_variables(group
  (
    _A = _0, _T = _0, _a = _0,
    for_generic_elements
    (
      element_quadrature
      (
        _A(u[_i], u[_i]) += nu_eff * transpose(nabla(u)) * nabla(u) + transpose(N(u) + tau_su*u_adv*nabla(u)) * u_adv*nabla(u), // Diffusion + advection
        _a[u[_i]]     += transpose(N(u) + tau_su*u_adv*nabla(u)) * nabla(p)[_i] / rho * nodal_values(p), // Pressure gradient (standard and SUPG)
        _A(u[_i], u[_j]) += transpose((tau_bulk + 0.33333333333333*nu_eff)*nabla(u)[_i] // Bulk viscosity and second viscosity effect
                              + 0.5*u_adv[_i]*(N(u) + tau_su*u_adv*nabla(u))) * nabla(u)[_j],  // skew symmetric part of advection (standard +SUPG)
        _T(u[_i], u[_i]) += transpose(N(u) + tau_su*u_adv*nabla(u)) * N(u) // Time, standard and SUPG
      )
    )
  ));
}

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
