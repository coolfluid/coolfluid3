// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"
#include "common/OptionURI.hpp"
#include "common/Core.hpp"
#include "common/EventHandler.hpp"
#include "common/OptionList.hpp"

#include "common/XML/SignalOptions.hpp"
#include "common/PE/Comm.hpp"

#include "mesh/Edges.hpp"
#include "mesh/Connectivity.hpp"
#include "mesh/GenerateLine3D.hpp"
#include "mesh/MergedParallelDistribution.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/MeshElements.hpp"
#include "mesh/ParallelDistribution.hpp"
#include "mesh/Region.hpp"
#include "mesh/Space.hpp"

#include <Eigen/Geometry>

namespace cf3 {
namespace mesh {

common::ComponentBuilder < GenerateLine3D, mesh::MeshGenerator, LibMesh > GenerateLine3D_builder;
  
////////////////////////////////////////////////////////////////////////////////

GenerateLine3D::GenerateLine3D ( const std::string& name  ) :
  MeshGenerator ( name )
{
  options().add("origin", std::vector<Real>())
    .pretty_name("Origin")
    .description("Starting point of the line")
    .mark_basic();
  
  options().add("end", std::vector<Real>())
    .pretty_name("End")
    .description("End point of the line")
    .mark_basic();
  
  options().add("segments", 1u)
    .pretty_name("Segments")
    .description("Number of segments in the line")
    .mark_basic();
}

GenerateLine3D::~GenerateLine3D()
{
}

void GenerateLine3D::execute()
{
  // From SimpleMeshGenerator::create_plane
  Mesh& mesh = *m_mesh;
  
  const Uint segments = options().value<Uint>("segments");
  if(segments == 0)
    throw common::SetupError(FromHere(), "segments must be greater than 0");

  const std::vector<Real> origin = options().value< std::vector<Real> >("origin");
  if(origin.size() != 3)
    throw common::SetupError(FromHere(), "origin size of " + common::to_str(origin.size()) + " is invalid, must be 3");
  
  const std::vector<Real> end = options().value< std::vector<Real> >("end");
  if(end.size() != 3)
    throw common::SetupError(FromHere(), "end size of " + common::to_str(end.size()) + " is invalid, must be 3");
  
  const RealVector3 start_point(origin.data());
  const RealVector3 end_point(end.data());
  const RealVector3 step = (end_point - start_point) / static_cast<Real>(segments);
  const Uint nb_parts = common::PE::Comm::instance().size();
  const Uint coord_dim = 3;

  const Uint part = common::PE::Comm::instance().rank();
  enum HashType { NODES=0, ELEMS=1 };
  // Create a hash
  boost::shared_ptr<MergedParallelDistribution> tmp_hash = common::allocate_component<MergedParallelDistribution>("tmp_hash");
  MergedParallelDistribution& hash = *tmp_hash;
  std::vector<Uint> num_obj(2);
  num_obj[NODES] = segments+1;
  num_obj[ELEMS] = segments;
  hash.options().set("nb_obj",num_obj);
  hash.options().set("nb_parts",nb_parts);

  Region& region = mesh.topology().create_region("interior");
  Dictionary& nodes = mesh.geometry_fields();


  // find ghost nodes
  std::map<Uint,Uint> ghost_nodes_loc;
  Uint glb_node_idx;
  for(Uint i = 0; i < segments; ++i)
  {
    if (hash.subhash(ELEMS).part_owns(part,i))
    {
      glb_node_idx = i;
      if (hash.subhash(NODES).part_owns(part,glb_node_idx) == false)
      {
        ghost_nodes_loc[glb_node_idx]=0; // this value will be set further
      }

      glb_node_idx = (i+1);
      if (hash.subhash(NODES).part_owns(part,glb_node_idx) == false)
      {
        ghost_nodes_loc[glb_node_idx]=0; // this value will be set further
      }
    }
  }

  mesh.initialize_nodes(hash.subhash(NODES).nb_objects_in_part(part)+ghost_nodes_loc.size(), DIM_3D);
  Uint glb_node_start_idx = hash.subhash(NODES).start_idx_in_part(part);

  for(Uint i = 0; i <= segments; ++i)
  {
    glb_node_idx = i;

    if (hash.subhash(NODES).part_owns(part,glb_node_idx))
    {
      cf3_assert(glb_node_idx-glb_node_start_idx < nodes.size());

      Eigen::Map<RealVector3> row(&(nodes.coordinates()[glb_node_idx-glb_node_start_idx][0]));
      row = static_cast<Real>(i) * step + start_point;

      nodes.rank()[glb_node_idx-glb_node_start_idx]=part;
      nodes.glb_idx()[glb_node_idx-glb_node_start_idx]=glb_node_idx;
    }
  }

  // add ghost nodes
  Uint glb_ghost_node_start_idx = hash.subhash(NODES).nb_objects_in_part(part);
  Uint loc_node_idx = glb_ghost_node_start_idx;
  cf3_assert(glb_ghost_node_start_idx <= nodes.size());
  foreach_container((const Uint glb_ghost_node_idx) (Uint& loc_ghost_node_idx),ghost_nodes_loc)
  {
    Uint i = glb_ghost_node_idx;
    loc_ghost_node_idx = loc_node_idx++;
    cf3_assert(loc_ghost_node_idx < nodes.size());

    Eigen::Map<RealVector3> row(&(nodes.coordinates()[loc_ghost_node_idx][0]));
    row = static_cast<Real>(i) * step + start_point;

    nodes.rank()[loc_ghost_node_idx]=hash.subhash(NODES).proc_of_obj(glb_ghost_node_idx);
    nodes.glb_idx()[loc_ghost_node_idx]=glb_ghost_node_idx;
  }
  Handle<Edges> edges = region.create_component<Edges>("Line");
  edges->initialize("cf3.mesh.LagrangeP1.Line3D",nodes);

  edges->resize(hash.subhash(ELEMS).nb_objects_in_part(part));
  Connectivity& connectivity = edges->geometry_space().connectivity();
  common::List<Uint>& elem_rank = edges->rank();
  common::List<Uint>& elem_glb_idx = edges->glb_idx();

  Uint glb_elem_start_idx = hash.subhash(ELEMS).start_idx_in_part(part);
  Uint glb_elem_idx;
  for(Uint i = 0; i < segments; ++i)
  {
    glb_elem_idx = i;

    if (hash.subhash(ELEMS).part_owns(part,glb_elem_idx))
    {
      Connectivity::Row nodes = connectivity[glb_elem_idx-glb_elem_start_idx];
      elem_rank[glb_elem_idx-glb_elem_start_idx] = part;
      elem_glb_idx[glb_elem_idx-glb_elem_start_idx] = glb_elem_idx;

      glb_node_idx = i;
      if (hash.subhash(NODES).part_owns(part,glb_node_idx) == false)
        nodes[0] = ghost_nodes_loc[glb_node_idx];
      else
        nodes[0] = glb_node_idx-glb_node_start_idx;

      glb_node_idx = (i+1);
      if (hash.subhash(NODES).part_owns(part,glb_node_idx) == false)
        nodes[1] = ghost_nodes_loc[glb_node_idx];
      else
        nodes[1] = glb_node_idx-glb_node_start_idx;
    }
  }

  mesh.update_structures();
}


////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3
