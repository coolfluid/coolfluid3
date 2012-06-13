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

#include "solver/Solver.hpp"
#include "solver/actions/ForAllCells.hpp"


#include "mesh/Field.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Elements.hpp"
#include "mesh/Space.hpp"
#include "mesh/ElementType.hpp"
#include "mesh/Region.hpp"
#include "mesh/Cells.hpp"
#include "mesh/CellFaces.hpp"
#include "mesh/Faces.hpp"
#include "mesh/FieldManager.hpp"
#include "mesh/Connectivity.hpp"

#include "physics/Variables.hpp"
#include "physics/PhysModel.hpp"

#include "sdm/CreateSDFields.hpp"
#include "sdm/Tags.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace sdm {

  using namespace common;
  using namespace mesh;
  using namespace solver::actions;
  using namespace solver;
  using namespace physics;

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < CreateSDFields, common::Action, LibSDM> CreateSDFields_builder;

//////////////////////////////////////////////////////////////////////////////

CreateSDFields::CreateSDFields( const std::string& name )
  : solver::Action(name)
{
  properties()["brief"] = std::string("Create Fields for use with SFD");
  properties()["description"] = std::string("Fields to be created: ...");
}

/////////////////////////////////////////////////////////////////////////////

void CreateSDFields::execute()
{

//  mesh().check_sanity();
  const Uint solution_order = solver().options().option(sdm::Tags::solution_order()).value<Uint>();

  std::string solution_space_name = "solution_space";
//  std::string boundary_space_name = "boundary_space";

  if ( is_not_null (find_component_ptr_recursively_with_tag<Dictionary>(mesh(),solution_space_name)))
  {
    CFinfo << "field group ["<<solution_space_name<<"] already exists, check now to create the fields" << CFendl;
  }
  else
  {
    std::vector< Handle<Entities> > cells_plus_bdry;
    boost_foreach(Entities& entities, find_components_recursively<Cells>(mesh().topology()))
    {
      cells_plus_bdry.push_back(entities.handle<Entities>());
    }
    boost_foreach(Entities& entities, find_components_recursively_with_tag<Entities>(mesh().topology(),mesh::Tags::face_entity()))
    {
      if (entities.has_tag(mesh::Tags::outer_faces())==true)
        cells_plus_bdry.push_back(entities.handle<Entities>());
      if (entities.has_tag(mesh::Tags::interface())==true)
        cells_plus_bdry.push_back(entities.handle<Entities>());
    }
    boost_foreach(Entities& entities, find_components_recursively<Faces>(mesh().topology()))
    {
        cells_plus_bdry.push_back(entities.handle<Entities>());
    }

    Dictionary& solution_space = mesh().create_discontinuous_space(solution_space_name,"cf3.sdm.P"+to_str(solution_order-1),cells_plus_bdry);
    solution_space.add_tag(solution_space_name);

    Component& solution_vars = find_component_with_tag(physical_model(),sdm::Tags::solution_vars());
    Field& solution   = solution_space.create_field(sdm::Tags::solution(), solution_vars.handle<Variables>()->description().description() );
    solver().field_manager().create_component<Link>(sdm::Tags::solution())->link_to(solution);
    solution.parallelize();

    Field& residual   = solution_space.create_field(sdm::Tags::residual(), solution.descriptor().description());
    residual.descriptor().prefix_variable_names("rhs_");
    solver().field_manager().create_component<Link>(sdm::Tags::residual())->link_to(residual);
    residual.properties()[sdm::Tags::L2norm()]=0.;
    residual.parallelize();

    Field& wave_speed = solution_space.create_field(sdm::Tags::wave_speed(), "ws[1]");
    solver().field_manager().create_component<Link>(sdm::Tags::wave_speed())->link_to(wave_speed);
    wave_speed.parallelize();

    Field& update_coeff = solution_space.create_field(sdm::Tags::update_coeff(), "uc[1]");
    solver().field_manager().create_component<Link>(sdm::Tags::update_coeff())->link_to(update_coeff);
    update_coeff.parallelize();

    Field& jacob_det = solution_space.create_field(sdm::Tags::jacob_det(), "jacob_det[1]");
    solver().field_manager().create_component<Link>(sdm::Tags::jacob_det())->link_to(jacob_det);

    Field& delta = solution_space.create_field(sdm::Tags::delta(), "delta[vector]");
    solver().field_manager().create_component<Link>(sdm::Tags::delta())->link_to(delta);

    boost_foreach(const Handle<Entities>& elements, solution_space.entities_range())
    {
      if ( is_null(elements->handle<Cells>()) ) continue;
      const Space& space = solution_space.space(*elements);

      const RealMatrix& local_coords = space.shape_function().local_coordinates();

      RealMatrix geometry_coords;
      elements->geometry_space().allocate_coordinates(geometry_coords);

      RealVector dKsi (elements->element_type().dimensionality()); dKsi.setConstant(2.);
      RealVector dX (elements->element_type().dimension());
      RealMatrix jacobian(elements->element_type().dimensionality(),elements->element_type().dimension());

      const Connectivity& field_connectivity = space.connectivity();

      for (Uint elem=0; elem<elements->size(); ++elem)
      {
        elements->geometry_space().put_coordinates(geometry_coords,elem);

        for (Uint node=0; node<local_coords.rows();++node)
        {
          const Uint p = field_connectivity[elem][node];
          jacob_det[p][0]=elements->element_type().jacobian_determinant(local_coords.row(node),geometry_coords);
          if (jacob_det[p][0] < 0)
            throw BadValue(FromHere(), "jacobian determinant is negative ("+to_str(jacob_det[p][0])+") in cell "+elements->uri().string()+"["+to_str(elem)+"] (glbidx="+to_str(+elements->glb_idx()[elem])+"). This is caused by a faulty node ordering in the mesh.");
          elements->element_type().compute_jacobian(local_coords.row(node),geometry_coords,jacobian);
          dX.noalias() = jacobian.transpose()*dKsi;
          for (Uint d=0; d<dX.size(); ++d)
            delta[p][d]=dX[d];
        }

      }
    }
  }


//  if ( is_not_null (find_component_ptr_recursively_with_tag<Dictionary>(mesh(),boundary_space_name)))
//  {
//    CFinfo << "field group ["<<boundary_space_name<<"] already exists, check now to create the fields" << CFendl;
//  }
//  else
//  {
//    Dictionary& boundary_space = mesh().create_space_and_dict(solution_space_name,Dictionary::Basis::FACE_BASED,"cf3.sdm.P"+to_str(solution_order-1));
//    boundary_space.add_tag(boundary_space_name);

//    Component& solution_vars = find_component_with_tag(physical_model(),sdm::Tags::solution_vars());
//    Field& solution   = boundary_space.create_field(sdm::Tags::solution(), solution_vars.handle<Variables>()->description().description() );
//  }

}

//////////////////////////////////////////////////////////////////////////////

} // sdm
} // cf3
