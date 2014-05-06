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

#include "mesh/Faces.hpp"
#include "mesh/Connectivity.hpp"
#include "mesh/GeneratePlane3D.hpp"
#include "mesh/MergedParallelDistribution.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/MeshElements.hpp"
#include "mesh/ParallelDistribution.hpp"
#include "mesh/Region.hpp"
#include "mesh/Space.hpp"

#include <Eigen/Geometry>

namespace cf3 {
namespace mesh {

common::ComponentBuilder < GeneratePlane3D, mesh::MeshGenerator, LibMesh > GeneratePlane3D_builder;
  
////////////////////////////////////////////////////////////////////////////////

GeneratePlane3D::GeneratePlane3D ( const std::string& name  ) :
  MeshGenerator ( name )
{
  options().add("origin", std::vector<Real>())
    .pretty_name("Origin")
    .description("Corner point of the plane")
    .mark_basic();
  
  options().add("normal", std::vector<Real>())
    .pretty_name("Normal")
    .description("Normal vector to the plane")
    .mark_basic();
    
  options().add("size", std::vector<Real>(2, 1.))
    .pretty_name("Size")
    .description("Size in the plane-local X and Y direction")
    .mark_basic();
  
  options().add("segments", std::vector<Uint>(2,4))
    .pretty_name("Segments")
    .description("Number of segments in the plane X and Y direction")
    .mark_basic();
}

GeneratePlane3D::~GeneratePlane3D()
{
}

void GeneratePlane3D::execute()
{
  // From SimpleMeshGenerator::create_plane
  Mesh& mesh = *m_mesh;
  
  const std::vector<Uint> segments = options().value< std::vector<Uint> >("segments");
  if(segments.size() != 2)
    throw common::SetupError(FromHere(), "Segments must be given for exactly two directions in the option segments");
  
  const std::vector<Real> size = options().value< std::vector<Real> >("size");
  if(size.size() != 2)
    throw common::SetupError(FromHere(), "Sizes must be given for exactly two directions in the option size");
  
  const std::vector<Real> origin = options().value< std::vector<Real> >("origin");
  if(origin.size() != 3)
    throw common::SetupError(FromHere(), "origin size of " + common::to_str(origin.size()) + " is invalid, must be 3");
  
  const std::vector<Real> normal = options().value< std::vector<Real> >("normal");
  if(normal.size() != 3)
    throw common::SetupError(FromHere(), "normal size of " + common::to_str(normal.size()) + " is invalid, must be 3");
  
  const Uint x_segments = segments[XX];
  const Uint y_segments = segments[YY];
  const Real x_len = 1.;
  const Real y_len = 1.;
  const Real x_offset = 0.;
  const Real y_offset = 0.;
  const Uint nb_parts = common::PE::Comm::instance().size();
  const Uint coord_dim = 3;

  const Uint part = common::PE::Comm::instance().rank();
  enum HashType { NODES=0, ELEMS=1 };
  // Create a hash
  boost::shared_ptr<MergedParallelDistribution> tmp_hash = common::allocate_component<MergedParallelDistribution>("tmp_hash");
  MergedParallelDistribution& hash = *tmp_hash;
  std::vector<Uint> num_obj(2);
  num_obj[NODES] = (x_segments+1)*(y_segments+1);
  num_obj[ELEMS] = x_segments*y_segments;
  hash.options().set("nb_obj",num_obj);
  hash.options().set("nb_parts",nb_parts);

  Region& region = mesh.topology().create_region("interior");
  Dictionary& nodes = mesh.geometry_fields();


  // find ghost nodes
  std::map<Uint,Uint> ghost_nodes_loc;
  Uint glb_node_idx;
  for(Uint j = 0; j < y_segments; ++j)
  {
    for(Uint i = 0; i < x_segments; ++i)
    {
      if (hash.subhash(ELEMS).part_owns(part,i+j*x_segments))
      {
        glb_node_idx = j * (x_segments+1) + i;
        if (hash.subhash(NODES).part_owns(part,glb_node_idx) == false)
        {
          ghost_nodes_loc[glb_node_idx]=0; // this value will be set further
        }

        glb_node_idx = j * (x_segments+1) + (i+1);
        if (hash.subhash(NODES).part_owns(part,glb_node_idx) == false)
        {
          ghost_nodes_loc[glb_node_idx]=0; // this value will be set further
        }

        glb_node_idx = (j+1) * (x_segments+1) + i;
        if (hash.subhash(NODES).part_owns(part,glb_node_idx) == false)
        {
          ghost_nodes_loc[glb_node_idx]=0; // this value will be set further
        }

        glb_node_idx = (j+1) * (x_segments+1) + (i+1);
        if (hash.subhash(NODES).part_owns(part,glb_node_idx) == false)
        {
          ghost_nodes_loc[glb_node_idx]=0; // this value will be set further
        }
      }
    }
  }

  mesh.initialize_nodes(hash.subhash(NODES).nb_objects_in_part(part)+ghost_nodes_loc.size(), DIM_3D);
  Uint glb_node_start_idx = hash.subhash(NODES).start_idx_in_part(part);

  const Real x_step = x_len / static_cast<Real>(x_segments);
  const Real y_step = y_len / static_cast<Real>(y_segments);
  Real y;
  for(Uint j = 0; j <= y_segments; ++j)
  {
    y = static_cast<Real>(j) * y_step;
    for(Uint i = 0; i <= x_segments; ++i)
    {
      glb_node_idx = j * (x_segments+1) + i;

      if (hash.subhash(NODES).part_owns(part,glb_node_idx))
      {
        cf3_assert(glb_node_idx-glb_node_start_idx < nodes.size());
        common::Table<Real>::Row row = nodes.coordinates()[glb_node_idx-glb_node_start_idx];
        for (Uint d=0; d<coord_dim; ++d)
          row[d]=0.;
        row[XX] = static_cast<Real>(i) * x_step + x_offset;
        row[YY] = y + y_offset;
        nodes.rank()[glb_node_idx-glb_node_start_idx]=part;
        nodes.glb_idx()[glb_node_idx-glb_node_start_idx]=glb_node_idx;
      }
    }
  }

  // add ghost nodes
  Uint glb_ghost_node_start_idx = hash.subhash(NODES).nb_objects_in_part(part);
  Uint loc_node_idx = glb_ghost_node_start_idx;
  cf3_assert(glb_ghost_node_start_idx <= nodes.size());
  foreach_container((const Uint glb_ghost_node_idx) (Uint& loc_ghost_node_idx),ghost_nodes_loc)
  {
    Uint j = glb_ghost_node_idx / (x_segments+1);
    Uint i = glb_ghost_node_idx - j*(x_segments+1);
    loc_ghost_node_idx = loc_node_idx++;
    cf3_assert(loc_ghost_node_idx < nodes.size());
    common::Table<Real>::Row row = nodes.coordinates()[loc_ghost_node_idx];
    for (Uint d=0; d<coord_dim; ++d)
      row[d]=0.;
    row[XX] = static_cast<Real>(i) * x_step + x_offset;
    row[YY] = static_cast<Real>(j) * y_step + y_offset;
    nodes.rank()[loc_ghost_node_idx]=hash.subhash(NODES).proc_of_obj(glb_ghost_node_idx);
    nodes.glb_idx()[loc_ghost_node_idx]=glb_ghost_node_idx;
  }
  Handle<Faces> faces = region.create_component<Faces>("Quad");
  faces->initialize("cf3.mesh.LagrangeP1.Quad3D",nodes);

  faces->resize(hash.subhash(ELEMS).nb_objects_in_part(part));
  Connectivity& connectivity = faces->geometry_space().connectivity();
  common::List<Uint>& elem_rank = faces->rank();
  common::List<Uint>& elem_glb_idx = faces->glb_idx();

  Uint glb_elem_start_idx = hash.subhash(ELEMS).start_idx_in_part(part);
  Uint glb_elem_idx;
  for(Uint j = 0; j < y_segments; ++j)
  {
    for(Uint i = 0; i < x_segments; ++i)
    {
      glb_elem_idx = j * x_segments + i;

      if (hash.subhash(ELEMS).part_owns(part,glb_elem_idx))
      {
        Connectivity::Row nodes = connectivity[glb_elem_idx-glb_elem_start_idx];
        elem_rank[glb_elem_idx-glb_elem_start_idx] = part;
        elem_glb_idx[glb_elem_idx-glb_elem_start_idx] = glb_elem_idx;

        glb_node_idx = j * (x_segments+1) + i;
        if (hash.subhash(NODES).part_owns(part,glb_node_idx) == false)
          nodes[0] = ghost_nodes_loc[glb_node_idx];
        else
          nodes[0] = glb_node_idx-glb_node_start_idx;

        glb_node_idx = j * (x_segments+1) + (i+1);
        if (hash.subhash(NODES).part_owns(part,glb_node_idx) == false)
          nodes[1] = ghost_nodes_loc[glb_node_idx];
        else
          nodes[1] = glb_node_idx-glb_node_start_idx;

        glb_node_idx = (j+1) * (x_segments+1) + i;
        if (hash.subhash(NODES).part_owns(part,glb_node_idx) == false)
          nodes[3] = ghost_nodes_loc[glb_node_idx];
        else
          nodes[3] = glb_node_idx-glb_node_start_idx;

        glb_node_idx = (j+1) * (x_segments+1) + (i+1);
        if (hash.subhash(NODES).part_owns(part,glb_node_idx) == false)
          nodes[2] = ghost_nodes_loc[glb_node_idx];
        else
          nodes[2] = glb_node_idx-glb_node_start_idx;
      }
    }
  }

  mesh.update_structures();
  
  // Transform the generated plane to the requested position, size and orientation
  const RealVector3 standard_normal(0., 0., 1.);
  RealVector3 target_normal(normal.data());
  target_normal.normalize();
  
  const Real angle = ::acos(standard_normal.dot(target_normal));
  const RealVector3 axis = ::fabs(angle) > 1e-10 ? target_normal.cross(standard_normal).normalized() : standard_normal;
  
  const RealMatrix3 transformation_matrix = (Eigen::AngleAxis<Real>(angle, axis).toRotationMatrix() * Eigen::DiagonalMatrix<Real, 3>(size[0], size[1], 1.)).transpose();
  const RealVector3 offset(origin.data());
  
  mesh::Field::ArrayT& coords = nodes.coordinates().array();
  const Uint nb_nodes = nodes.coordinates().size();
  for(Uint i = 0; i != nb_nodes; ++i)
  {
    Eigen::Map<RealRowVector3> point(&coords[i][0]);
    point *= transformation_matrix;
    point += offset;
  }
}


////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3
