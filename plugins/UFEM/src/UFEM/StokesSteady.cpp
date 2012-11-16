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

#include "StokesSteady.hpp"
#include "Tags.hpp"

namespace cf3 {
namespace UFEM {

using namespace common;
using namespace solver;
using namespace solver::actions;
using namespace solver::actions::Proto;
using namespace mesh;

ComponentBuilder < StokesSteady, LSSAction, LibUFEM > StokesSteady_builder;


StokesSteady::StokesSteady ( const std::string& name ) : LSSAction ( name )
{
  set_solution_tag("stokes_solution");

  FieldVariable<0, ScalarField> p("pressure", "stokes_solution");
  FieldVariable<1, VectorField> u("velocity", "stokes_solution");

  create_component<math::LSS::ZeroLSS>("ZeroLSS");
  
  add_component(create_proto_action("Assembly", elements_expression
  (
    boost::mpl::vector1<LagrangeP1::Hexa3D>(),
    group
    (
      _A = _0,
      element_quadrature
      (
        _A(p, u[_i])     += transpose(N(p)) * nabla(u)[_i],
        _A(p , p)        += 0.01 * transpose(nabla(p))*nabla(p),
        _A(u[_i], u[_i]) += transpose(nabla(u)) * nabla(u),
        _A(u[_i], p)     += transpose(N(u)) * nabla(p)[_i]
      ),
      system_matrix += _A
    )
  )));

  Handle<BoundaryConditions> bc = create_component<BoundaryConditions>("BoundaryConditions");
  bc->mark_basic();
  bc->set_solution_tag(solution_tag());

  create_component<math::LSS::SolveLSS>("SolveLSS");
  
  add_component(create_proto_action("SetSolution", nodes_expression(group(p = solution(p), u = solution(u)))));
}

void StokesSteady::on_initial_conditions_set(InitialConditions& initial_conditions)
{
  initial_conditions.create_initial_condition(solution_tag());
}


} // UFEM
} // cf3
