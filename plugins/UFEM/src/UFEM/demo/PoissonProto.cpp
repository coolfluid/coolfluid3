// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_PROTO_MAX_ARITY 10
#define BOOST_MPL_LIMIT_METAFUNCTION_ARITY 10

#include "common/Builder.hpp"
#include "common/OptionList.hpp"

#include "mesh/LagrangeP2/Line1D.hpp"
#include "mesh/LagrangeP2/LibLagrangeP2.hpp"
#include "mesh/LagrangeP1/LibLagrangeP1.hpp"

#include "math/LSS/SolveLSS.hpp"
#include "math/LSS/ZeroLSS.hpp"

#include "solver/actions/Proto/Expression.hpp"
#include "solver/Tags.hpp"

#include "UFEM/Tags.hpp"
#include "UFEM/NavierStokesPhysics.hpp"

#include "PoissonProto.hpp"


namespace cf3 {
namespace UFEM {
namespace demo {

using namespace solver::actions::Proto;

common::ComponentBuilder < PoissonProto, LSSAction, LibUFEMDemo > PoissonProto_builder;

PoissonProto::PoissonProto ( const std::string& name ) : LSSAction ( name )
{
  // This determines the name of the field that will be used to store the solution
  set_solution_tag("poisson_solution");

  // Create action components that wil be executed in the order they are created here:
  // 1. Set the linear system matrix and vectors to zero
  create_component<math::LSS::ZeroLSS>("ZeroLSS");

  // 2. Assemble the system matrix and RHS using a proto expression
  Handle<ProtoAction> assembly = create_component<ProtoAction>("Assembly");

  // The unknown function. The first template argument is a constant to distinguish each variable at compile time
  FieldVariable<0, ScalarField> u("u", solution_tag());

  // The source term, to be set at runtime using an initial condition
  FieldVariable<1, ScalarField> f("f", "source_term");

  // The expression itself
  assembly->set_expression(elements_expression
  (
    mesh::LagrangeP1::CellTypes(), // All P1 volume cell types are supported
    group
    (
      _A = _0, _a = _0,// The element matrix and RHS vector, initialized to 0
      element_quadrature // Integration over the element
      (
        _A(u) += transpose(nabla(u)) * nabla(u),
        _a[u] += transpose(N(u))*f
      ),
      system_matrix +=  _A, // Assemble into the global linear system
      system_rhs += _a
    )
  ));

  // 3. Apply bondary conditions
  Handle<BoundaryConditions> bc = create_component<BoundaryConditions>("BoundaryConditions");
  bc->mark_basic();
  bc->set_solution_tag(solution_tag());

  // 4. Solve the linear system
  create_component<math::LSS::SolveLSS>("SolveLSS");
  
  // 5. Update the solution
  create_component<ProtoAction>("Update")->set_expression(nodes_expression(u = solution(u)));
}

} // demo
} // UFEM
} // cf3
