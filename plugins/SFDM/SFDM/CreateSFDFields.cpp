// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Log.hpp"
#include "common/Builder.hpp"
#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"
#include "common/Builder.hpp"
#include "common/OptionT.hpp"
#include "common/OptionComponent.hpp"

#include "common/PE/debug.hpp"

#include "math/VariablesDescriptor.hpp"

#include "solver/CSolver.hpp"
#include "solver/actions/CForAllCells.hpp"

#include "mesh/Field.hpp"
#include "mesh/SpaceFields.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Elements.hpp"
#include "mesh/Space.hpp"
#include "mesh/ElementType.hpp"
#include "mesh/Region.hpp"
#include "mesh/Cells.hpp"
#include "mesh/FieldManager.hpp"
#include "mesh/Connectivity.hpp"

#include "physics/Variables.hpp"
#include "physics/PhysModel.hpp"

#include "SFDM/CreateSFDFields.hpp"
#include "SFDM/Tags.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace SFDM {

  using namespace common;
  using namespace mesh;
  using namespace solver::actions;
  using namespace solver;
  using namespace physics;

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < CreateSFDFields, common::Action, LibSFDM> CreateSFDFields_builder;

//////////////////////////////////////////////////////////////////////////////

CreateSFDFields::CreateSFDFields( const std::string& name )
  : solver::Action(name)
{
  properties()["brief"] = std::string("Create Fields for use with SFD");
  properties()["description"] = std::string("Fields to be created: ...");
}

/////////////////////////////////////////////////////////////////////////////

void CreateSFDFields::execute()
{
  const Uint solution_order = solver().option(SFDM::Tags::solution_order()).value<Uint>();

  std::string sfdm_fields_space = "sfdm_fields_P"+to_str(solution_order-1);

  if ( is_not_null (find_component_ptr_recursively_with_tag<SpaceFields>(mesh(),sfdm_fields_space)))
  {
    CFinfo << "field group ["<<sfdm_fields_space<<"] already exists, check now to create the fields" << CFendl;
  }
  else
  {
    SpaceFields& sfdm_fields = mesh().create_space_and_field_group(sfdm_fields_space,SpaceFields::Basis::CELL_BASED,"cf3.SFDM.P"+to_str(solution_order-1));
    sfdm_fields.add_tag(sfdm_fields_space);

    Component& solution_vars = find_component_with_tag(physical_model(),SFDM::Tags::solution_vars());
    Field& solution   = sfdm_fields.create_field(SFDM::Tags::solution(), solution_vars.as_type<Variables>().description().description() );
    solver().field_manager().create_component<Link>(SFDM::Tags::solution()).link_to(solution);
    solution.parallelize();

    Field& residual   = sfdm_fields.create_field(SFDM::Tags::residual(), solution.descriptor().description());
    residual.descriptor().prefix_variable_names("rhs_");
    solver().field_manager().create_component<Link>(SFDM::Tags::residual()).link_to(residual);

    Field& wave_speed = sfdm_fields.create_field(SFDM::Tags::wave_speed(), "ws[1]");
    solver().field_manager().create_component<Link>(SFDM::Tags::wave_speed()).link_to(wave_speed);

    Field& update_coeff = sfdm_fields.create_field(SFDM::Tags::update_coeff(), "uc[1]");
    solver().field_manager().create_component<Link>(SFDM::Tags::update_coeff()).link_to(update_coeff);

    Field& jacob_det = sfdm_fields.create_field(SFDM::Tags::jacob_det(), "jacob_det[1]");
    solver().field_manager().create_component<Link>(SFDM::Tags::jacob_det()).link_to(jacob_det);

    boost_foreach(Cells& elements, find_components_recursively<Cells>(sfdm_fields.topology()))
    {
      Space& space = jacob_det.space(elements);

      const RealMatrix& local_coords = space.shape_function().local_coordinates();

      RealMatrix geometry_coords;
      elements.allocate_coordinates(geometry_coords);

      for (Uint elem=0; elem<elements.size(); ++elem)
      {
        elements.put_coordinates(geometry_coords,elem);

        Connectivity::ConstRow field_idx = space.indexes_for_element(elem);

        for (Uint node=0; node<local_coords.rows();++node)
        {
          jacob_det[field_idx[node]][0]=elements.element_type().jacobian_determinant(local_coords.row(node),geometry_coords);
        }

      }
    }
  }
}

//////////////////////////////////////////////////////////////////////////////

} // SFDM
} // cf3
