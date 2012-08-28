// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_PROTO_MAX_ARITY 10
#define BOOST_MPL_LIMIT_METAFUNCTION_ARITY 10

#include "common/Builder.hpp"

#include "solver/actions/SolveLSS.hpp"
#include "solver/actions/ZeroLSS.hpp"

#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"

#include "HeatConductionSteady.hpp"
#include "Tags.hpp"

namespace cf3 {
namespace UFEM {

using namespace common;
using namespace solver;
using namespace solver::actions;
using namespace solver::actions::Proto;

ComponentBuilder < HeatConductionSteady, LSSAction, LibUFEM > HeatConductionSteady_builder;

HeatConductionSteady::HeatConductionSteady(const std::string& name) : LSSAction(name)
{
  set_solution_tag("heat_conduction_solution");

  FieldVariable<0, ScalarField> temperature("Temperature", solution_tag());
  FieldVariable<1, ScalarField> heat("Heat", Tags::source_terms());

  ConfigurableConstant<Real> k("k", "Thermal conductivity (J/(mK))", 1.);
  ConfigurableConstant<Real> relaxation_factor_hc("relaxation_factor", "factor for relaxation in case of coupling", 0.1);

  create_component<ZeroLSS>("ZeroLSS");

  *this <<                                                                                          // The linear problem (= inner loop, but executed once here)
    create_proto_action("Assembly", elements_expression                                             // Assembly action added to linear problem
    (
      group
      (
        _A = _0, _T = _0,
        element_quadrature
        (
          _A(temperature) += k * transpose(nabla(temperature)) * nabla(temperature),
          _T(temperature) += transpose(N(temperature))*N(temperature)
        ),
        system_matrix +=  _A,
        system_rhs += -_A * _x + _T * nodal_values(heat)
      )
    ))
    << allocate_component<BoundaryConditions>("BoundaryConditions")                                                                        // boundary conditions
    << allocate_component<SolveLSS>("SolveLSS")                                                       // Solve the LSS
    << create_proto_action("SetSolution", nodes_expression(temperature += relaxation_factor_hc * solution(temperature)));     // Set the solution

  Handle<BoundaryConditions>(get_child("BoundaryConditions"))->set_solution_tag(solution_tag());
  get_child("BoundaryConditions")->mark_basic();
}

void HeatConductionSteady::on_initial_conditions_set(InitialConditions& initial_conditions)
{
  initial_conditions.create_initial_condition(solution_tag());
}


} // UFEM
} // cf3
