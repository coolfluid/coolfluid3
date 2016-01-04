// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <set>

#include "common/Builder.hpp"

#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"
#include "common/Option.hpp"
#include "common/OptionList.hpp"
#include "common/List.hpp"

#include "math/VariablesDescriptor.hpp"

#include "mesh/ConnectivityData.hpp"
#include "mesh/DiscontinuousDictionary.hpp"
#include "mesh/ElementData.hpp"
#include "mesh/Faces.hpp"
#include "mesh/Region.hpp"
#include "mesh/Space.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Field.hpp"
#include "mesh/Functions.hpp"
#include "mesh/Connectivity.hpp"
#include "mesh/ElementData.hpp"

#include "mesh/LagrangeP1/Triag2D.hpp"
#include "mesh/LagrangeP1/Quad2D.hpp"

#include "physics/PhysModel.hpp"

#include "YPlus.hpp"

using namespace cf3::mesh;

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace solver {
namespace actions {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < YPlus, common::Action, solver::actions::LibActions> YPlus_Builder;

////////////////////////////////////////////////////////////////////////////////

YPlus::YPlus(const std::string& name) : solver::Action(name)
{
  options().add("velocity_tag", std::string("navier_stokes_solution"))
    .pretty_name("Velocity Tag")
    .description("Tag for the velocity field")
    .mark_basic();
}

void YPlus::execute()
{
  Mesh& mesh = this->mesh();

  // Geometry data
  const Field& coords = mesh.geometry_fields().coordinates();
  const Uint nb_nodes = coords.size();
  const Uint dim = coords.row_size();

  // Velocity data
  const Field& velocity_field = common::find_component_recursively_with_tag<Field>(mesh, options().value<std::string>("velocity_tag"));
  const auto velocity_dict_handle = Handle<Dictionary const>(velocity_field.parent());
  cf3_assert(velocity_dict_handle != nullptr);
  const Dictionary& velocity_dict = *velocity_dict_handle;
  const Uint vel_offset = velocity_field.descriptor().offset("Velocity");

  // initialize if needed
  auto volume_node_connectivity = Handle<NodeConnectivity>(mesh.get_child("volume_node_connectivity"));
  if(volume_node_connectivity == nullptr)
  {
    // Node-to-element connectivity for the volume elements
    volume_node_connectivity = mesh.create_component<NodeConnectivity>("volume_node_connectivity");
    std::vector< Handle<Entities const> > volume_entities;
    for(const mesh::Elements& elements : common::find_components_recursively_with_filter<mesh::Elements>(mesh, IsElementsVolume()))
    {
      volume_entities.push_back(elements.handle<Entities const>());
    }
    volume_node_connectivity->initialize(nb_nodes, volume_entities);

    // Create the field for the wall shear stress
    Handle<Dictionary>(mesh.get_child_checked("wall_P0"))->create_field("wall_velocity_gradient").add_tag("wall_velocity_gradient");
    mesh.geometry_fields().create_field("wall_velocity_gradient_nodal").add_tag("wall_velocity_gradient_nodal");

    for(const Handle<Region>& region : regions())
    {
      for(mesh::Elements& elements : common::find_components_recursively_with_filter<mesh::Elements>(*region, IsElementsSurface()))
      {
        elements.create_component<FaceConnectivity>("wall_face_connectivity")->initialize(*volume_node_connectivity);
      }
    }

    // Create the y+ field in the geometry dictionary
    if(common::find_component_ptr_with_tag(mesh.geometry_fields(), "yplus") == nullptr)
    {
      mesh.geometry_fields().create_field("yplus").add_tag("yplus");
    }
  }

  // Compute shear stress
  Dictionary& wall_P0 = *Handle<mesh::Dictionary>(mesh.get_child_checked("wall_P0"));
  Field& wall_velocity_gradient_field = *Handle<Field>(wall_P0.get_child_checked("wall_velocity_gradient"));
  const Field& normals = *Handle<Field>(wall_P0.get_child_checked(mesh::Tags::normal()));
  for(const Handle<Region>& region : regions())
  {
    for(const mesh::Elements& elements : common::find_components_recursively_with_filter<mesh::Elements>(*region, IsElementsSurface()))
    {
      const Uint nb_elements = elements.geometry_space().connectivity().size();
      cf3_assert(elements.element_type().nb_faces() == 1);
      const auto& face_connectivity = *Handle<FaceConnectivity const>(elements.get_child_checked("wall_face_connectivity"));
      const auto& wall_conn = elements.space(wall_P0).connectivity();
      for(Uint surface_elm_idx = 0; surface_elm_idx != nb_elements; ++surface_elm_idx)
      {
        if(face_connectivity.has_adjacent_element(surface_elm_idx, 0))
        {
          // Get the wall normal vector
          const Uint wall_field_idx = wall_conn[surface_elm_idx][0];
          const Eigen::Map<RealVector const> normal(&normals[wall_field_idx][0], dim);
          RealVector3 normal3;
          normal3[0] = normal[0];
          normal3[1] = normal[1];
          normal3[2] = dim == 3 ? normal[2] : 0.;

          // The connected volume element
          NodeConnectivity::ElementReferenceT connected = face_connectivity.adjacent_element(surface_elm_idx, 0);
          const mesh::Entities& volume_entities = *face_connectivity.node_connectivity().entities()[connected.first];
          const Uint volume_elem_idx = connected.second;
          const auto& velocity_conn = volume_entities.space(velocity_dict).connectivity();
          const auto& velocity_sf = volume_entities.space(velocity_dict).shape_function();
          const auto& geom_conn = volume_entities.geometry_space().connectivity();
          const ElementType& volume_etype = volume_entities.element_type();

          const Uint nb_vel_nodes = velocity_sf.nb_nodes();
          const RealVector centroid_mapped_coord = 0.5*(velocity_sf.local_coordinates().colwise().minCoeff() + velocity_sf.local_coordinates().colwise().maxCoeff());
          RealMatrix elem_coords(geom_conn.row_size(), dim);
          fill(elem_coords, coords, geom_conn[volume_elem_idx]);

          RealVector tangential_velocity(nb_vel_nodes); // For every node, the component of the velocity tangential to the wall
          RealVector3 v3;
          for(Uint i = 0; i != nb_vel_nodes; ++i)
          {
            Eigen::Map<RealVector const> v(&velocity_field[velocity_conn[volume_elem_idx][i]][vel_offset], dim);
            v3[0] = v[0];
            v3[1] = v[1];
            v3[2] = dim == 3 ? v[2] : 0.;
            tangential_velocity[i] = v3.cross(normal3).norm();
          }
          wall_velocity_gradient_field[wall_field_idx][0] = fabs((volume_etype.jacobian(centroid_mapped_coord, elem_coords).inverse() * velocity_sf.gradient(centroid_mapped_coord) * tangential_velocity).dot(-normal));
        }
      }
    }
  }

  wall_velocity_gradient_field.synchronize();

  // Compute a nodal version of the wall velocity gradient
  Field& wall_velocity_gradient_field_nodal = *Handle<Field>(mesh.geometry_fields().get_child_checked("wall_velocity_gradient_nodal"));
  const auto& wall_node_connectivity = *Handle<NodeConnectivity>(mesh.get_child_checked("wall_node_connectivity"));
  for(Uint node_idx = 0; node_idx != nb_nodes; ++node_idx)
  {
    Uint nb_connected_elems = 0;
    wall_velocity_gradient_field_nodal[node_idx][0] = 0;
    for(const NodeConnectivity::ElementReferenceT elref : wall_node_connectivity.node_element_range(node_idx))
    {
      const Uint wall_field_idx = wall_node_connectivity.entities()[elref.first]->space(wall_P0).connectivity()[elref.second][0];
      wall_velocity_gradient_field_nodal[node_idx][0] += wall_velocity_gradient_field[wall_field_idx][0];
      ++nb_connected_elems;
    }
    if(nb_connected_elems != 0)
    {
      wall_velocity_gradient_field_nodal[node_idx][0] /= static_cast<Real>(nb_connected_elems);
    }
  }


  // Set Yplus
  Field& yplus_field = *Handle<Field>(mesh.geometry_fields().get_child_checked("yplus"));
  const Field& wall_distance_field = *Handle<Field>(mesh.geometry_fields().get_child_checked("wall_distance"));
  const auto& node_to_wall_element = *Handle<common::Table<Uint>>(mesh.get_child_checked("node_to_wall_element"));
  const Real nu = physical_model().options().value<Real>("kinematic_viscosity");
  for(Uint node_idx = 0; node_idx != nb_nodes; ++node_idx)
  {
    yplus_field[node_idx][0] = 0;
    if(node_to_wall_element[node_idx][0] != 0)
    {
      const Entities& wall_entities = *wall_node_connectivity.entities()[node_to_wall_element[node_idx][1]];
      const Uint wall_field_idx = wall_entities.space(wall_P0).connectivity()[node_to_wall_element[node_idx][2]][0];
      yplus_field[node_idx][0] = wall_distance_field[node_idx][0] * sqrt(nu*wall_velocity_gradient_field[wall_field_idx][0]) / nu;
    }
    else
    {
      yplus_field[node_idx][0] = wall_distance_field[node_idx][0] * sqrt(nu*wall_velocity_gradient_field_nodal[node_to_wall_element[node_idx][1]][0]) / nu;
    }
  }
}

//////////////////////////////////////////////////////////////////////////////


} // actions
} // solver
} // cf3
