// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include "common/Builder.hpp"
#include "common/OptionList.hpp"
#include "common/OptionComponent.hpp"
#include "common/FindComponents.hpp"
#include "common/Log.hpp"
#include "common/List.hpp"

#include "physics/Variables.hpp"

#include "mesh/Dictionary.hpp"
#include "mesh/Region.hpp"
#include "mesh/Field.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Cells.hpp"
#include "mesh/ElementType.hpp"
#include "mesh/Space.hpp"
#include "mesh/Connectivity.hpp"

#include "solver/Solver.hpp"

#include "sdm/Init.hpp"
#include "sdm/Tags.hpp"

using namespace cf3::common;
using namespace cf3::mesh;
using namespace cf3::physics;

namespace cf3 {
namespace sdm {

////////////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < Init, common::Action, LibSDM > Init_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

Init::Init ( const std::string& name ) :
  cf3::solver::Action(name)
{
  mark_basic();

  options().add("solution_field", m_field )
      .pretty_name("Solution Field")
      .description("The field to Initialize")
      .link_to(&m_field);

  options().add("input_vars", m_input_vars)
      .pretty_name("Input Variables")
      .description("The input variables.\nIf empty, Solution Variables will be used")
      .link_to(&m_input_vars);

  options().add("functions", std::vector<std::string>())
      .pretty_name("Functions")
      .description("math function applied as initial condition using Input Variables (vars x,y)")
      .attach_trigger ( boost::bind ( &Init::config_function, this ) )
      .mark_basic();

  m_function.variables("x,y,z");
}


void Init::config_function()
{
  std::vector<std::string> vs = options()["functions"].value<std::vector<std::string> >();

  m_function.functions( vs );

  m_function.parse();
}


void Init::execute()
{
  if( is_null( m_field ) )
    throw SetupError(FromHere(),"Solution field not set.");

  Field& solution = *m_field;

  Handle<Variables> solution_vars(find_component_ptr_with_tag(physical_model(),sdm::Tags::solution_vars()));

  if (is_null(m_input_vars))
    options().set("input_vars",solution_vars);

  Variables& input_vars = *m_input_vars;

  std::vector<Real> params(DIM_3D,0.);
  RealVector return_val( solution.row_size() );
  RealMatrix grad_vars( physical_model().neqs(), physical_model().ndim() );
  RealVector sol (physical_model().neqs() );

  std::auto_ptr<physics::Properties> props = physical_model().create_properties();

  boost_foreach(const Handle<Entities>& entities, solution.entities_range())
  {
    const Space& space = solution.space(*entities);

    const RealMatrix& local_coords = space.shape_function().local_coordinates();

    RealMatrix geometry_coords;
    entities->geometry_space().allocate_coordinates(geometry_coords);

    RealMatrix space_coords;
    space.allocate_coordinates(space_coords);

    const ShapeFunction& geometry_shape_func = entities->element_type().shape_function();
    RealRowVector geometry_shape_func_values (geometry_shape_func.nb_nodes());

    const Connectivity& field_connectivity = space.connectivity();

    for (Uint elem=0; elem<entities->size(); ++elem)
    {
      entities->geometry_space().put_coordinates(geometry_coords,elem);

      for (Uint node=0; node<space.shape_function().nb_nodes();++node)
      {
        Uint p = field_connectivity[elem][node];
        // Get the coordinates for this point, and put in params
        geometry_shape_func.compute_value(local_coords.row(node),geometry_shape_func_values);
        space_coords.row(node) = geometry_shape_func_values * geometry_coords;
        for (Uint d=0; d<space_coords.cols(); ++d)
          params[d] = space_coords(node,d);

        // Evaluate function
        m_function.evaluate(params,return_val);

        // Transform the return_val of the function to solution variables,
        if (m_input_vars != solution_vars)
        {
          input_vars.compute_properties(space_coords.row(node),return_val,grad_vars,*props);
          solution_vars->compute_variables(*props,sol);

          // Copy in the solution field
          solution.set_row(p,sol);
        }
        else
        {
          // Copy in the solution field
          solution.set_row(p,return_val);
        }

      }

    }
  }
  solution.synchronize();
}

////////////////////////////////////////////////////////////////////////////////////////////

} // sdm
} // cf3
