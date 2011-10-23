// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/list_of.hpp>

#include "common/Builder.hpp"
#include "common/Core.hpp"
#include "common/Exception.hpp"
#include "common/EventHandler.hpp"
#include "common/Log.hpp"
#include "common/OptionURI.hpp"
#include "common/XML/SignalFrame.hpp"
#include "common/XML/SignalOptions.hpp"
#include "common/Timer.hpp"

#include "common/PE/Comm.hpp"

#include "mesh/BlockMesh/BlockData.hpp"
#include "mesh/BlockMesh/WriteDict.hpp"

#include "mesh/Cells.hpp"
#include "common/Table.hpp"
#include "mesh/Elements.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/MeshElements.hpp"
#include "mesh/MeshTransformer.hpp"
#include "mesh/Geometry.hpp"
#include "mesh/ConnectivityData.hpp"
#include "mesh/Region.hpp"
#include "mesh/ElementData.hpp"

#include "mesh/LagrangeP1/Hexa3D.hpp"
#include "mesh/LagrangeP1/Line1D.hpp"
#include "mesh/LagrangeP1/Quad2D.hpp"

namespace cf3 {
namespace mesh {
namespace BlockMesh {

using namespace cf3::common;
using namespace cf3::mesh;
using namespace cf3::mesh::LagrangeP1;

ComponentBuilder < BlockData, Component, LibBlockMesh > BlockData_Builder;

BlockData::BlockData(const std::string& name): Component(name)
{
}


bool BlockData::operator==(const BlockData& other) const
{
  return dimension == other.dimension &&
         block_distribution == other.block_distribution &&
         block_gradings == other.block_gradings &&
         block_points == other.block_points &&
         block_subdivisions == other.block_subdivisions &&
         patch_names == other.patch_names &&
         patch_points == other.patch_points &&
         patch_types == other.patch_types &&
         points == other.points &&
         scaling_factor == other.scaling_factor;
}

void BlockData::copy_to(BlockData& other) const
{
  other.scaling_factor = scaling_factor;
  other.dimension = dimension;
  other.block_distribution = block_distribution;
  other.block_gradings = block_gradings;
  other.block_points = block_points;
  other.block_subdivisions = block_subdivisions;
  other.patch_names = patch_names;
  other.patch_points = patch_points;
  other.patch_types = patch_types;
  other.points = points;
}



/// Some helper functions for mesh building
namespace detail {

/// Creates a mesh containing only the blocks
void create_block_mesh_3d(const BlockData& block_data, Mesh& mesh, std::map<std::string, std::string>& patch_types)
{
  const Uint nb_nodes = block_data.points.size();

  // root region and coordinates
  Region& block_mesh_region = mesh.topology().create_region("block_mesh_region");
  mesh.initialize_nodes(nb_nodes, static_cast<Uint>(DIM_3D));
  Geometry& block_nodes = mesh.geometry();

  // Fill the coordinates array
  common::Table<Real>::ArrayT& coords_array = block_nodes.coordinates().array();
  coords_array.resize(boost::extents[nb_nodes][3]);
  for(Uint node_idx = 0; node_idx != nb_nodes; ++node_idx)
  {
    const BlockData::PointT& point = block_data.points[node_idx];
    coords_array[node_idx][XX] = point[XX];
    coords_array[node_idx][YY] = point[YY];
    coords_array[node_idx][ZZ] = point[ZZ];
  }

  // Define the volume cells, i.e. the blocks
  Cells& block_elements = block_mesh_region.create_region("blocks").create_component<Cells>("interior");
  block_elements.initialize("cf3.mesh.LagrangeP1.Hexa3D", block_nodes);
  common::Table<Uint>::ArrayT& block_connectivity = block_elements.node_connectivity().array();
  const Uint nb_blocks = block_data.block_points.size();
  block_connectivity.resize(boost::extents[nb_blocks][8]);
  for(Uint block_idx = 0; block_idx != nb_blocks; ++block_idx)
  {
    const BlockData::IndicesT& block = block_data.block_points[block_idx];
    std::copy(block.begin(), block.end(), block_connectivity[block_idx].begin());
  }

  // Define the surface patches
  const Uint nb_patches = block_data.patch_names.size();
  for(Uint patch_idx = 0 ; patch_idx != nb_patches; ++patch_idx)
  {
    Elements& patch_elements = block_mesh_region.create_region(block_data.patch_names[patch_idx]).create_elements("cf3.mesh.LagrangeP1.Quad3D", block_nodes);
    patch_types[block_data.patch_names[patch_idx]] = block_data.patch_types[patch_idx];
    common::Table<Uint>::ArrayT& patch_connectivity = patch_elements.node_connectivity().array();
    const BlockData::IndicesT patch_points = block_data.patch_points[patch_idx];
    const Uint nb_patch_elements = patch_points.size() / 4;
    patch_connectivity.resize(boost::extents[nb_patch_elements][4]);
    for(Uint patch_element_idx = 0; patch_element_idx != nb_patch_elements; ++patch_element_idx)
    {
      std::copy(patch_points.begin() + 4*patch_element_idx, patch_points.begin() + 4*patch_element_idx + 4, patch_connectivity[patch_element_idx].begin());
    }
  }

  // Create connectivity data
  CNodeConnectivity::Ptr node_connectivity = block_mesh_region.create_component_ptr<CNodeConnectivity>("node_connectivity");
  node_connectivity->initialize(find_components_recursively<Elements>(block_mesh_region));
  BOOST_FOREACH(Elements& celements, find_components_recursively<Elements>(block_mesh_region))
  {
    celements.create_component_ptr<CFaceConnectivity>("face_connectivity")->initialize(*node_connectivity);
  }
}

/// Creates a mesh containing only the blocks
void create_block_mesh_2d(const BlockData& block_data, Mesh& mesh, std::map<std::string, std::string>& patch_types)
{
  cf3_assert(block_data.dimension == 2);

  const Uint nb_nodes = block_data.points.size();

  // root region and coordinates
  Region& block_mesh_region = mesh.topology().create_region("block_mesh_region");
  mesh.initialize_nodes(nb_nodes, block_data.dimension);
  Geometry& block_nodes = mesh.geometry();

  // Fill the coordinates array
  common::Table<Real>::ArrayT& coords_array = block_nodes.coordinates().array();
  coords_array.resize(boost::extents[nb_nodes][block_data.dimension]);
  for(Uint node_idx = 0; node_idx != nb_nodes; ++node_idx)
  {
    const BlockData::PointT& point = block_data.points[node_idx];
    coords_array[node_idx][XX] = point[XX];
    coords_array[node_idx][YY] = point[YY];
  }

  // Define the volume cells, i.e. the blocks
  Cells& block_elements = block_mesh_region.create_region("blocks").create_component<Cells>("interior");
  block_elements.initialize("cf3.mesh.LagrangeP1.Quad2D", block_nodes);
  common::Table<Uint>::ArrayT& block_connectivity = block_elements.node_connectivity().array();
  const Uint nb_blocks = block_data.block_points.size();
  block_connectivity.resize(boost::extents[nb_blocks][4]);
  for(Uint block_idx = 0; block_idx != nb_blocks; ++block_idx)
  {
    const BlockData::IndicesT& block = block_data.block_points[block_idx];
    std::copy(block.begin(), block.end(), block_connectivity[block_idx].begin());
  }

  // Define the surface patches
  const Uint nb_patches = block_data.patch_names.size();
  for(Uint patch_idx = 0 ; patch_idx != nb_patches; ++patch_idx)
  {
    Elements& patch_elements = block_mesh_region.create_region(block_data.patch_names[patch_idx]).create_elements("cf3.mesh.LagrangeP1.Line2D", block_nodes);
    patch_types[block_data.patch_names[patch_idx]] = block_data.patch_types[patch_idx];
    common::Table<Uint>::ArrayT& patch_connectivity = patch_elements.node_connectivity().array();
    const BlockData::IndicesT patch_points = block_data.patch_points[patch_idx];
    const Uint nb_patch_elements = patch_points.size() / 2;
    patch_connectivity.resize(boost::extents[nb_patch_elements][2]);
    for(Uint patch_element_idx = 0; patch_element_idx != nb_patch_elements; ++patch_element_idx)
    {
      std::copy(patch_points.begin() + 2*patch_element_idx, patch_points.begin() + 2*patch_element_idx + 2, patch_connectivity[patch_element_idx].begin());
    }
  }

  // Create connectivity data
  CNodeConnectivity::Ptr node_connectivity = block_mesh_region.create_component_ptr<CNodeConnectivity>("node_connectivity");
  node_connectivity->initialize(find_components_recursively<Elements>(block_mesh_region));
  BOOST_FOREACH(Elements& celements, find_components_recursively<Elements>(block_mesh_region))
  {
    celements.create_component_ptr<CFaceConnectivity>("face_connectivity")->initialize(*node_connectivity);
  }
}

/// looks up node indices based on structured block indices
struct NodeIndices3D
{
  typedef std::vector<Uint> IndicesT;
  typedef std::vector<Uint> CountsT;
  typedef boost::multi_array<bool, 2> Bools2T;

  // Intersection of planes
  enum Bounds { XY = 3, XZ = 4, YZ = 5, XYZ = 6 };

  NodeIndices3D(const CFaceConnectivity& face_connectivity, const BlockData& block_data, const Uint rank, const Uint nb_procs) :
    m_face_connectivity(face_connectivity),
    m_block_data(block_data),
    m_rank(rank),
    m_nb_procs(nb_procs),
    ghost_counter(0)
  {
    const Uint nb_blocks = m_block_data.block_subdivisions.size();
    bounded.resize(boost::extents[nb_blocks][7]);
    block_first_nodes.reserve(nb_blocks + 1);
    block_first_nodes.push_back(0);
    for(Uint block = 0; block != nb_blocks; ++block)
    {
      const BlockData::CountsT& segments = m_block_data.block_subdivisions[block];
      const Uint x_segs = segments[XX];
      const Uint y_segs = segments[YY];
      const Uint z_segs = segments[ZZ];

      const Uint XPOS = Hexa::KSI_POS;
      const Uint YPOS = Hexa::ETA_POS;
      const Uint ZPOS = Hexa::ZTA_POS;

      bounded[block][XX] = m_face_connectivity.adjacent_element(block, XPOS).first->element_type().dimensionality() == DIM_2D;
      bounded[block][YY] = m_face_connectivity.adjacent_element(block, YPOS).first->element_type().dimensionality() == DIM_2D;
      bounded[block][ZZ] = m_face_connectivity.adjacent_element(block, ZPOS).first->element_type().dimensionality() == DIM_2D;
      bounded[block][XY] = bounded[block][XX] && bounded[block][YY];
      bounded[block][XZ] = bounded[block][XX] && bounded[block][ZZ];
      bounded[block][YZ] = bounded[block][YY] && bounded[block][ZZ];
      bounded[block][XYZ] = bounded[block][XX] && bounded[block][YY] && bounded[block][ZZ];

      const Uint nb_nodes = x_segs*y_segs*z_segs +
                            bounded[block][XX]*y_segs*z_segs +
                            bounded[block][YY]*x_segs*z_segs +
                            bounded[block][ZZ]*x_segs*y_segs +
                            bounded[block][XY]*z_segs +
                            bounded[block][XZ]*y_segs +
                            bounded[block][YZ]*x_segs +
                            bounded[block][XYZ];

      block_first_nodes.push_back(block_first_nodes.back() + nb_nodes);
    }

    // Initialize the nodes distribution
    nodes_dist.reserve(m_nb_procs+1);
    nodes_dist.push_back(0);
    for(Uint proc = 0; proc != m_nb_procs; ++proc)
    {
      nodes_dist.push_back(nodes_dist.back() + block_first_nodes[m_block_data.block_distribution[proc+1]] - block_first_nodes[m_block_data.block_distribution[proc]]);
    }

    m_local_nodes_begin = nodes_dist[m_rank];
    m_local_nodes_end = nodes_dist[m_rank+1];
  }

  /// Look up the local node index of node (i, j, k) in block
  /// @param block The block index
  /// @param i Node index in the X direction
  /// @param j Node index in the Y direction
  /// @param k Node index in the Z direction
  /// If the node is not owned by the current rank, a ghost node is added to the ghost map.
  Uint operator()(const Uint block, const Uint i, const Uint j, const Uint k)
  {
    const Uint gid = global_idx(block, i, j, k);
    if(gid >= m_local_nodes_begin && gid < m_local_nodes_end)
      return gid - m_local_nodes_begin;

    const Uint lid = m_local_nodes_end - m_local_nodes_begin + ghost_counter;
    std::pair<IndexMapT::iterator, bool> stored_gid = global_to_local.insert(std::make_pair(gid, lid));

    // increment the number of ghosts if we didn't add a ghost for this gid before
    if(stored_gid.second)
      ++ghost_counter;

    return stored_gid.first->second;
  }

  /// Look up the global node index of node (i, j, k) in block
  /// @param block The block index
  /// @param i Node index in the X direction
  /// @param j Node index in the Y direction
  /// @param k Node index in the Z direction
  Uint global_idx(const Uint block, const Uint i, const Uint j, const Uint k)
  {
    cf3_assert(block < m_block_data.block_subdivisions.size());

    const BlockData::CountsT& segments = m_block_data.block_subdivisions[block];
    const Uint x_segs = segments[XX];
    const Uint y_segs = segments[YY];
    const Uint z_segs = segments[ZZ];
    const Uint nb_internal_nodes = x_segs*y_segs*z_segs;

    const Uint XPOS = Hexa::KSI_POS;
    const Uint YPOS = Hexa::ETA_POS;
    const Uint ZPOS = Hexa::ZTA_POS;

    cf3_assert(i <= x_segs);
    cf3_assert(j <= y_segs);
    cf3_assert(k <= z_segs);


    // blocks contain their own nodes, except for XPOS, YPOS and ZPOS planes
    if(i != x_segs && j != y_segs && k != z_segs)
    {
      const Uint retval = block_first_nodes[block] + i + j*x_segs + k*x_segs*y_segs;
      cf3_assert(retval < block_first_nodes.back());
      return retval;
    }

    // XPOS plane
    if(i == x_segs && j != y_segs && k != z_segs)
    {
      if(!bounded[block][XX])
      {
        const Uint adj_block = m_face_connectivity.adjacent_element(block, XPOS).second;
        const BlockData::CountsT& adj_segs = m_block_data.block_subdivisions[adj_block];
        const Uint retval = block_first_nodes[adj_block] + j*adj_segs[XX] + k*adj_segs[XX]*adj_segs[YY];
        cf3_assert(retval < block_first_nodes.back());
        return retval;
      }
      else
      {
        const Uint retval = block_first_nodes[block] + nb_internal_nodes + j + k*y_segs;
        cf3_assert(retval < block_first_nodes.back());
        return retval;
      }
    }

    // YPOS plane
    if(i != x_segs && j == y_segs && k != z_segs)
    {
      if(!bounded[block][YY])
      {
        const Uint adj_block = m_face_connectivity.adjacent_element(block, YPOS).second;
        const BlockData::CountsT& adj_segs = m_block_data.block_subdivisions[adj_block];
        const Uint retval = block_first_nodes[adj_block] + i + k*adj_segs[XX]*adj_segs[YY];
        cf3_assert(retval < block_first_nodes.back());
        return retval;
      }
      else
      {
        const Uint retval = block_first_nodes[block] + nb_internal_nodes + bounded[block][XX]*y_segs*z_segs + i + k*x_segs;
        cf3_assert(retval < block_first_nodes.back());
        return retval;
      }
    }

    // ZPOS plane
    if(i != x_segs && j != y_segs && k == z_segs)
    {
      if(!bounded[block][ZZ])
      {
        const Uint adj_block = m_face_connectivity.adjacent_element(block, ZPOS).second;
        const BlockData::CountsT& adj_segs = m_block_data.block_subdivisions[adj_block];
        const Uint retval = block_first_nodes[adj_block] + i + j*adj_segs[XX];
        cf3_assert(retval < block_first_nodes.back());
        return retval;
      }
      else
      {
        const Uint retval = block_first_nodes[block] + nb_internal_nodes + bounded[block][XX]*y_segs*z_segs + bounded[block][YY]*x_segs*z_segs + i + j*x_segs;
        cf3_assert(retval < block_first_nodes.back());
        return retval;
      }
    }

    // XPOS and YPOS intersection
    if(i == x_segs && j == y_segs && k != z_segs)
    {
      if(!bounded[block][XY])
      {
        if(!bounded[block][XX])
        {
          const Uint x_adj = m_face_connectivity.adjacent_element(block, XPOS).second;
          const Uint retval = global_idx(x_adj, 0, j, k);
          cf3_assert(retval < block_first_nodes.back());
          return retval;
        }
        else
        {
          const Uint y_adj = m_face_connectivity.adjacent_element(block, YPOS).second;
          const Uint retval = global_idx(y_adj, i, 0, k);
          cf3_assert(retval < block_first_nodes.back());
          return retval;
        }
      }
      else
      {
        const Uint retval = block_first_nodes[block] + nb_internal_nodes +
               bounded[block][XX]*y_segs*z_segs +
               bounded[block][YY]*x_segs*z_segs +
               bounded[block][ZZ]*x_segs*y_segs +
               k;
        cf3_assert(retval < block_first_nodes.back());
        return retval;
      }
    }

    // XPOS and ZPOS intersection
    if(i == x_segs && j != y_segs && k == z_segs)
    {
      if(!bounded[block][XZ])
      {
        if(!bounded[block][XX])
        {
          const Uint x_adj = m_face_connectivity.adjacent_element(block, XPOS).second;
          const Uint retval = global_idx(x_adj, 0, j, k);
          cf3_assert(retval < block_first_nodes.back());
          return retval;
        }
        else
        {
          const Uint z_adj = m_face_connectivity.adjacent_element(block, ZPOS).second;
          const Uint retval = global_idx(z_adj, i, j, 0);
          cf3_assert(retval < block_first_nodes.back());
          return retval;
        }
      }
      else
      {
        const Uint retval = block_first_nodes[block] + nb_internal_nodes +
               bounded[block][XX]*y_segs*z_segs +
               bounded[block][YY]*x_segs*z_segs +
               bounded[block][ZZ]*x_segs*y_segs +
               bounded[block][XY]*z_segs +
               j;
        cf3_assert(retval < block_first_nodes.back());
        return retval;
      }
    }

    // YPOS and ZPOS intersection
    if(i != x_segs && j == y_segs && k == z_segs)
    {
      if(!bounded[block][YZ])
      {
        if(!bounded[block][YY])
        {
          const Uint y_adj = m_face_connectivity.adjacent_element(block, YPOS).second;
          const Uint retval = global_idx(y_adj, i, 0, k);
          cf3_assert(retval < block_first_nodes.back());
          return retval;
        }
        else
        {
          const Uint z_adj = m_face_connectivity.adjacent_element(block, ZPOS).second;
          const Uint retval = global_idx(z_adj, i, j, 0);
          cf3_assert(retval < block_first_nodes.back());
          return retval;
        }
      }
      else
      {
        const Uint retval = block_first_nodes[block] + nb_internal_nodes +
               bounded[block][XX]*y_segs*z_segs +
               bounded[block][YY]*x_segs*z_segs +
               bounded[block][ZZ]*x_segs*y_segs +
               bounded[block][XY]*z_segs +
               bounded[block][XZ]*y_segs +
               i;
        cf3_assert(retval < block_first_nodes.back());
        return retval;
      }
    }

    // XPOS, YPOS and ZPOS intersection
    if(i == x_segs && j == y_segs && k == z_segs)
    {
      if(!bounded[block][XYZ])
      {
        if(!bounded[block][XX])
        {
          const Uint x_adj = m_face_connectivity.adjacent_element(block, XPOS).second;
          const Uint retval = global_idx(x_adj, 0, j, k);
          cf3_assert(retval < block_first_nodes.back());
          return retval;
        }
        if(!bounded[block][YY])
        {
          const Uint y_adj = m_face_connectivity.adjacent_element(block, YPOS).second;
          const Uint retval = global_idx(y_adj, i, 0, k);
          cf3_assert(retval < block_first_nodes.back());
          return retval;
        }
        if(!bounded[block][ZZ])
        {
          const Uint z_adj = m_face_connectivity.adjacent_element(block, ZPOS).second;
          const Uint retval = global_idx(z_adj, i, j, 0);
          cf3_assert(retval < block_first_nodes.back());
          return retval;
        }
      }
      else
      {
        const Uint retval = block_first_nodes[block] + nb_internal_nodes +
              bounded[block][XX]*y_segs*z_segs +
              bounded[block][YY]*x_segs*z_segs +
              bounded[block][ZZ]*x_segs*y_segs +
              bounded[block][XY]*z_segs +
              bounded[block][XZ]*y_segs +
              bounded[block][YZ]*x_segs;
        cf3_assert(retval < block_first_nodes.back());
        return retval;
      }
    }

    throw ShouldNotBeHere(FromHere(), "Bad node index combination");
  }

  /// Index of the first node in the global node array for each block. The last element is actually the total
  /// number of nodes in the mesh
  IndicesT block_first_nodes;

  /// Distribution of the nodes among the processes. nodes_dist[i] is the first node on each process.
  /// Length is nb_procs + 1, so the last element is the total number of nodes
  IndicesT nodes_dist;

  /// For each block, indicate if it is bounded by a boundary patch, in the X, Y, Z, XY, XZ, YZ and XYZ directions
  Bools2T bounded;

  /// Type defining a mapping between two indices
  typedef std::map<Uint, Uint> IndexMapT;

  /// Global to local mapping for this rank, containing only the ghost nodes.
  IndexMapT global_to_local;

  /// Counter for the ghost nodes
  Uint ghost_counter;

private:
  const CFaceConnectivity& m_face_connectivity;
  const BlockData& m_block_data;
  const Uint m_rank;
  const Uint m_nb_procs;

  // First local node
  Uint m_local_nodes_begin;
  // Last local node + 1
  Uint m_local_nodes_end;
};

/// looks up node indices based on structured block indices
struct NodeIndices2D
{
  typedef std::vector<Uint> IndicesT;
  typedef std::vector<Uint> CountsT;
  typedef boost::multi_array<bool, 2> Bools2T;

  // Intersection of planes
  enum Bounds { XY = 2 };

  NodeIndices2D(const CFaceConnectivity& face_connectivity, const BlockData& block_data, const Uint rank, const Uint nb_procs) :
    m_face_connectivity(face_connectivity),
    m_block_data(block_data),
    m_rank(rank),
    m_nb_procs(nb_procs),
    ghost_counter(0)
  {
    const Uint nb_blocks = m_block_data.block_subdivisions.size();
    bounded.resize(boost::extents[nb_blocks][3]);
    block_first_nodes.reserve(nb_blocks + 1);
    block_first_nodes.push_back(0);
    for(Uint block = 0; block != nb_blocks; ++block)
    {
      const BlockData::CountsT& segments = m_block_data.block_subdivisions[block];
      const Uint x_segs = segments[XX];
      const Uint y_segs = segments[YY];

      // Indices of the side faces in positive X and Y direction
      const Uint XPOS = 1;
      const Uint YPOS = 2;

      bounded[block][XX] = m_face_connectivity.adjacent_element(block, XPOS).first->element_type().dimensionality() == DIM_1D;
      bounded[block][YY] = m_face_connectivity.adjacent_element(block, YPOS).first->element_type().dimensionality() == DIM_1D;
      bounded[block][XY] = bounded[block][XX] && bounded[block][YY];

      const Uint nb_nodes = x_segs*y_segs +
                            bounded[block][XX]*y_segs +
                            bounded[block][YY]*x_segs +
                            bounded[block][XY];

      block_first_nodes.push_back(block_first_nodes.back() + nb_nodes);
    }

    // Initialize the nodes distribution
    nodes_dist.reserve(m_nb_procs+1);
    nodes_dist.push_back(0);
    for(Uint proc = 0; proc != m_nb_procs; ++proc)
    {
      nodes_dist.push_back(nodes_dist.back() + block_first_nodes[m_block_data.block_distribution[proc+1]] - block_first_nodes[m_block_data.block_distribution[proc]]);
    }

    m_local_nodes_begin = nodes_dist[m_rank];
    m_local_nodes_end = nodes_dist[m_rank+1];
  }

  /// Look up the local node index of node (i, j, k) in block
  /// @param block The block index
  /// @param i Node index in the X direction
  /// @param j Node index in the Y direction
  /// If the node is not owned by the current rank, a ghost node is added to the ghost map.
  Uint operator()(const Uint block, const Uint i, const Uint j)
  {
    const Uint gid = global_idx(block, i, j);
    if(gid >= m_local_nodes_begin && gid < m_local_nodes_end)
      return gid - m_local_nodes_begin;

    const Uint lid = m_local_nodes_end - m_local_nodes_begin + ghost_counter;
    std::pair<IndexMapT::iterator, bool> stored_gid = global_to_local.insert(std::make_pair(gid, lid));

    // increment the number of ghosts if we didn't add a ghost for this gid before
    if(stored_gid.second)
      ++ghost_counter;

    return stored_gid.first->second;
  }

  /// Look up the global node index of node (i, j, k) in block
  /// @param block The block index
  /// @param i Node index in the X direction
  /// @param j Node index in the Y direction
  Uint global_idx(const Uint block, const Uint i, const Uint j)
  {
    cf3_assert(block < m_block_data.block_subdivisions.size());

    const BlockData::CountsT& segments = m_block_data.block_subdivisions[block];
    const Uint x_segs = segments[XX];
    const Uint y_segs = segments[YY];
    const Uint nb_internal_nodes = x_segs*y_segs;

    const Uint XPOS = 1;
    const Uint YPOS = 2;

    cf3_assert(i <= x_segs);
    cf3_assert(j <= y_segs);

    // blocks contain their own nodes, except for XPOS and YPOS boundaries
    if(i != x_segs && j != y_segs)
    {
      const Uint retval = block_first_nodes[block] + i + j*x_segs;
      cf3_assert(retval < block_first_nodes.back());
      return retval;
    }

    // XPOS plane
    if(i == x_segs && j != y_segs)
    {
      if(!bounded[block][XX])
      {
        const Uint adj_block = m_face_connectivity.adjacent_element(block, XPOS).second;
        const BlockData::CountsT& adj_segs = m_block_data.block_subdivisions[adj_block];
        const Uint retval = block_first_nodes[adj_block] + j*adj_segs[XX];
        cf3_assert(retval < block_first_nodes.back());
        return retval;
      }
      else
      {
        const Uint retval = block_first_nodes[block] + nb_internal_nodes + j;
        cf3_assert(retval < block_first_nodes.back());
        return retval;
      }
    }

    // YPOS plane
    if(i != x_segs && j == y_segs)
    {
      if(!bounded[block][YY])
      {
        const Uint adj_block = m_face_connectivity.adjacent_element(block, YPOS).second;
        const BlockData::CountsT& adj_segs = m_block_data.block_subdivisions[adj_block];
        const Uint retval = block_first_nodes[adj_block] + i;
        cf3_assert(retval < block_first_nodes.back());
        return retval;
      }
      else
      {
        const Uint retval = block_first_nodes[block] + nb_internal_nodes + bounded[block][XX]*y_segs + i;
        cf3_assert(retval < block_first_nodes.back());
        return retval;
      }
    }

    // XPOS and YPOS
    if(i == x_segs && j == y_segs)
    {
      if(!bounded[block][XY])
      {
        if(!bounded[block][XX])
        {
          const Uint x_adj = m_face_connectivity.adjacent_element(block, XPOS).second;
          const Uint retval = global_idx(x_adj, 0, j);
          cf3_assert(retval < block_first_nodes.back());
          return retval;
        }
        if(!bounded[block][YY])
        {
          const Uint y_adj = m_face_connectivity.adjacent_element(block, YPOS).second;
          const Uint retval = global_idx(y_adj, i, 0);
          cf3_assert(retval < block_first_nodes.back());
          return retval;
        }
      }
      else
      {
        const Uint retval = block_first_nodes[block] + nb_internal_nodes +
              bounded[block][XX]*y_segs +
              bounded[block][YY]*x_segs;
        cf3_assert(retval < block_first_nodes.back());
        return retval;
      }
    }

    throw ShouldNotBeHere(FromHere(), "Bad node index combination");
  }

  /// Index of the first node in the global node array for each block. The last element is actually the total
  /// number of nodes in the mesh
  IndicesT block_first_nodes;

  /// Distribution of the nodes among the processes. nodes_dist[i] is the first node on each process.
  /// Length is nb_procs + 1, so the last element is the total number of nodes
  IndicesT nodes_dist;

  /// For each block, indicate if it is bounded by a boundary patch, in the X, Y, Z, XY, XZ, YZ and XYZ directions
  Bools2T bounded;

  /// Type defining a mapping between two indices
  typedef std::map<Uint, Uint> IndexMapT;

  /// Global to local mapping for this rank, containing only the ghost nodes.
  IndexMapT global_to_local;

  /// Counter for the ghost nodes
  Uint ghost_counter;

private:
  const CFaceConnectivity& m_face_connectivity;
  const BlockData& m_block_data;
  const Uint m_rank;
  const Uint m_nb_procs;

  // First local node
  Uint m_local_nodes_begin;
  // Last local node + 1
  Uint m_local_nodes_end;
};

/// Create the first step length and expansion ratios in each direction
void create_mapped_coords(const Uint segments, BlockData::GradingT::const_iterator gradings, common::Table<Real>::ArrayT& mapped_coords, const Uint nb_edges)
{
  const Real eps = 150*std::numeric_limits<Real>::epsilon();
  mapped_coords.resize(boost::extents[segments+1][nb_edges]);
  for(Uint edge = 0; edge != nb_edges; ++edge)
  {
    Real grading = *(gradings++);
    if(fabs(grading-1.) > 1.e-6)
    {
      const Real r = pow(grading, 1. / static_cast<Real>(segments - 1)); // expansion ratio
      for(Uint i = 0; i <= segments; ++i)
      {
        const Real result = 2. * (1. - pow(r, (int)i)) / (1. - grading*r) - 1.;
        mapped_coords[i][edge] = result;
        cf3_assert(fabs(result) < (1. + eps));
      }
    }
    else
    {
      const Real step = 2. / static_cast<Real>(segments);
      for(Uint i = 0; i <= segments; ++i)
      {
        mapped_coords[i][edge] = i*step - 1.;
        cf3_assert(fabs(mapped_coords[i][edge]) < 1. + eps);
      }
    }
    const Real start = mapped_coords[0][edge];
    cf3_assert(fabs(start+1.) < eps);
    const Real end = mapped_coords[segments][edge];
    cf3_assert(fabs(end-1.) < eps);
  }
}

void build_mesh_3d(BlockData& block_data, Mesh& mesh)
{
  common::Timer timer;
  const Uint nb_procs = PE::Comm::instance().size();
  const Uint rank = PE::Comm::instance().rank();
  cf3_assert(block_data.block_distribution.size() == nb_procs+1);

  // This is a "dummy" mesh, in which each element corresponds to a block in the blockMeshDict file.
  // The final mesh will in fact be a refinement of this mesh. Using a Mesh allows us to use the
  // coolfluid connectivity functions to determine inter-block connectivity and the relation to boundary patches.
  Mesh& block_mesh = block_data.create_component<Mesh>("block_mesh");
  std::map<std::string, std::string> patch_types;
  detail::create_block_mesh_3d(block_data, block_mesh, patch_types);

  const Elements& block_elements = find_component_recursively<Cells>(block_mesh);
  const common::Table<Uint>::ArrayT& block_connectivity = block_elements.node_connectivity().array();
  const common::Table<Real>& block_coordinates = block_mesh.geometry().coordinates();

  // Get the distribution of the elements across the CPUs
  detail::NodeIndices3D::IndicesT elements_dist;
  elements_dist.reserve(nb_procs+1);
  elements_dist.push_back(0);
  for(Uint proc = 0; proc != nb_procs; ++proc)
  {
    const Uint proc_begin = block_data.block_distribution[proc];
    const Uint proc_end = block_data.block_distribution[proc+1];
    Uint nb_elements = 0;
    for(Uint block = proc_begin; block != proc_end; ++block)
      nb_elements += block_data.block_subdivisions[block][XX] * block_data.block_subdivisions[block][YY] * block_data.block_subdivisions[block][ZZ];
    elements_dist.push_back(elements_dist.back() + nb_elements);
  }

  ////////////////////////////////////
  // Create the refined mesh
  ////////////////////////////////////

  // Helper data to get the inter-block connectivity right
  const CFaceConnectivity& volume_to_face_connectivity = find_component<CFaceConnectivity>(block_elements);

  // Helper class to compute node indices
  detail::NodeIndices3D nodes(volume_to_face_connectivity, block_data, rank, nb_procs);

  // begin and end for the nodes and blocks on this CPU
  const Uint blocks_begin = block_data.block_distribution[rank];
  const Uint blocks_end = block_data.block_distribution[rank+1];
  const Uint nodes_begin = nodes.nodes_dist[rank];
  const Uint nodes_end = nodes.nodes_dist[rank+1];
  const Uint nb_nodes_local = nodes_end - nodes_begin;

  Region& root_region = mesh.topology().create_region("root_region");
  Elements& volume_elements = root_region.create_region("volume").create_component<Cells>("interior");
  volume_elements.initialize("cf3.mesh.LagrangeP1.Hexa3D");
  volume_elements.node_connectivity().resize(elements_dist[rank+1]-elements_dist[rank]);
  common::Table<Uint>::ArrayT& volume_connectivity = volume_elements.node_connectivity().array();

  // Set the connectivity, this also updates ghost node indices
  Uint element_idx = 0; // global element index
  for(Uint block = blocks_begin; block != blocks_end; ++block)
  {
    const BlockData::IndicesT& segments = block_data.block_subdivisions[block];
    // Fill the volume connectivity table
    for(Uint k = 0; k != segments[ZZ]; ++k)
    {
      for(Uint j = 0; j != segments[YY]; ++j)
      {
        for(Uint i = 0; i != segments[XX]; ++i)
        {
          common::Table<Uint>::Row element_connectivity = volume_connectivity[element_idx++];
          element_connectivity[0] = nodes(block, i  , j  , k  );
          element_connectivity[1] = nodes(block, i+1, j  , k  );
          element_connectivity[2] = nodes(block, i+1, j+1, k  );
          element_connectivity[3] = nodes(block, i  , j+1, k  );
          element_connectivity[4] = nodes(block, i  , j  , k+1);
          element_connectivity[5] = nodes(block, i+1, j  , k+1);
          element_connectivity[6] = nodes(block, i+1, j+1, k+1);
          element_connectivity[7] = nodes(block, i  , j+1, k+1);
        }
      }
    }
  }

  // the total number of nodes is now known
  const Uint nb_nodes = nb_nodes_local + nodes.ghost_counter;
  mesh.initialize_nodes(nb_nodes, static_cast<Uint>(DIM_3D));

  // Create the node coordinates
  Geometry& mesh_geo_comp = root_region.geometry();
  common::Table<Real>::ArrayT& mesh_coords = mesh_geo_comp.coordinates().array();

  // Set the nodes, now the number of nodes is known
  volume_elements.assign_geometry(mesh_geo_comp);

  // Fill the coordinate array
  for(Uint block = blocks_begin; block != blocks_end; ++block)
  {
    typedef Hexa3D ET;
    ET::NodesT block_nodes;
    fill(block_nodes, block_coordinates, block_connectivity[block]);
    const BlockData::IndicesT& segments = block_data.block_subdivisions[block];
    const BlockData::GradingT& gradings = block_data.block_gradings[block];

    common::Table<Real>::ArrayT ksi, eta, zta; // Mapped coordinates along each edge
    detail::create_mapped_coords(segments[XX], gradings.begin(), ksi, 4);
    detail::create_mapped_coords(segments[YY], gradings.begin() + 4, eta, 4);
    detail::create_mapped_coords(segments[ZZ], gradings.begin() + 8, zta, 4);

    Real w[4][3]; // weights for each edge
    Real w_mag[3]; // Magnitudes of the weights
    for(Uint k = 0; k <= segments[ZZ]; ++k)
    {
      for(Uint j = 0; j <= segments[YY]; ++j)
      {
        for(Uint i = 0; i <= segments[XX]; ++i)
        {
          // Weights are calculating according to the BlockMesh algorithm
          w[0][KSI] = (1. - ksi[i][0])*(1. - eta[j][0])*(1. - zta[k][0]) + (1. + ksi[i][0])*(1. - eta[j][1])*(1. - zta[k][1]);
          w[1][KSI] = (1. - ksi[i][1])*(1. + eta[j][0])*(1. - zta[k][3]) + (1. + ksi[i][1])*(1. + eta[j][1])*(1. - zta[k][2]);
          w[2][KSI] = (1. - ksi[i][2])*(1. + eta[j][3])*(1. + zta[k][3]) + (1. + ksi[i][2])*(1. + eta[j][2])*(1. + zta[k][2]);
          w[3][KSI] = (1. - ksi[i][3])*(1. - eta[j][3])*(1. + zta[k][0]) + (1. + ksi[i][3])*(1. - eta[j][2])*(1. + zta[k][1]);
          w_mag[KSI] = (w[0][KSI] + w[1][KSI] + w[2][KSI] + w[3][KSI]);

          w[0][ETA] = (1. - eta[j][0])*(1. - ksi[i][0])*(1. - zta[k][0]) + (1. + eta[j][0])*(1. - ksi[i][1])*(1. - zta[k][3]);
          w[1][ETA] = (1. - eta[j][1])*(1. + ksi[i][0])*(1. - zta[k][1]) + (1. + eta[j][1])*(1. + ksi[i][1])*(1. - zta[k][2]);
          w[2][ETA] = (1. - eta[j][2])*(1. + ksi[i][3])*(1. + zta[k][1]) + (1. + eta[j][2])*(1. + ksi[i][2])*(1. + zta[k][2]);
          w[3][ETA] = (1. - eta[j][3])*(1. - ksi[i][3])*(1. + zta[k][0]) + (1. + eta[j][3])*(1. - ksi[i][2])*(1. + zta[k][3]);
          w_mag[ETA] = (w[0][ETA] + w[1][ETA] + w[2][ETA] + w[3][ETA]);

          w[0][ZTA] = (1. - zta[k][0])*(1. - ksi[i][0])*(1. - eta[j][0]) + (1. + zta[k][0])*(1. - ksi[i][3])*(1. - eta[j][3]);
          w[1][ZTA] = (1. - zta[k][1])*(1. + ksi[i][0])*(1. - eta[j][1]) + (1. + zta[k][1])*(1. + ksi[i][3])*(1. - eta[j][2]);
          w[2][ZTA] = (1. - zta[k][2])*(1. + ksi[i][1])*(1. + eta[j][1]) + (1. + zta[k][2])*(1. + ksi[i][2])*(1. + eta[j][2]);
          w[3][ZTA] = (1. - zta[k][3])*(1. - ksi[i][1])*(1. + eta[j][0]) + (1. + zta[k][3])*(1. - ksi[i][2])*(1. + eta[j][3]);
          w_mag[ZTA] = (w[0][ZTA] + w[1][ZTA] + w[2][ZTA] + w[3][ZTA]);

          // Get the mapped coordinates of the node to add
          ET::MappedCoordsT mapped_coords;
          mapped_coords[KSI] = (w[0][KSI]*ksi[i][0] + w[1][KSI]*ksi[i][1] + w[2][KSI]*ksi[i][2] + w[3][KSI]*ksi[i][3]) / w_mag[KSI];
          mapped_coords[ETA] = (w[0][ETA]*eta[j][0] + w[1][ETA]*eta[j][1] + w[2][ETA]*eta[j][2] + w[3][ETA]*eta[j][3]) / w_mag[ETA];
          mapped_coords[ZTA] = (w[0][ZTA]*zta[k][0] + w[1][ZTA]*zta[k][1] + w[2][ZTA]*zta[k][2] + w[3][ZTA]*zta[k][3]) / w_mag[ZTA];

          ET::SF::ValueT sf;
          ET::SF::compute_value(mapped_coords, sf);

          // Transform to real coordinates
          ET::CoordsT coords = sf * block_nodes;

          // Store the result
          const Uint node_idx = nodes(block, i, j, k);
          cf3_assert(node_idx < mesh_coords.size());
          mesh_coords[node_idx][XX] = coords[XX];
          mesh_coords[node_idx][YY] = coords[YY];
          mesh_coords[node_idx][ZZ] = coords[ZZ];
        }
      }
    }
  }

  // Create the boundary elements
  std::map<std::string, std::vector<Uint> > patch_first_elements;
  std::map<std::string, std::vector<Uint> > patch_elements_counts;
  const Region& block_mesh_region = find_component<Region>(block_mesh.topology());
  BOOST_FOREACH(const Elements& patch_block, find_components_recursively_with_filter<Elements>(block_mesh_region, IsElementsSurface()))
  {
    const CFaceConnectivity& adjacency_data = find_component<CFaceConnectivity>(patch_block);
    // Create the volume cells connectivity
    const std::string& patch_name = patch_block.parent().name();
    Elements& patch_elements = root_region.create_region(patch_name).create_elements("cf3.mesh.LagrangeP1.Quad3D", mesh_geo_comp);
    common::Table<Uint>::ArrayT& patch_connectivity = patch_elements.node_connectivity().array();

    const Uint nb_patches = patch_block.node_connectivity().array().size();
    for(Uint patch_idx = 0; patch_idx != nb_patches; ++patch_idx)
    {
      const Uint adjacent_face = adjacency_data.adjacent_face(patch_idx, 0);
      const CFaceConnectivity::ElementReferenceT block = adjacency_data.adjacent_element(patch_idx, 0);
      if(block.second < blocks_begin || block.second >= blocks_end)
        continue;
      const BlockData::CountsT& segments = block_data.block_subdivisions[block.second];
      if(adjacent_face == Hexa::KSI_NEG || adjacent_face == Hexa::KSI_POS)
      {
        const Uint patch_begin = patch_connectivity.size();
        const Uint patch_end = patch_begin + segments[YY]*segments[ZZ];
        patch_first_elements[patch_name].push_back(patch_begin);
        patch_elements_counts[patch_name].push_back(patch_end - patch_begin);
        patch_connectivity.resize(boost::extents[patch_end][4]);
        const Uint i = adjacent_face == Hexa::KSI_NEG ? 0 : segments[XX];
        for(Uint k = 0; k != segments[ZZ]; ++k)
        {
          for(Uint j = 0; j != segments[YY]; ++j)
          {
            common::Table<Uint>::Row element_connectivity = patch_connectivity[patch_begin + k*segments[YY] + j];
            element_connectivity[0] = nodes(block.second, i  , j  , k  );
            element_connectivity[adjacent_face == Hexa::KSI_NEG ? 1 : 3] = nodes(block.second, i  , j  , k+1);
            element_connectivity[2] = nodes(block.second, i  , j+1, k+1);
            element_connectivity[adjacent_face == Hexa::KSI_NEG ? 3 : 1] = nodes(block.second, i  , j+1, k  );
          }
        }
      }
      else if(adjacent_face == Hexa::ETA_NEG || adjacent_face == Hexa::ETA_POS)
      {
        const Uint patch_begin = patch_connectivity.size();
        const Uint patch_end = patch_begin + segments[XX]*segments[ZZ];
        patch_first_elements[patch_name].push_back(patch_begin);
        patch_elements_counts[patch_name].push_back(patch_end - patch_begin);
        patch_connectivity.resize(boost::extents[patch_end][4]);
        const Uint j = adjacent_face == Hexa::ETA_NEG ? 0 : segments[YY];
        for(Uint k = 0; k != segments[ZZ]; ++k)
        {
          for(Uint i = 0; i != segments[XX]; ++i)
          {
            common::Table<Uint>::Row element_connectivity = patch_connectivity[patch_begin + k*segments[XX] + i];
            element_connectivity[0] = nodes(block.second, i  , j  , k  );
            element_connectivity[adjacent_face == Hexa::ETA_NEG ? 3 : 1] = nodes(block.second, i  , j  , k+1);
            element_connectivity[2] = nodes(block.second, i+1, j  , k+1);
            element_connectivity[adjacent_face == Hexa::ETA_NEG ? 1 : 3] = nodes(block.second, i+1, j  , k);
          }
        }
      }
      else if(adjacent_face == Hexa::ZTA_NEG || adjacent_face == Hexa::ZTA_POS)
      {
        const Uint patch_begin = patch_connectivity.size();
        const Uint patch_end = patch_begin + segments[XX]*segments[YY];
        patch_first_elements[patch_name].push_back(patch_begin);
        patch_elements_counts[patch_name].push_back(patch_end - patch_begin);
        patch_connectivity.resize(boost::extents[patch_end][4]);
        const Uint k = adjacent_face == Hexa::ZTA_NEG ? 0 : segments[ZZ];
        for(Uint j = 0; j != segments[YY]; ++j)
        {
          for(Uint i = 0; i != segments[XX]; ++i)
          {
            common::Table<Uint>::Row element_connectivity = patch_connectivity[patch_begin + j*segments[XX] + i];
            element_connectivity[0] = nodes(block.second, i  , j  , k  );
            element_connectivity[adjacent_face == Hexa::ZTA_NEG ? 1 : 3] = nodes(block.second, i  , j+1, k  );
            element_connectivity[2] = nodes(block.second, i+1, j+1, k  );
            element_connectivity[adjacent_face == Hexa::ZTA_NEG ? 3 : 1] = nodes(block.second, i+1, j  , k  );
          }
        }
      }
      else
      {
        throw ShouldNotBeHere(FromHere(), "Invalid patch data");
      }
    }
  }

  if(PE::Comm::instance().is_active())
  {
    common::List<Uint>& gids = mesh.geometry().glb_idx(); gids.resize(nb_nodes);
    common::List<Uint>& ranks = mesh.geometry().rank(); ranks.resize(nb_nodes);

    // Local nodes
    for(Uint i = 0; i != nb_nodes_local; ++i)
    {
      gids[i] = i + nodes_begin;
      ranks[i] = rank;
    }

    // Ghosts
    for(detail::NodeIndices3D::IndexMapT::const_iterator ghost_it = nodes.global_to_local.begin(); ghost_it != nodes.global_to_local.end(); ++ghost_it)
    {
      const Uint global_id = ghost_it->first;
      const Uint local_id = ghost_it->second;
      gids[local_id] = global_id;
      ranks[local_id] = std::upper_bound(nodes.nodes_dist.begin(), nodes.nodes_dist.end(), global_id) - 1 - nodes.nodes_dist.begin();
    }

    mesh.geometry().coordinates().parallelize_with(mesh.geometry().comm_pattern());
    mesh.geometry().coordinates().synchronize();
  }
}

void build_mesh_2d(BlockData& block_data, Mesh& mesh)
{
  const Uint nb_procs = PE::Comm::instance().size();
  const Uint rank = PE::Comm::instance().rank();
  cf3_assert(block_data.block_distribution.size() == nb_procs+1);

  // This is a "dummy" mesh, in which each element corresponds to a block in the blockMeshDict file.
  // The final mesh will in fact be a refinement of this mesh. Using a Mesh allows us to use the
  // coolfluid connectivity functions to determine inter-block connectivity and the relation to boundary patches.
  Mesh& block_mesh = block_data.create_component<Mesh>("block_mesh");
  std::map<std::string, std::string> patch_types;
  detail::create_block_mesh_2d(block_data, block_mesh, patch_types);

  const Elements& block_elements = find_component_recursively<Cells>(block_mesh);
  const common::Table<Uint>::ArrayT& block_connectivity = block_elements.node_connectivity().array();
  const common::Table<Real>& block_coordinates = block_mesh.geometry().coordinates();

  // Get the distribution of the elements across the CPUs
  detail::NodeIndices2D::IndicesT elements_dist;
  elements_dist.reserve(nb_procs+1);
  elements_dist.push_back(0);
  for(Uint proc = 0; proc != nb_procs; ++proc)
  {
    const Uint proc_begin = block_data.block_distribution[proc];
    const Uint proc_end = block_data.block_distribution[proc+1];
    Uint nb_elements = 0;
    for(Uint block = proc_begin; block != proc_end; ++block)
      nb_elements += block_data.block_subdivisions[block][XX] * block_data.block_subdivisions[block][YY];
    elements_dist.push_back(elements_dist.back() + nb_elements);
  }

  ////////////////////////////////////
  // Create the refined mesh
  ////////////////////////////////////

  // Helper data to get the inter-block connectivity right
  const CFaceConnectivity& volume_to_face_connectivity = find_component<CFaceConnectivity>(block_elements);

  // Helper class to compute node indices
  detail::NodeIndices2D nodes(volume_to_face_connectivity, block_data, rank, nb_procs);

  // begin and end for the nodes and blocks on this CPU
  const Uint blocks_begin = block_data.block_distribution[rank];
  const Uint blocks_end = block_data.block_distribution[rank+1];
  const Uint nodes_begin = nodes.nodes_dist[rank];
  const Uint nodes_end = nodes.nodes_dist[rank+1];
  const Uint nb_nodes_local = nodes_end - nodes_begin;

  Region& root_region = mesh.topology().create_region("root_region");
  Elements& volume_elements = root_region.create_region("volume").create_component<Cells>("interior");
  volume_elements.initialize("cf3.mesh.LagrangeP1.Quad2D");
  volume_elements.node_connectivity().resize(elements_dist[rank+1]-elements_dist[rank]);
  common::Table<Uint>::ArrayT& volume_connectivity = volume_elements.node_connectivity().array();

  // Set the connectivity, this also updates ghost node indices
  Uint element_idx = 0; // global element index
  for(Uint block = blocks_begin; block != blocks_end; ++block)
  {
    const BlockData::IndicesT& segments = block_data.block_subdivisions[block];
    // Fill the volume connectivity table
    for(Uint j = 0; j != segments[YY]; ++j)
    {
      for(Uint i = 0; i != segments[XX]; ++i)
      {
        common::Table<Uint>::Row element_connectivity = volume_connectivity[element_idx++];
        element_connectivity[0] = nodes(block, i  , j  );
        element_connectivity[1] = nodes(block, i+1, j  );
        element_connectivity[2] = nodes(block, i+1, j+1);
        element_connectivity[3] = nodes(block, i  , j+1);
      }
    }
  }

  // the total number of nodes is now known
  const Uint nb_nodes = nb_nodes_local + nodes.ghost_counter;
  mesh.initialize_nodes(nb_nodes, static_cast<Uint>(DIM_2D));

  // Create the node coordinates
  Geometry& mesh_geo_comp = root_region.geometry();
  common::Table<Real>::ArrayT& mesh_coords = mesh_geo_comp.coordinates().array();

  // Set the nodes, now the number of nodes is known
  volume_elements.assign_geometry(mesh_geo_comp);

  // Fill the coordinate array
  for(Uint block = blocks_begin; block != blocks_end; ++block)
  {
    typedef Quad2D ET;
    ET::NodesT block_nodes;
    fill(block_nodes, block_coordinates, block_connectivity[block]);
    const BlockData::IndicesT& segments = block_data.block_subdivisions[block];
    const BlockData::GradingT& gradings = block_data.block_gradings[block];

    common::Table<Real>::ArrayT ksi, eta; // Mapped coordinates along each edge
    detail::create_mapped_coords(segments[XX], gradings.begin(), ksi, 2);
    detail::create_mapped_coords(segments[YY], gradings.begin() + 2, eta, 2);

    Real w[2][2]; // weights for each edge
    Real w_mag[2]; // Magnitudes of the weights
    for(Uint j = 0; j <= segments[YY]; ++j)
    {
      for(Uint i = 0; i <= segments[XX]; ++i)
      {
        // Weights are calculating according to the BlockMesh algorithm
        w[0][KSI] = (1. - ksi[i][0])*(1. - eta[j][0]) + (1. + ksi[i][0])*(1. - eta[j][1]);
        w[1][KSI] = (1. - ksi[i][1])*(1. + eta[j][0]) + (1. + ksi[i][1])*(1. + eta[j][1]);
        w_mag[KSI] = (w[0][KSI] + w[1][KSI]);

        w[0][ETA] = (1. - eta[j][0])*(1. - ksi[i][0]) + (1. + eta[j][0])*(1. - ksi[i][1]);
        w[1][ETA] = (1. - eta[j][1])*(1. + ksi[i][0]) + (1. + eta[j][1])*(1. + ksi[i][1]);
        w_mag[ETA] = (w[0][ETA] + w[1][ETA]);

        // Get the mapped coordinates of the node to add
        ET::MappedCoordsT mapped_coords;
        mapped_coords[KSI] = (w[0][KSI]*ksi[i][0] + w[1][KSI]*ksi[i][1]) / w_mag[KSI];
        mapped_coords[ETA] = (w[0][ETA]*eta[j][0] + w[1][ETA]*eta[j][1]) / w_mag[ETA];

        ET::SF::ValueT sf;
        ET::SF::compute_value(mapped_coords, sf);

        // Transform to real coordinates
        ET::CoordsT coords = sf * block_nodes;

        // Store the result
        const Uint node_idx = nodes(block, i, j);
        cf3_assert(node_idx < mesh_coords.size());
        mesh_coords[node_idx][XX] = coords[XX];
        mesh_coords[node_idx][YY] = coords[YY];
      }
    }
  }

  // Create the boundary elements
  std::map<std::string, std::vector<Uint> > patch_first_elements;
  std::map<std::string, std::vector<Uint> > patch_elements_counts;
  const Region& block_mesh_region = find_component<Region>(block_mesh.topology());
  BOOST_FOREACH(const Elements& patch_block, find_components_recursively_with_filter<Elements>(block_mesh_region, IsElementsSurface()))
  {
    const CFaceConnectivity& adjacency_data = find_component<CFaceConnectivity>(patch_block);
    // Create the volume cells connectivity
    const std::string& patch_name = patch_block.parent().name();
    Elements& patch_elements = root_region.create_region(patch_name).create_elements("cf3.mesh.LagrangeP1.Line2D", mesh_geo_comp);
    common::Table<Uint>::ArrayT& patch_connectivity = patch_elements.node_connectivity().array();

    // Numbering of the faces
    const Uint XNEG = 3;
    const Uint XPOS = 1;
    const Uint YNEG = 0;
    const Uint YPOS = 2;

    const Uint nb_patches = patch_block.node_connectivity().array().size();
    for(Uint patch_idx = 0; patch_idx != nb_patches; ++patch_idx)
    {
      const Uint adjacent_face = adjacency_data.adjacent_face(patch_idx, 0);
      const CFaceConnectivity::ElementReferenceT block = adjacency_data.adjacent_element(patch_idx, 0);
      if(block.second < blocks_begin || block.second >= blocks_end)
        continue;
      const BlockData::CountsT& segments = block_data.block_subdivisions[block.second];
      if(adjacent_face == XNEG || adjacent_face == XPOS)
      {
        const Uint patch_begin = patch_connectivity.size();
        const Uint patch_end = patch_begin + segments[YY];
        patch_first_elements[patch_name].push_back(patch_begin);
        patch_elements_counts[patch_name].push_back(patch_end - patch_begin);
        patch_connectivity.resize(boost::extents[patch_end][2]);
        const Uint i = adjacent_face == XNEG ? 0 : segments[XX];
        for(Uint j = 0; j != segments[YY]; ++j)
        {
          common::Table<Uint>::Row element_connectivity = patch_connectivity[patch_begin + j];
          element_connectivity[adjacent_face == XNEG ? 0 : 1] = nodes(block.second, i  , j  );
          element_connectivity[adjacent_face == XNEG ? 1 : 0] = nodes(block.second, i  , j+1);
        }
      }
      else if(adjacent_face == YNEG || adjacent_face == YPOS)
      {
        const Uint patch_begin = patch_connectivity.size();
        const Uint patch_end = patch_begin + segments[XX];
        patch_first_elements[patch_name].push_back(patch_begin);
        patch_elements_counts[patch_name].push_back(patch_end - patch_begin);
        patch_connectivity.resize(boost::extents[patch_end][2]);
        const Uint j = adjacent_face == YNEG ? 0 : segments[YY];
          for(Uint i = 0; i != segments[XX]; ++i)
          {
            common::Table<Uint>::Row element_connectivity = patch_connectivity[patch_begin + i];
            element_connectivity[adjacent_face == YNEG ? 0 : 1] = nodes(block.second, i  , j);
            element_connectivity[adjacent_face == YNEG ? 1 : 0] = nodes(block.second, i+1, j);
          }
      }
      else
      {
        throw ShouldNotBeHere(FromHere(), "Invalid patch data");
      }
    }
  }

  if(PE::Comm::instance().is_active())
  {
    common::List<Uint>& gids = mesh.geometry().glb_idx(); gids.resize(nb_nodes);
    common::List<Uint>& ranks = mesh.geometry().rank(); ranks.resize(nb_nodes);

    // Local nodes
    for(Uint i = 0; i != nb_nodes_local; ++i)
    {
      gids[i] = i + nodes_begin;
      ranks[i] = rank;
    }

    // Ghosts
    for(detail::NodeIndices2D::IndexMapT::const_iterator ghost_it = nodes.global_to_local.begin(); ghost_it != nodes.global_to_local.end(); ++ghost_it)
    {
      const Uint global_id = ghost_it->first;
      const Uint local_id = ghost_it->second;
      gids[local_id] = global_id;
      ranks[local_id] = std::upper_bound(nodes.nodes_dist.begin(), nodes.nodes_dist.end(), global_id) - 1 - nodes.nodes_dist.begin();
    }

    mesh.geometry().coordinates().parallelize_with(mesh.geometry().comm_pattern());
    mesh.geometry().coordinates().synchronize();
  }
}

} // detail

void build_mesh(BlockData& block_data, Mesh& mesh, const Uint overlap)
{
  if(block_data.dimension == 3)
    detail::build_mesh_3d(block_data, mesh);
  else if(block_data.dimension == 2)
    detail::build_mesh_2d(block_data, mesh);
  else
    throw BadValue(FromHere(), "Only 2D and 3D meshes are supported by the blockmesher. Requested dimension was " + to_str(block_data.dimension));

  const Uint rank = PE::Comm::instance().rank();
  const Uint nb_procs = PE::Comm::instance().size();

  // Total number of elements on this rank
  Uint mesh_nb_elems = 0;
  boost_foreach(Elements& elements , find_components_recursively<Elements>(mesh))
  {
    mesh_nb_elems += elements.size();
  }

  std::vector<Uint> nb_elements_accumulated;
  if(PE::Comm::instance().is_active())
  {
    // Get the total number of elements on each rank
    PE::Comm::instance().all_gather(mesh_nb_elems, nb_elements_accumulated);
  }
  else
  {
    nb_elements_accumulated.push_back(mesh_nb_elems);
  }
  cf3_assert(nb_elements_accumulated.size() == nb_procs);
  // Sum up the values
  for(Uint i = 1; i != nb_procs; ++i)
    nb_elements_accumulated[i] += nb_elements_accumulated[i-1];

  // Offset to start with for this rank
  Uint element_offset = rank == 0 ? 0 : nb_elements_accumulated[rank-1];

  // Update the element ranks and gids
  boost_foreach(Elements& elements , find_components_recursively<Elements>(mesh))
  {
    const Uint nb_elems = elements.size();
    elements.rank().resize(nb_elems);
    elements.glb_idx().resize(nb_elems);

    for (Uint elem=0; elem != nb_elems; ++elem)
    {
      elements.rank()[elem] = rank;
      elements.glb_idx()[elem] = elem + element_offset;
    }
    element_offset += nb_elems;
  }

  mesh.elements().update();

  mesh.update_statistics();

  if(overlap != 0 && PE::Comm::instance().size() > 1)
  {
    MeshTransformer& global_conn = mesh.create_component("GlobalConnectivity", "cf3.mesh.actions.GlobalConnectivity").as_type<MeshTransformer>();
    global_conn.transform(mesh);

    MeshTransformer& grow_overlap = mesh.create_component("GrowOverlap", "cf3.mesh.actions.GrowOverlap").as_type<MeshTransformer>();
    for(Uint i = 0; i != overlap; ++i)
      grow_overlap.transform(mesh);

    mesh.geometry().remove_component("CommPattern");
  }

  // Raise an event to indicate that a mesh was loaded happened
  XML::SignalOptions options;
  options.add_option< OptionURI >("mesh_uri", mesh.uri());

  XML::SignalFrame f= options.create_frame();
  Core::instance().event_handler().raise_event( "mesh_loaded", f );
}


void partition_blocks_3d(const BlockData& blocks_in, Mesh& block_mesh, const Uint nb_partitions, const CoordXYZ direction, BlockData& blocks_out)
{
  // Create a mesh for the serial blocks
  std::map<std::string, std::string> patch_types;
  detail::create_block_mesh_3d(blocks_in, block_mesh, patch_types);
  const Uint nb_blocks = blocks_in.block_points.size();

  Elements& block_elements = find_component_recursively<Cells>(block_mesh);
  common::Table<Real>::ArrayT& block_coordinates = block_elements.geometry().coordinates().array();
  const CFaceConnectivity& volume_to_face_connectivity = find_component<CFaceConnectivity>(block_elements);

  // Direction to search from
  const Uint start_direction = direction == XX ? Hexa::KSI_NEG : (direction == YY ? Hexa::ETA_NEG : Hexa::ZTA_NEG);
  const Uint end_direction = direction == XX ? Hexa::KSI_POS : (direction == YY ? Hexa::ETA_POS : Hexa::ZTA_POS);

  // Cache local node indices in the start direction
  BlockData::IndicesT start_face_nodes;
  BOOST_FOREACH(const Uint face_node_idx, Hexa3D::faces().nodes_range(start_direction))
  {
    start_face_nodes.push_back(face_node_idx);
  }

  // Cache local node indices in the opposite direction
  BlockData::IndicesT end_face_nodes;
  BOOST_FOREACH(const Uint face_node_idx, Hexa3D::faces().nodes_range(end_direction))
  {
    end_face_nodes.push_back(face_node_idx);
  }

  // Transverse directions
  BlockData::IndicesT transverse_directions;
  BlockData::IndicesT transverse_axes;
  if(direction == XX)
  {
    transverse_directions = boost::assign::list_of(Hexa::ETA_NEG)(Hexa::ETA_POS)(Hexa::ZTA_NEG)(Hexa::ZTA_POS);
    transverse_axes = boost::assign::list_of(YY)(ZZ);
  }
  else if(direction == YY)
  {
    transverse_directions = boost::assign::list_of(Hexa::KSI_NEG)(Hexa::KSI_POS)(Hexa::ZTA_NEG)(Hexa::ZTA_POS);
    transverse_axes = boost::assign::list_of(XX)(ZZ);
  }
  else if(direction == ZZ)
  {
    transverse_directions = boost::assign::list_of(Hexa::ETA_NEG)(Hexa::ETA_POS)(Hexa::KSI_NEG)(Hexa::KSI_POS);
    transverse_axes = boost::assign::list_of(YY)(XX);
  }

  // All the blocks at the start of the direction to partition in
  BlockData::IndicesT next_block_layer;
  for(Uint block_idx = 0; block_idx != nb_blocks; ++block_idx)
  {
    if(volume_to_face_connectivity.adjacent_element(block_idx, start_direction).first->element_type().dimensionality() != DIM_2D)
      continue;

    bool is_start = true;
    BOOST_FOREACH(const Uint transverse_direction, transverse_directions)
    {
      CFaceConnectivity::ElementReferenceT transverse_element = volume_to_face_connectivity.adjacent_element(block_idx, transverse_direction);
      if(transverse_element.first->element_type().dimensionality() == DIM_2D)
        continue;

      if(volume_to_face_connectivity.adjacent_element(transverse_element.second, start_direction).first->element_type().dimensionality() == DIM_3D)
      {
        is_start = false;
        break;
      }
    }
    if(is_start)
    {
      next_block_layer.push_back(block_idx);
    }
  }

  // total number of elements
  Uint global_nb_elements = 0;
  for(Uint block = 0; block != nb_blocks; ++block)
    global_nb_elements += blocks_in.block_subdivisions[block][XX] * blocks_in.block_subdivisions[block][YY] * blocks_in.block_subdivisions[block][ZZ];

  BlockData::Ptr blocks_to_partition = allocate_component<BlockData>("tmp_blocks"); //copy, so we can shrink partially-partitioned blocks
  blocks_in.copy_to(*blocks_to_partition);

  // Init output data
  blocks_in.copy_to(blocks_out);
  blocks_out.block_gradings.clear();
  blocks_out.block_points.clear();
  blocks_out.block_subdivisions.clear();
  blocks_out.patch_points.clear();
  blocks_out.patch_points.resize(blocks_in.patch_points.size());
  blocks_out.block_distribution.clear();

  // Size of one partition
  const Uint partition_size = static_cast<Uint>( ceil( static_cast<Real>(global_nb_elements) / static_cast<Real>(nb_partitions) ) );

  const Uint nb_nodes = blocks_in.points.size();
  BlockData::IndicesT start_node_mapping(nb_nodes);
  for(Uint node_idx = 0; node_idx != nb_nodes; ++node_idx)
    start_node_mapping[node_idx] = node_idx;

  BlockData::IndicesT end_node_mapping = start_node_mapping;

  // map patch names to their patch index
  std::map<std::string, Uint> patch_idx_map;
  for(Uint patch_idx = 0; patch_idx != blocks_in.patch_names.size(); ++patch_idx)
    patch_idx_map[blocks_in.patch_names[patch_idx]] = patch_idx;

  Uint nb_partitioned = 0;
  for(Uint partition = 0; partition != nb_partitions; ++partition)
  {
    blocks_out.block_distribution.push_back(blocks_out.block_points.size());

    BlockData::IndicesT current_block_layer = next_block_layer;
    // Get the total size of a slice of elements in the current direction
    Uint slice_size = 0;
    BOOST_FOREACH(const Uint block_idx, current_block_layer)
    {
      const BlockData::CountsT segments = blocks_to_partition->block_subdivisions[block_idx];
      slice_size += segments[transverse_axes[0]] * segments[transverse_axes[1]];
    }
    cf3_assert(slice_size);
    Uint partition_nb_slices = static_cast<Uint>( ceil( static_cast<Real>(partition_size) / static_cast<Real>(slice_size) ) );
    if((nb_partitioned + (partition_nb_slices * slice_size)) > global_nb_elements)
    {
      cf3_assert(partition == nb_partitions-1);
      const Uint nb_remaining_elements = global_nb_elements - nb_partitioned;
      cf3_assert( (nb_remaining_elements % slice_size) == 0 );
      partition_nb_slices = nb_remaining_elements / slice_size;
    }

    nb_partitioned += partition_nb_slices * slice_size;
    while(partition_nb_slices)
    {
      const Uint block_nb_slices = blocks_to_partition->block_subdivisions[current_block_layer.front()][direction];
      BlockData::BooleansT node_is_mapped(nb_nodes, false);

      // Create new blocks with the correct start node indices
      std::vector<BlockData::IndicesT> new_blocks;
      BOOST_FOREACH(const Uint block_idx, current_block_layer)
      {
        BlockData::IndicesT new_block_points(8);
        BOOST_FOREACH(const Uint i, start_face_nodes)
          new_block_points[i] = start_node_mapping[blocks_in.block_points[block_idx][i]];

        new_blocks.push_back(new_block_points);
      }

      if(block_nb_slices > partition_nb_slices) // block is larger than the remaining number of slices
      {
        BOOST_FOREACH(const Uint block_idx, current_block_layer)
        {
          common::Table<Real>::ArrayT mapped_coords;
          detail::create_mapped_coords(block_nb_slices, blocks_to_partition->block_gradings[block_idx].begin() + 4*direction, mapped_coords, 4);

          //Adjust gradings and nodes
          BlockData::GradingT new_gradings = blocks_in.block_gradings[block_idx];
          for(Uint i = 0; i != 4; ++i)
          {
            const Uint original_end_node_idx = blocks_in.block_points[block_idx][end_face_nodes[i]];
            const Uint start_i = (i == 0 || i == 2) ? i : (i == 3 ? 1 : 3);
            const Uint original_start_node_idx = blocks_in.block_points[block_idx][start_face_nodes[start_i]];
            const Uint grading_idx = (end_direction != Hexa::ETA_POS || i == 0 || i == 3) ? i : (i == 1 ? 3 : 2);

            if(!node_is_mapped[original_end_node_idx])
            {
              node_is_mapped[original_end_node_idx] = true;
              end_node_mapping[original_end_node_idx] = blocks_out.points.size();
              // Get new block node coords
              Line1D::MappedCoordsT mapped_coord;
              mapped_coord << mapped_coords[partition_nb_slices][grading_idx];

              const BlockData::PointT& old_node = blocks_in.points[original_end_node_idx];
              RealVector3 new_node;

              Line1D::NodesT block_nodes;
              block_nodes(0, XX) = block_coordinates[original_start_node_idx][direction];
              block_nodes(1, XX) = block_coordinates[original_end_node_idx][direction];
              Line1D::SF::ValueT sf_1d;
              Line1D::SF::compute_value(mapped_coord, sf_1d);
              const Line1D::CoordsT node_1d = sf_1d * block_nodes;

              new_node[XX] = direction == XX ? node_1d[XX] : old_node[XX];
              new_node[YY] = direction == YY ? node_1d[XX] : old_node[YY];
              new_node[ZZ] = direction == ZZ ? node_1d[XX] : old_node[ZZ];

              blocks_out.points.push_back(BlockData::PointT(3));
              blocks_out.points.back() = boost::assign::list_of(new_node[XX])(new_node[YY])(new_node[ZZ]);

              // adjust mapping of start nodes
              start_node_mapping[original_start_node_idx] = end_node_mapping[original_end_node_idx];
            }

            // Adjust the gradings
            new_gradings[4*direction + i] =   (mapped_coords[partition_nb_slices][grading_idx] - mapped_coords[partition_nb_slices - 1][grading_idx])
                                            / (mapped_coords[1][grading_idx] - mapped_coords[0][grading_idx]);
            blocks_to_partition->block_gradings[block_idx][4*direction + i] = (mapped_coords[block_nb_slices][grading_idx] - mapped_coords[block_nb_slices - 1][grading_idx])
                                                                           / (mapped_coords[partition_nb_slices + 1][grading_idx] - mapped_coords[partition_nb_slices][grading_idx]);
          }

          // Adjust number of segments
          BlockData::CountsT new_block_subdivisions = blocks_to_partition->block_subdivisions[block_idx];
          new_block_subdivisions[direction] = partition_nb_slices;
          blocks_to_partition->block_subdivisions[block_idx][direction] -= partition_nb_slices;

          // append data to the output
          blocks_out.block_gradings.push_back(new_gradings);
          blocks_out.block_subdivisions.push_back(new_block_subdivisions);
        }

        // Adjust coordinates of the block mesh
        for(Uint i = 0; i != nb_nodes; ++i)
        {
          const BlockData::PointT& new_point = blocks_out.points[start_node_mapping[i]];
          block_coordinates[i][XX] = new_point[XX];
          block_coordinates[i][YY] = new_point[YY];
          block_coordinates[i][ZZ] = new_point[ZZ];
        }

        // All slices are immediatly accounted for
        partition_nb_slices = 0;
      }
      else // blocks fits entirely into the partition
      {
        next_block_layer.clear();
        BOOST_FOREACH(const Uint block_idx, current_block_layer)
        {
          blocks_out.block_gradings.push_back(blocks_to_partition->block_gradings[block_idx]);
          blocks_out.block_subdivisions.push_back(blocks_to_partition->block_subdivisions[block_idx]);

          for(Uint i = 0; i != 4; ++i)
          {
            const Uint original_end_node_idx = blocks_in.block_points[block_idx][end_face_nodes[i]];
            end_node_mapping[original_end_node_idx] = original_end_node_idx;
          }

          // Update the next block layer
          const CFaceConnectivity::ElementReferenceT next_block = volume_to_face_connectivity.adjacent_element(block_idx, start_direction);
          if(next_block.first->element_type().dimensionality() == DIM_3D)
            next_block_layer.push_back(next_block.second);
        }

        // grow the next layer in case any new spanwise blocks appear
        BOOST_FOREACH(const Uint block_idx, next_block_layer)
        {
          BOOST_FOREACH(const Uint transverse_direction, transverse_directions)
          {
            const CFaceConnectivity::ElementReferenceT transverse_block = volume_to_face_connectivity.adjacent_element(block_idx, transverse_direction);
            if(transverse_block.first->element_type().dimensionality() == DIM_3D)
            {
              if(!std::count(next_block_layer.begin(), next_block_layer.end(), transverse_block.second))
              {
                next_block_layer.push_back(transverse_block.second);
              }
            }
          }
        }

        // deduct the number of slices this layer added
        partition_nb_slices -= block_nb_slices;
      }

      BOOST_FOREACH(const Uint block_idx, current_block_layer)
      {
        BOOST_FOREACH(const Uint i, end_face_nodes)
          new_blocks[block_idx][i] = end_node_mapping[blocks_in.block_points[block_idx][i]];

        blocks_out.block_points.push_back(new_blocks[block_idx]);

        // Add new patches
        BOOST_FOREACH(const Uint transverse_direction, transverse_directions)
        {
          const CFaceConnectivity::ElementReferenceT adjacent_element = volume_to_face_connectivity.adjacent_element(block_idx, transverse_direction);
          if(adjacent_element.first->element_type().dimensionality() == DIM_2D)
          {
            const Uint patch_idx = patch_idx_map[adjacent_element.first->parent().name()];
            BOOST_FOREACH(const Uint i, Hexa3D::faces().nodes_range(transverse_direction))
            {
              blocks_out.patch_points[patch_idx].push_back(new_blocks[block_idx][i]);
            }
          }
        }
      }
    }
  }

  blocks_out.block_distribution.push_back(blocks_out.block_points.size());

  // Preserve original start and end patches
  const BlockData::IndicesT start_end_directions = boost::assign::list_of(start_direction)(end_direction);
  for(Uint block_idx = 0; block_idx != nb_blocks; ++block_idx)
  {
    BOOST_FOREACH(const Uint lengthwise_direcion, start_end_directions)
    {
      const CFaceConnectivity::ElementReferenceT adjacent_element = volume_to_face_connectivity.adjacent_element(block_idx, lengthwise_direcion);
      if(adjacent_element.first->element_type().dimensionality() == DIM_2D)
      {
        const Uint patch_idx = patch_idx_map[adjacent_element.first->parent().name()];
        BOOST_FOREACH(const Uint i, Hexa3D::faces().nodes_range(lengthwise_direcion))
        {
          blocks_out.patch_points[patch_idx].push_back(blocks_in.block_points[block_idx][i]);
        }
      }
    }
  }

  cf3_assert(blocks_out.dimension == 3);
}

void partition_blocks_2d(const BlockData& blocks_in, Mesh& block_mesh, const Uint nb_partitions, const CoordXYZ direction, BlockData& blocks_out)
{
  // Create a mesh for the serial blocks
  std::map<std::string, std::string> patch_types;
  detail::create_block_mesh_2d(blocks_in, block_mesh, patch_types);
  const Uint nb_blocks = blocks_in.block_points.size();

  Elements& block_elements = find_component_recursively<Cells>(block_mesh);
  common::Table<Real>::ArrayT& block_coordinates = block_elements.geometry().coordinates().array();
  const CFaceConnectivity& volume_to_face_connectivity = find_component<CFaceConnectivity>(block_elements);

  // Numbering of the faces
  const Uint XNEG = 3;
  const Uint XPOS = 1;
  const Uint YNEG = 0;
  const Uint YPOS = 2;

  // Direction to search from
  const Uint start_direction = direction == XX ? XNEG : YNEG;
  const Uint end_direction = direction == XX ? XPOS : YPOS;

  // Cache local node indices in the start direction
  BlockData::IndicesT start_face_nodes;
  BOOST_FOREACH(const Uint face_node_idx, Quad2D::faces().nodes_range(start_direction))
  {
    start_face_nodes.push_back(face_node_idx);
  }

  // Cache local node indices in the opposite direction
  BlockData::IndicesT end_face_nodes;
  BOOST_FOREACH(const Uint face_node_idx, Quad2D::faces().nodes_range(end_direction))
  {
    end_face_nodes.push_back(face_node_idx);
  }

  // Transverse directions
  const BlockData::IndicesT transverse_directions = direction == XX ? boost::assign::list_of(YNEG)(YPOS) : boost::assign::list_of(XNEG)(XPOS);
  const Uint transverse_axe = direction == XX ? YY : XX;

  // All the blocks at the start of the direction to partition in
  BlockData::IndicesT next_block_layer;
  for(Uint block_idx = 0; block_idx != nb_blocks; ++block_idx)
  {
    if(volume_to_face_connectivity.adjacent_element(block_idx, start_direction).first->element_type().dimensionality() != DIM_1D)
      continue;

    bool is_start = true;
    BOOST_FOREACH(const Uint transverse_direction, transverse_directions)
    {
      CFaceConnectivity::ElementReferenceT transverse_element = volume_to_face_connectivity.adjacent_element(block_idx, transverse_direction);
      if(transverse_element.first->element_type().dimensionality() == DIM_1D)
        continue;

      if(volume_to_face_connectivity.adjacent_element(transverse_element.second, start_direction).first->element_type().dimensionality() == DIM_2D)
      {
        is_start = false;
        break;
      }
    }
    if(is_start)
    {
      next_block_layer.push_back(block_idx);
    }
  }

  // total number of elements
  Uint global_nb_elements = 0;
  for(Uint block = 0; block != nb_blocks; ++block)
    global_nb_elements += blocks_in.block_subdivisions[block][XX] * blocks_in.block_subdivisions[block][YY];

  BlockData::Ptr blocks_to_partition = allocate_component<BlockData>("tmp_blocks"); //copy, so we can shrink partially-partitioned blocks
  blocks_in.copy_to(*blocks_to_partition);

  // Init output data
  blocks_in.copy_to(blocks_out);
  blocks_out.block_gradings.clear();
  blocks_out.block_points.clear();
  blocks_out.block_subdivisions.clear();
  blocks_out.patch_points.clear();
  blocks_out.patch_points.resize(blocks_in.patch_points.size());
  blocks_out.block_distribution.clear();

  // Size of one partition
  const Uint partition_size = static_cast<Uint>( ceil( static_cast<Real>(global_nb_elements) / static_cast<Real>(nb_partitions) ) );

  const Uint nb_nodes = blocks_in.points.size();
  BlockData::IndicesT start_node_mapping(nb_nodes);
  for(Uint node_idx = 0; node_idx != nb_nodes; ++node_idx)
    start_node_mapping[node_idx] = node_idx;

  BlockData::IndicesT end_node_mapping = start_node_mapping;

  // map patch names to their patch index
  std::map<std::string, Uint> patch_idx_map;
  for(Uint patch_idx = 0; patch_idx != blocks_in.patch_names.size(); ++patch_idx)
    patch_idx_map[blocks_in.patch_names[patch_idx]] = patch_idx;

  Uint nb_partitioned = 0;
  for(Uint partition = 0; partition != nb_partitions; ++partition)
  {
    blocks_out.block_distribution.push_back(blocks_out.block_points.size());

    BlockData::IndicesT current_block_layer = next_block_layer;
    // Get the total size of a slice of elements in the current direction
    Uint slice_size = 0;
    BOOST_FOREACH(const Uint block_idx, current_block_layer)
    {
      const BlockData::CountsT segments = blocks_to_partition->block_subdivisions[block_idx];
      slice_size += segments[transverse_axe];
    }
    cf3_assert(slice_size);
    Uint partition_nb_slices = static_cast<Uint>( ceil( static_cast<Real>(partition_size) / static_cast<Real>(slice_size) ) );
    if((nb_partitioned + (partition_nb_slices * slice_size)) > global_nb_elements)
    {
      cf3_assert(partition == nb_partitions-1);
      const Uint nb_remaining_elements = global_nb_elements - nb_partitioned;
      cf3_assert( (nb_remaining_elements % slice_size) == 0 );
      partition_nb_slices = nb_remaining_elements / slice_size;
    }

    nb_partitioned += partition_nb_slices * slice_size;
    while(partition_nb_slices)
    {
      const Uint block_nb_slices = blocks_to_partition->block_subdivisions[current_block_layer.front()][direction];
      BlockData::BooleansT node_is_mapped(nb_nodes, false);

      // Create new blocks with the correct start node indices
      std::vector<BlockData::IndicesT> new_blocks;
      BOOST_FOREACH(const Uint block_idx, current_block_layer)
      {
        BlockData::IndicesT new_block_points(4);
        BOOST_FOREACH(const Uint i, start_face_nodes)
          new_block_points[i] = start_node_mapping[blocks_in.block_points[block_idx][i]];

        new_blocks.push_back(new_block_points);
      }

      if(block_nb_slices > partition_nb_slices) // block is larger than the remaining number of slices
      {
        BOOST_FOREACH(const Uint block_idx, current_block_layer)
        {
          common::Table<Real>::ArrayT mapped_coords;
          detail::create_mapped_coords(block_nb_slices, blocks_to_partition->block_gradings[block_idx].begin() + 2*direction, mapped_coords, 2);

          //Adjust gradings and nodes
          BlockData::GradingT new_gradings = blocks_in.block_gradings[block_idx];
          for(Uint i = 0; i != 2; ++i)
          {
            const Uint original_end_node_idx = blocks_in.block_points[block_idx][end_face_nodes[i]];
            const Uint original_start_node_idx = blocks_in.block_points[block_idx][start_face_nodes[i == 0 ? 1 : 0]];
            const Uint grading_idx = (end_direction != YPOS) ? i : (i == 0 ? 1 : 0);

            if(!node_is_mapped[original_end_node_idx])
            {
              node_is_mapped[original_end_node_idx] = true;
              end_node_mapping[original_end_node_idx] = blocks_out.points.size();
              // Get new block node coords
              Line1D::MappedCoordsT mapped_coord;
              mapped_coord << mapped_coords[partition_nb_slices][grading_idx];

              const BlockData::PointT& old_node = blocks_in.points[original_end_node_idx];
              RealVector2 new_node;

              Line1D::NodesT block_nodes;
              block_nodes(0, XX) = block_coordinates[original_start_node_idx][direction];
              block_nodes(1, XX) = block_coordinates[original_end_node_idx][direction];
              Line1D::SF::ValueT sf_1d;
              Line1D::SF::compute_value(mapped_coord, sf_1d);
              const Line1D::CoordsT node_1d = sf_1d * block_nodes;

              new_node[XX] = direction == XX ? node_1d[XX] : old_node[XX];
              new_node[YY] = direction == YY ? node_1d[XX] : old_node[YY];

              blocks_out.points.push_back(BlockData::PointT(2));
              blocks_out.points.back() = boost::assign::list_of(new_node[XX])(new_node[YY]);

              // adjust mapping of start nodes
              start_node_mapping[original_start_node_idx] = end_node_mapping[original_end_node_idx];
            }

            // Adjust the gradings
            new_gradings[2*direction + i] =   (mapped_coords[partition_nb_slices][grading_idx] - mapped_coords[partition_nb_slices - 1][grading_idx])
                                            / (mapped_coords[1][grading_idx] - mapped_coords[0][grading_idx]);
            blocks_to_partition->block_gradings[block_idx][2*direction + i] = (mapped_coords[block_nb_slices][grading_idx] - mapped_coords[block_nb_slices - 1][grading_idx])
                                                                           / (mapped_coords[partition_nb_slices + 1][grading_idx] - mapped_coords[partition_nb_slices][grading_idx]);
          }

          // Adjust number of segments
          BlockData::CountsT new_block_subdivisions = blocks_to_partition->block_subdivisions[block_idx];
          new_block_subdivisions[direction] = partition_nb_slices;
          blocks_to_partition->block_subdivisions[block_idx][direction] -= partition_nb_slices;

          // append data to the output
          blocks_out.block_gradings.push_back(new_gradings);
          blocks_out.block_subdivisions.push_back(new_block_subdivisions);
        }

        // Adjust coordinates of the block mesh
        for(Uint i = 0; i != nb_nodes; ++i)
        {
          const BlockData::PointT& new_point = blocks_out.points[start_node_mapping[i]];
          block_coordinates[i][XX] = new_point[XX];
          block_coordinates[i][YY] = new_point[YY];
        }

        // All slices are immediatly accounted for
        partition_nb_slices = 0;
      }
      else // blocks fits entirely into the partition
      {
        next_block_layer.clear();
        BOOST_FOREACH(const Uint block_idx, current_block_layer)
        {
          blocks_out.block_gradings.push_back(blocks_to_partition->block_gradings[block_idx]);
          blocks_out.block_subdivisions.push_back(blocks_to_partition->block_subdivisions[block_idx]);

          for(Uint i = 0; i != 2; ++i)
          {
            const Uint original_end_node_idx = blocks_in.block_points[block_idx][end_face_nodes[i]];
            end_node_mapping[original_end_node_idx] = original_end_node_idx;
          }

          // Update the next block layer
          const CFaceConnectivity::ElementReferenceT next_block = volume_to_face_connectivity.adjacent_element(block_idx, start_direction);
          if(next_block.first->element_type().dimensionality() == DIM_2D)
            next_block_layer.push_back(next_block.second);
        }

        // grow the next layer in case any new spanwise blocks appear
        BOOST_FOREACH(const Uint block_idx, next_block_layer)
        {
          BOOST_FOREACH(const Uint transverse_direction, transverse_directions)
          {
            const CFaceConnectivity::ElementReferenceT transverse_block = volume_to_face_connectivity.adjacent_element(block_idx, transverse_direction);
            if(transverse_block.first->element_type().dimensionality() == DIM_2D)
            {
              if(!std::count(next_block_layer.begin(), next_block_layer.end(), transverse_block.second))
              {
                next_block_layer.push_back(transverse_block.second);
              }
            }
          }
        }

        // deduct the number of slices this layer added
        partition_nb_slices -= block_nb_slices;
      }

      BOOST_FOREACH(const Uint block_idx, current_block_layer)
      {
        BOOST_FOREACH(const Uint i, end_face_nodes)
          new_blocks[block_idx][i] = end_node_mapping[blocks_in.block_points[block_idx][i]];

        blocks_out.block_points.push_back(new_blocks[block_idx]);

        // Add new patches
        BOOST_FOREACH(const Uint transverse_direction, transverse_directions)
        {
          const CFaceConnectivity::ElementReferenceT adjacent_element = volume_to_face_connectivity.adjacent_element(block_idx, transverse_direction);
          if(adjacent_element.first->element_type().dimensionality() == DIM_1D)
          {
            const Uint patch_idx = patch_idx_map[adjacent_element.first->parent().name()];
            BOOST_FOREACH(const Uint i, Quad2D::faces().nodes_range(transverse_direction))
            {
              blocks_out.patch_points[patch_idx].push_back(new_blocks[block_idx][i]);
            }
          }
        }
      }
    }
  }

  blocks_out.block_distribution.push_back(blocks_out.block_points.size());

  // Preserve original start and end patches
  const BlockData::IndicesT start_end_directions = boost::assign::list_of(start_direction)(end_direction);
  for(Uint block_idx = 0; block_idx != nb_blocks; ++block_idx)
  {
    BOOST_FOREACH(const Uint lengthwise_direcion, start_end_directions)
    {
      const CFaceConnectivity::ElementReferenceT adjacent_element = volume_to_face_connectivity.adjacent_element(block_idx, lengthwise_direcion);
      if(adjacent_element.first->element_type().dimensionality() == DIM_1D)
      {
        const Uint patch_idx = patch_idx_map[adjacent_element.first->parent().name()];
        BOOST_FOREACH(const Uint i, Quad2D::faces().nodes_range(lengthwise_direcion))
        {
          blocks_out.patch_points[patch_idx].push_back(blocks_in.block_points[block_idx][i]);
        }
      }
    }
  }
}

void partition_blocks(const cf3::mesh::BlockMesh::BlockData& blocks_in, const cf3::Uint nb_partitions, const cf3::CoordXYZ direction, cf3::mesh::BlockMesh::BlockData& blocks_out)
{
  Mesh& block_mesh = blocks_out.create_component<Mesh>("serial_block_mesh");

  if(blocks_in.dimension == 3)
  {
    partition_blocks_3d(blocks_in, block_mesh, nb_partitions, direction, blocks_out);
  }
  else if(blocks_in.dimension == 2)
  {
    partition_blocks_2d(blocks_in, block_mesh, nb_partitions, direction, blocks_out);
  }
  else
  {
    throw BadValue(FromHere(), "Only 2D and 3D meshes are supported by the blockmesher. Requested dimension was " + to_str(blocks_in.dimension));
  }
}



void create_block_mesh(const BlockData& block_data, Mesh& mesh)
{
  std::map<std::string, std::string> unused;
  if(block_data.dimension == 3)
    detail::create_block_mesh_3d(block_data, mesh, unused);
  else if(block_data.dimension == 2)
    detail::create_block_mesh_2d(block_data, mesh, unused);
  else
    throw BadValue(FromHere(), "Only 2D and 3D meshes are supported by the blockmesher. Requested dimension was " + to_str(block_data.dimension));
}


} // BlockMesh
} // mesh
} // cf3
