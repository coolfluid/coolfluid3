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

#include "solver/actions/SolveLSS.hpp"
#include "solver/actions/ZeroLSS.hpp"

#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"
#include <solver/Tags.hpp>

#include "HeatConductionSteady.hpp"
#include "Tags.hpp"

namespace cf3 {
namespace UFEM {

using namespace common;
using namespace solver;
using namespace solver::actions;
using namespace solver::actions::Proto;
using namespace mesh;

ComponentBuilder < HeatConductionSteady, LSSAction, LibUFEM > HeatConductionSteady_builder;

HeatConductionSteady::HeatConductionSteady ( const std::string& name ) : LSSAction ( name )
{
  options().add("heat_space_name", "geometry")
    .pretty_name("Heat Space Name")
    .description("The space to use for the heat source term")
    .attach_trigger(boost::bind(&HeatConductionSteady::trigger, this));

  trigger();
}

void HeatConductionSteady::trigger()
{
  // Fist clear the structure
  std::vector<std::string> child_names;
  BOOST_FOREACH(const Component& child, *this)
  {
    child_names.push_back(child.name());
  }

  BOOST_FOREACH(const std::string& name, child_names)
  {
    remove_component(name);
  }

  set_solution_tag("heat_conduction_solution");

  ConfigurableConstant<Real> k("k", "Thermal conductivity (J/(mK))", 1.);
  ConfigurableConstant<Real> relaxation_factor_hc("relaxation_factor_hc", "factor for relaxation in case of coupling", 0.5);

  FieldVariable<0, ScalarField> T("Temperature", "heat_conduction_solution");
  FieldVariable<1, ScalarField> q("Heat", "source_terms", options().option("heat_space_name").value<std::string>());

  create_component<ZeroLSS>("ZeroLSS");

  *this <<                                                                                          // The linear problem (= inner loop, but executed once here)
    create_proto_action("Assembly", elements_expression                                             // Assembly action added to linear problem
    (
      boost::mpl::vector6<LagrangeP1::Line1D, LagrangeP1::Triag2D, LagrangeP1::Tetra3D, LagrangeP1::Quad2D, LagrangeP1::Hexa3D, LagrangeP2::Line1D>(),
      group
      (
        _A = _0,
        element_quadrature
        (
          _A(T) += k * transpose(nabla(T)) * nabla(T)
        ),
        system_matrix +=  _A,
        system_rhs += -_A * _x + integral<2>(transpose(N(T))*N(q)*jacobian_determinant) * nodal_values(q)
      )
    ))
    << allocate_component<BoundaryConditions>("BoundaryConditions")                                                                        // boundary conditions
    << allocate_component<SolveLSS>("SolveLSS")                                                       // Solve the LSS
    << create_proto_action("SetSolution", nodes_expression(T += relaxation_factor_hc*solution(T)));     // Set the solution

  Handle<BoundaryConditions>(get_child("BoundaryConditions"))->set_solution_tag(solution_tag());
  get_child("BoundaryConditions")->mark_basic();

  configure_option_recursively(solver::Tags::physical_model(), m_physical_model);
}

void HeatConductionSteady::on_initial_conditions_set(InitialConditions& initial_conditions)
{
  initial_conditions.create_initial_condition(solution_tag());
}


} // UFEM
} // cf3
