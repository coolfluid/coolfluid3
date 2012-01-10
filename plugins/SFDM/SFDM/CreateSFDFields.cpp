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
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"

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
  const Uint solution_order = solver().options().option(SFDM::Tags::solution_order()).value<Uint>();

  std::string solution_space_name = "solution_space";
//  std::string boundary_space_name = "boundary_space";

  if ( is_not_null (find_component_ptr_recursively_with_tag<SpaceFields>(mesh(),solution_space_name)))
  {
    CFinfo << "field group ["<<solution_space_name<<"] already exists, check now to create the fields" << CFendl;
  }
  else
  {
    SpaceFields& solution_space = mesh().create_space_and_field_group(solution_space_name,SpaceFields::Basis::ELEMENT_BASED,"cf3.SFDM.P"+to_str(solution_order-1));
    solution_space.add_tag(solution_space_name);

    Component& solution_vars = find_component_with_tag(physical_model(),SFDM::Tags::solution_vars());
    Field& solution   = solution_space.create_field(SFDM::Tags::solution(), solution_vars.handle<Variables>()->description().description() );
    solver().field_manager().create_component<Link>(SFDM::Tags::solution())->link_to(solution);
    solution.parallelize();

    Field& residual   = solution_space.create_field(SFDM::Tags::residual(), solution.descriptor().description());
    residual.descriptor().prefix_variable_names("rhs_");
    solver().field_manager().create_component<Link>(SFDM::Tags::residual())->link_to(residual);
    residual.properties()[SFDM::Tags::L2norm()]=0.;

    Field& wave_speed = solution_space.create_field(SFDM::Tags::wave_speed(), "ws[1]");
    solver().field_manager().create_component<Link>(SFDM::Tags::wave_speed())->link_to(wave_speed);

    Field& update_coeff = solution_space.create_field(SFDM::Tags::update_coeff(), "uc[1]");
    solver().field_manager().create_component<Link>(SFDM::Tags::update_coeff())->link_to(update_coeff);

    Field& jacob_det = solution_space.create_field(SFDM::Tags::jacob_det(), "jacob_det[1]");
    solver().field_manager().create_component<Link>(SFDM::Tags::jacob_det())->link_to(jacob_det);

    Field& delta = solution_space.create_field(SFDM::Tags::delta(), "delta[vector]");
    solver().field_manager().create_component<Link>(SFDM::Tags::delta())->link_to(delta);

    boost_foreach(Cells& elements, find_components_recursively<Cells>(solution_space.topology()))
    {
      Space& space = jacob_det.space(elements);

      const RealMatrix& local_coords = space.shape_function().local_coordinates();

      RealMatrix geometry_coords;
      elements.allocate_coordinates(geometry_coords);

      RealVector dKsi (elements.element_type().dimensionality()); dKsi.setConstant(2.);
      RealVector dX (elements.element_type().dimension());
      RealMatrix jacobian(elements.element_type().dimensionality(),elements.element_type().dimension());

      for (Uint elem=0; elem<elements.size(); ++elem)
      {
        elements.put_coordinates(geometry_coords,elem);

        Connectivity::ConstRow field_idx = space.indexes_for_element(elem);

        for (Uint node=0; node<local_coords.rows();++node)
        {

          jacob_det[field_idx[node]][0]=elements.element_type().jacobian_determinant(local_coords.row(node),geometry_coords);
          elements.element_type().compute_jacobian(local_coords.row(node),geometry_coords,jacobian);
          dX.noalias() = jacobian.transpose()*dKsi;
          for (Uint d=0; d<dX.size(); ++d)
            delta[field_idx[node]][d]=dX[d];
        }

      }
    }
  }


//  if ( is_not_null (find_component_ptr_recursively_with_tag<SpaceFields>(mesh(),boundary_space_name)))
//  {
//    CFinfo << "field group ["<<boundary_space_name<<"] already exists, check now to create the fields" << CFendl;
//  }
//  else
//  {
//    SpaceFields& boundary_space = mesh().create_space_and_field_group(solution_space_name,SpaceFields::Basis::FACE_BASED,"cf3.SFDM.P"+to_str(solution_order-1));
//    boundary_space.add_tag(boundary_space_name);

//    Component& solution_vars = find_component_with_tag(physical_model(),SFDM::Tags::solution_vars());
//    Field& solution   = boundary_space.create_field(SFDM::Tags::solution(), solution_vars.handle<Variables>()->description().description() );
//  }

}

//////////////////////////////////////////////////////////////////////////////

} // SFDM
} // cf3
