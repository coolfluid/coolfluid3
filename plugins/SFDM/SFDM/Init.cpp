// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"
#include "common/OptionArray.hpp"
#include "common/OptionComponent.hpp"
#include "common/FindComponents.hpp"
#include "common/Log.hpp"

#include "Physics/Variables.hpp"

#include "mesh/Geometry.hpp"
#include "mesh/CRegion.hpp"
#include "mesh/Field.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/CCells.hpp"
#include "mesh/ElementType.hpp"
#include "mesh/CList.hpp"
#include "mesh/CSpace.hpp"

#include "Solver/CSolver.hpp"

#include "SFDM/Init.hpp"
#include "SFDM/Tags.hpp"

using namespace cf3::common;
using namespace cf3::mesh;
using namespace cf3::Physics;

namespace cf3 {
namespace SFDM {

////////////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < Init, common::Action, LibSFDM > Init_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

Init::Init ( const std::string& name ) :
  cf3::Solver::Action(name)
{
  mark_basic();

  m_options.add_option(OptionComponent<Field>::create( "solution_field", &m_field ))
      ->pretty_name("Solution Field")
      ->description("The field to Initialize");

  m_options.add_option(OptionComponent<Variables>::create( "input_vars", &m_input_vars))
      ->pretty_name("Input Variables")
      ->description("The input variables.\nIf empty, Solution Variables will be used");

  m_options.add_option< OptionArrayT<std::string> > ("functions", std::vector<std::string>())
      ->pretty_name("Functions")
      ->description("math function applied as initial condition using Input Variables (vars x,y)")
      ->attach_trigger ( boost::bind ( &Init::config_function, this ) )
      ->mark_basic();

  m_function.variables("x,y,z");
}


void Init::config_function()
{
  std::vector<std::string> vs = m_options["functions"].value<std::vector<std::string> >();

  m_function.functions( vs );

  m_function.parse();
}


void Init::execute()
{
  if( is_null( m_field.lock() ) )
    throw SetupError(FromHere(),"Solution field not set.");

  Field& solution = *m_field.lock();

  Variables& solution_vars = find_component_with_tag(physical_model(),SFDM::Tags::solution_vars()).as_type<Variables>();

  if (m_input_vars.expired())
    configure_option("input_vars",solution_vars.uri());

  Variables& input_vars = *m_input_vars.lock();

  std::vector<Real> params(DIM_3D,0.);
  RealVector return_val( solution.row_size() );
  RealMatrix grad_vars( physical_model().neqs(), physical_model().ndim() );
  RealVector sol (physical_model().neqs() );

  std::auto_ptr<Physics::Properties> props = physical_model().create_properties();

  boost_foreach(CCells& elements, find_components_recursively<CCells>(solution.topology()))
  {
    CSpace& space = solution.space(elements);

    const RealMatrix& local_coords = space.shape_function().local_coordinates();

    RealMatrix geometry_coords;
    elements.allocate_coordinates(geometry_coords);

    RealMatrix space_coords;
    space.allocate_coordinates(space_coords);

    const ShapeFunction& geometry_shape_func = elements.element_type().shape_function();
    RealRowVector geometry_shape_func_values (geometry_shape_func.nb_nodes());

    for (Uint elem=0; elem<elements.size(); ++elem)
    {
      elements.put_coordinates(geometry_coords,elem);

      CConnectivity::ConstRow field_idx = space.indexes_for_element(elem);

      for (Uint node=0; node<space.nb_states();++node)
      {
        // Get the coordinates for this point, and put in params
        geometry_shape_func.compute_value(local_coords.row(node),geometry_shape_func_values);
        space_coords.row(node) = geometry_shape_func_values * geometry_coords;
        for (Uint d=0; d<space_coords.cols(); ++d)
          params[d] = space_coords(node,d);

        // Evaluate function
        m_function.evaluate(params,return_val);

        // Transform the return_val of the function to solution variables,
        if (&input_vars != &solution_vars)
        {
          input_vars.compute_properties(space_coords.row(node),return_val,grad_vars,*props);
          solution_vars.compute_variables(*props,sol);

          // Copy in the solution field
          solution.set_row(field_idx[node],sol);
        }
        else
        {
          // Copy in the solution field
          solution.set_row(field_idx[node],return_val);
        }

      }

    }
  }
  solution.synchronize();
}

////////////////////////////////////////////////////////////////////////////////////////////

} // SFDM
} // cf3
