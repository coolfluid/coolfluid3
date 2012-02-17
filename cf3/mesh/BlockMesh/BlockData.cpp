// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/list_of.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

#include "common/Builder.hpp"
#include "common/Core.hpp"
#include "common/Exception.hpp"
#include "common/EventHandler.hpp"
#include "common/Group.hpp"
#include "common/List.hpp"
#include "common/Log.hpp"
#include "common/OptionURI.hpp"
#include "common/XML/SignalFrame.hpp"
#include "common/XML/SignalOptions.hpp"
#include "common/Timer.hpp"
#include "common/Table.hpp"
#include "common/StreamHelpers.hpp"

#include "common/PE/Comm.hpp"

#include "mesh/BlockMesh/BlockData.hpp"

#include "mesh/Cells.hpp"
#include "mesh/Elements.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/MeshElements.hpp"
#include "mesh/MeshTransformer.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Field.hpp"
#include "mesh/ConnectivityData.hpp"
#include "mesh/Region.hpp"
#include "mesh/ElementData.hpp"
#include "mesh/Space.hpp"

#include "mesh/LagrangeP1/Hexa3D.hpp"
#include "mesh/LagrangeP1/Line1D.hpp"
#include "mesh/LagrangeP1/Quad2D.hpp"

namespace cf3 {
namespace mesh {
namespace BlockMesh {

using namespace cf3::common;
using namespace cf3::mesh;
using namespace cf3::mesh::LagrangeP1;

namespace detail
{
  /// Shortcut to create a signal reply
  void create_reply(SignalArgs& args, const Component& created_component)
  {
    SignalFrame reply = args.create_reply(created_component.parent()->uri());
    SignalOptions reply_options(reply);
    reply_options.add_option("created_component", created_component.uri());
  }

  /// Create the first step length and expansion ratios in each direction (in the mapped space)
  void create_mapped_coords(const Uint segments, const Real* gradings, common::Table<Real>::ArrayT& mapped_coords, const Uint nb_edges)
  {
    const Real eps = 1500*std::numeric_limits<Real>::epsilon();
    mapped_coords.resize(boost::extents[segments+1][nb_edges]);
    for(Uint edge = 0; edge != nb_edges; ++edge)
    {
      Real grading = gradings[edge];
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

  // Helper function to convert from a table to the old-style storage
  template<typename ValT, typename RowT>
  void add_row(std::vector<ValT>& output, const RowT& input)
  {
    output.push_back(ValT(input.size()));
    std::copy(input.begin(), input.end(), output.back().begin());
  }
}

ComponentBuilder < BlockArrays, Component, LibBlockMesh > BlockArrays_Builder;

/// Helper struct to keep using the old partitioning method
struct BlocksPartitioning
{
  /// Type to store indices into another vector
  typedef std::vector<Uint> IndicesT;
  /// Data type for counts of data, i.e. number of points
  typedef std::vector<Uint> CountsT;
  /// Storage for a single point coordinate (STL vector for ease of use with boost::spirit)
  typedef std::vector<Real> PointT;
  /// Storage for a grading corresponding to a single block
  typedef std::vector<Real> GradingT;
  /// Storage for true/false flags
  typedef std::vector<bool> BooleansT;

  Uint dimension;

  /// The coordinates for all the nodes
  std::vector<PointT> points;

  /// Points for each block, in terms of node indices
  std::vector<IndicesT> block_points;
  /// Subdivisions for each block, along X, Y and Z
  std::vector<CountsT> block_subdivisions;
  /// edgeGrading for each block
  std::vector<GradingT> block_gradings;
  /// Distribution of blocks among processors
  IndicesT block_distribution;

  /// Name for each patch
  std::vector<std::string> patch_names;
  /// Point indices for each patch (grouped per 4 or 2, depending on dimension)
  std::vector<IndicesT> patch_points;
};

struct BlockArrays::Implementation
{
  Handle< common::Table<Real> > points;
  Handle< common::Table<Uint> > blocks;
  Handle< common::Table<Uint> > block_subdivisions;
  Handle< common::Table<Real> > block_gradings;

  Handle< common::Group > patches;

  Handle<Mesh> block_mesh;
  Handle<Connectivity> default_shell_connectivity;
  Handle<CFaceConnectivity> face_connectivity;

  /// Encapsulate a single block, providing all data needed to produce the mesh connectivity
  struct Block
  {
    /// Constructor taking the number of dimensions as argument
    Block(const Uint dim) :
      dimensions(dim),
      nb_points(dim),
      segments(dim),
      bounded(dim),
      neighbors(dim, nullptr),
      strides(dim)
    {
    }

    /// Get the block corresponding to index i in a certain direction. Meant to be called sequentially like:
    ///  block[i][j][k]
    Block operator[](const Uint i) const
    {
      const Uint search_direction = search_indices.size();
      cf3_assert(search_direction < dimensions);

      // Data can be found in the neigboring block
      if(i == nb_points[search_direction])
      {
        Block neighbor = *neighbors[search_direction];
        neighbor.search_indices = search_indices;
        neighbor.search_indices.push_back(0);
        return neighbor;
      }

      // We have the data here
      Block result = *this;
      result.search_indices.push_back(i);
      return result;
    }

    /// Get the global index. Available after "dimension" subsequent calls to operator[]
    Uint global_idx() const
    {
      cf3_assert(search_indices.size() == dimensions);
      Uint result = start_index;
      for(Uint i = 0; i != dimensions; ++i)
      {
        result += strides[i]*search_indices[i];
      }
      return result;
    }

    /// Number of dimensions (2 or 3)
    Uint dimensions;
    /// Previous indices passed to operator[]
    std::vector<Uint> search_indices;
    /// Number of points in each direction
    std::vector<Uint> nb_points;
    /// Number of elements
    Uint nb_elems;
    /// Segments in each direction
    std::vector<Uint> segments;
    /// True if bounded in on the positive side for each direction
    std::vector<bool> bounded;
    /// Neighbors in the positive direction
    std::vector<Block*> neighbors;
    /// Strides in each direction
    std::vector<Uint> strides;
    /// Starting index for this block
    Uint start_index;
    /// True if the block is stored on the current MPI rank
    bool is_local;
  };

  struct Patch
  {
    Patch(const Block& a_block, const Uint fixed_dir, const Uint idx) :
      block(a_block),
      fixed_direction(fixed_dir),
      fixed_idx(idx)
    {
      nb_elems = 1;
      segments.reserve(block.dimensions-1);
      for(Uint i = 0; i != block.dimensions; ++i)
      {
        if(i != fixed_dir)
        {
          segments.push_back(block.segments[i]);
          nb_elems *= block.segments[i];
        }
      }
    }

    /// Access to a global index, 1D version
    Uint global_idx(Uint i) const
    {
      cf3_assert(block.dimensions == 2);
      i = fixed_idx ? i : segments[0]-i;
      return block[fixed_direction == 0 ? fixed_idx : i][fixed_direction == 1 ? fixed_idx : i].global_idx();
    }

    /// Access to a global index, 2D version
    Uint global_idx(Uint i, Uint j) const
    {
      cf3_assert(block.dimensions == 3);
      if(fixed_direction != 2)
        i = fixed_idx ? i : segments[0]-i;
      else
        i = fixed_idx ? segments[0]-i : i;
      switch(fixed_direction)
      {
        case 0:
          return block[fixed_idx][i][j].global_idx();
        case 1:
          return block[i][fixed_idx][j].global_idx();
        case 2:
          return block[i][j][fixed_idx].global_idx();
      }
      return 0;
    }

    const Block& block;
    Uint nb_elems;
    std::vector<Uint> segments;
    Uint fixed_direction;
    Uint fixed_idx;
  };

  void trigger_block_distribution()
  {
    const Uint nb_blocks = blocks->size();
    if(block_distribution.empty() || block_distribution.back() != blocks->size())
    {
      block_distribution.assign(2, 0);
      block_distribution.back() = nb_blocks;
    }
  }

  /// Create a list of blocks, initialized based on the blockmesh structure
  void create_blocks()
  {
    ghost_counter = 0;
    const Uint rank = PE::Comm::instance().rank();
    const Uint partition_begin = block_distribution[rank];
    const Uint partition_end = block_distribution[rank+1];

    const Uint nb_blocks = blocks->size();
    const Uint dimensions = points->row_size();

    CFaceConnectivity& face_conn = *face_connectivity;

    // Unify positive axis face_indices between 2D and 3D cases
    std::vector<Uint> positive_faces(dimensions);
    std::vector<Uint> negative_faces(dimensions);
    if(dimensions == 3)
    {
      positive_faces[0] = LagrangeP1::Hexa::KSI_POS;
      positive_faces[1] = LagrangeP1::Hexa::ETA_POS;
      positive_faces[2] = LagrangeP1::Hexa::ZTA_POS;

      negative_faces[0] = LagrangeP1::Hexa::KSI_NEG;
      negative_faces[1] = LagrangeP1::Hexa::ETA_NEG;
      negative_faces[2] = LagrangeP1::Hexa::ZTA_NEG;
    }
    else
    {
      positive_faces[0] = 1;
      positive_faces[1] = 2;

      negative_faces[0] = 3;
      negative_faces[1] = 0;
    }

    block_list.assign(nb_blocks, Block(dimensions));
    patch_map.clear();
    const Table<Uint>& block_subdivs = *block_subdivisions;
    Uint block_start = 0;
    for(Uint block_idx = 0; block_idx != nb_blocks; ++block_idx)
    {
      Block& block = block_list[block_idx];
      block.is_local = block_idx >= partition_begin && block_idx < partition_end;
      block.start_index = block_start;

      const Table<Uint>::ConstRow row = (*blocks)[block_idx];
      const Table<Uint>::ConstRow subdiv_row = block_subdivs[block_idx];

      Uint stride = 1;
      Uint nb_points = 1;
      block.nb_elems = 1;
      for(Uint i = 0; i != dimensions; ++i)
      {
        // Add the block
        CFaceConnectivity::ElementReferenceT adj_elem = face_conn.adjacent_element(block_idx, positive_faces[i]);
        block.strides[i] = stride;
        block.bounded[i] = adj_elem.first->element_type().dimensionality() != dimensions;
        block.nb_points[i] = subdiv_row[i] + (block.bounded[i] ? 1 : 0);
        block.segments[i] = subdiv_row[i];
        block.nb_elems *= subdiv_row[i];
        stride *= block.nb_points[i];
        nb_points *= block.nb_points[i];
        block.neighbors[i] = block.bounded[i] ? nullptr : &block_list[adj_elem.second];
      }

      // Add patches
      for(Uint i = 0; i != dimensions; ++i)
      {
        const Elements* adj_elems[2];
        adj_elems[0] = face_conn.adjacent_element(block_idx, negative_faces[i]).first;
        adj_elems[1] = face_conn.adjacent_element(block_idx, positive_faces[i]).first;
        // check for a patch both in the positive and negative direction
        for(Uint dir = 0; dir != 2; ++dir)
        {
          if(adj_elems[dir]->element_type().dimensionality() == (dimensions-1))
          {
            patch_map[adj_elems[dir]->parent()->name()].push_back(new Patch(block, i, dir * (block.nb_points[i]-1)));
          }
        }
      }
      block_start += nb_points;
    }
  }

  /// Distribution of nodes among the CPUs
  void create_nodes_distribution(const Uint nb_procs, const Uint rank)
  {
    cf3_assert(rank < nb_procs);

    if(block_distribution.size() != (nb_procs+1))
      throw SetupError(FromHere(), "Block distribution size of " + boost::lexical_cast<std::string>(block_distribution.size()) + " does not match number of processes " + boost::lexical_cast<std::string>(nb_procs) + "+1. Did you parallelize the blocks?");

    // Initialize the nodes distribution
    nodes_dist.reserve(nb_procs+1);
    nodes_dist.push_back(0);
    for(Uint proc = 0; proc != (nb_procs-1); ++proc)
    {
      nodes_dist.push_back(nodes_dist.back() + block_list[block_distribution[proc+1]].start_index - block_list[block_distribution[proc]].start_index);
    }
    const Block& last_block = block_list[block_distribution[nb_procs]-1];
    Uint last_block_nb_nodes = 1;
    for(Uint i = 0; i != last_block.dimensions; ++i)
      last_block_nb_nodes *= last_block.nb_points[i];

    nodes_dist.push_back(nodes_dist.back() + last_block.start_index - block_list[block_distribution[nb_procs-1]].start_index + last_block_nb_nodes);

    local_nodes_begin = nodes_dist[rank];
    local_nodes_end = nodes_dist[rank+1];
  }

  /// Convert a global index to a local one, creating a ghost node if needed
  Uint to_local(const Uint gid)
  {
    if(gid >= local_nodes_begin && gid < local_nodes_end)
      return gid - local_nodes_begin;

    const Uint lid = local_nodes_end - local_nodes_begin + ghost_counter;
    std::pair<IndexMapT::iterator, bool> stored_gid = global_to_local.insert(std::make_pair(gid, lid));

    // increment the number of ghosts if we didn't add a ghost for this gid before
    if(stored_gid.second)
      ++ghost_counter;

    return stored_gid.first->second;
  }

  template<typename T>
  void check_handle(const Handle<T>& h, const std::string& signal_name, const std::string& description)
  {
    if(is_null(h))
      throw SetupError(FromHere(), description + " not defined. Did you call the " + signal_name + " signal?");
  }

  void add_block(const Table<Uint>::ConstRow& segments, const Uint block_idx, Connectivity& volume_connectivity, Uint& element_idx)
  {
    if(segments.size() == 3)
    {
      for(Uint k = 0; k != segments[ZZ]; ++k)
      {
        for(Uint j = 0; j != segments[YY]; ++j)
        {
          for(Uint i = 0; i != segments[XX]; ++i)
          {
            common::Table<Uint>::Row element_connectivity = volume_connectivity[element_idx++];
            element_connectivity[0] = to_local(block_list[block_idx][i  ][j  ][k  ].global_idx());
            element_connectivity[1] = to_local(block_list[block_idx][i+1][j  ][k  ].global_idx());
            element_connectivity[2] = to_local(block_list[block_idx][i+1][j+1][k  ].global_idx());
            element_connectivity[3] = to_local(block_list[block_idx][i  ][j+1][k  ].global_idx());
            element_connectivity[4] = to_local(block_list[block_idx][i  ][j  ][k+1].global_idx());
            element_connectivity[5] = to_local(block_list[block_idx][i+1][j  ][k+1].global_idx());
            element_connectivity[6] = to_local(block_list[block_idx][i+1][j+1][k+1].global_idx());
            element_connectivity[7] = to_local(block_list[block_idx][i  ][j+1][k+1].global_idx());
          }
        }
      }
    }
    else
    {
      cf3_assert(segments.size() == 2);
      for(Uint j = 0; j != segments[YY]; ++j)
      {
        for(Uint i = 0; i != segments[XX]; ++i)
        {
          common::Table<Uint>::Row element_connectivity = volume_connectivity[element_idx++];
          element_connectivity[0] = to_local(block_list[block_idx][i  ][j  ].global_idx());
          element_connectivity[1] = to_local(block_list[block_idx][i+1][j  ].global_idx());
          element_connectivity[2] = to_local(block_list[block_idx][i+1][j+1].global_idx());
          element_connectivity[3] = to_local(block_list[block_idx][i  ][j+1].global_idx());
        }
      }
    }
  }

  /// Create the block coordinates
  template<typename ET>
  void fill_block_coordinates_3d(Table<Real>& mesh_coords, const Uint block_idx)
  {
    typename ET::NodesT block_nodes;
    fill(block_nodes, *points, (*blocks)[block_idx]);
    const Table<Uint>::ConstRow& segments = (*block_subdivisions)[block_idx];
    const Table<Real>::ConstRow& gradings = (*block_gradings)[block_idx];

    common::Table<Real>::ArrayT ksi, eta, zta; // Mapped coordinates along each edge
    detail::create_mapped_coords(segments[XX], &gradings[0], ksi, 4);
    detail::create_mapped_coords(segments[YY], &gradings[4], eta, 4);
    detail::create_mapped_coords(segments[ZZ], &gradings[8], zta, 4);

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
          typename ET::MappedCoordsT mapped_coords;
          mapped_coords[KSI] = (w[0][KSI]*ksi[i][0] + w[1][KSI]*ksi[i][1] + w[2][KSI]*ksi[i][2] + w[3][KSI]*ksi[i][3]) / w_mag[KSI];
          mapped_coords[ETA] = (w[0][ETA]*eta[j][0] + w[1][ETA]*eta[j][1] + w[2][ETA]*eta[j][2] + w[3][ETA]*eta[j][3]) / w_mag[ETA];
          mapped_coords[ZTA] = (w[0][ZTA]*zta[k][0] + w[1][ZTA]*zta[k][1] + w[2][ZTA]*zta[k][2] + w[3][ZTA]*zta[k][3]) / w_mag[ZTA];

          typename ET::SF::ValueT sf;
          ET::SF::compute_value(mapped_coords, sf);

          // Transform to real coordinates
          typename ET::CoordsT coords = sf * block_nodes;

          // Store the result
          const Uint node_idx = to_local(block_list[block_idx][i][j][k].global_idx());
          cf3_assert(node_idx < mesh_coords.size());
          mesh_coords[node_idx][XX] = coords[XX];
          mesh_coords[node_idx][YY] = coords[YY];
          mesh_coords[node_idx][ZZ] = coords[ZZ];
        }
      }
    }
  }

  /// Create the block coordinates
  template<typename ET>
  void fill_block_coordinates_2d(Table<Real>& mesh_coords, const Uint block_idx)
  {
    typename ET::NodesT block_nodes;
    fill(block_nodes, *points, (*blocks)[block_idx]);
    const Table<Uint>::ConstRow& segments = (*block_subdivisions)[block_idx];
    const Table<Real>::ConstRow& gradings = (*block_gradings)[block_idx];

    common::Table<Real>::ArrayT ksi, eta; // Mapped coordinates along each edge
    detail::create_mapped_coords(segments[XX], &gradings[0], ksi, 2);
    detail::create_mapped_coords(segments[YY], &gradings[2], eta, 2);

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
        typename ET::MappedCoordsT mapped_coords;
        mapped_coords[KSI] = (w[0][KSI]*ksi[i][0] + w[1][KSI]*ksi[i][1]) / w_mag[KSI];
        mapped_coords[ETA] = (w[0][ETA]*eta[j][0] + w[1][ETA]*eta[j][1]) / w_mag[ETA];

        typename ET::SF::ValueT sf;
        ET::SF::compute_value(mapped_coords, sf);

        // Transform to real coordinates
        typename ET::CoordsT coords = sf * block_nodes;

        // Store the result
        const Uint node_idx = to_local(block_list[block_idx][i][j].global_idx());
        cf3_assert(node_idx < mesh_coords.size());
        mesh_coords[node_idx][XX] = coords[XX];
        mesh_coords[node_idx][YY] = coords[YY];
      }
    }
  }

  void add_patch(const std::string& name, Elements& patch_elems)
  {
    const Uint dimensions = points->row_size();

    // Determine patch number of elements
    Uint patch_nb_elems = 0;
    BOOST_FOREACH(const Patch& patch, patch_map[name])
    {
      if(patch.block.is_local)
        patch_nb_elems += patch.nb_elems;
    }
    patch_elems.resize(patch_nb_elems);

    Connectivity& patch_conn = patch_elems.geometry_space().connectivity();

    if(dimensions == 3)
    {
      Uint elem_idx = 0;
      BOOST_FOREACH(const Patch& patch, patch_map[name])
      {
        if(!patch.block.is_local)
          continue;
        for(Uint i = 0; i != patch.segments[0]; ++i)
        {
          for(Uint j = 0; j != patch.segments[1]; ++j)
          {
            Connectivity::Row elem_row = patch_conn[elem_idx++];
            elem_row[0] = to_local(patch.global_idx(i,   j  ));
            elem_row[1] = to_local(patch.global_idx(i+1, j  ));
            elem_row[2] = to_local(patch.global_idx(i+1, j+1));
            elem_row[3] = to_local(patch.global_idx(i,   j+1));
          }
        }
      }
    }
    else
    {
      cf3_assert(dimensions == 2);
      Uint elem_idx = 0;
      BOOST_FOREACH(const Patch& patch, patch_map[name])
      {
        if(!patch.block.is_local)
          continue;
        for(Uint i = 0; i != patch.segments[0]; ++i)
        {
          Connectivity::Row elem_row = patch_conn[elem_idx++];
          elem_row[0] = to_local(patch.global_idx(i  ));
          elem_row[1] = to_local(patch.global_idx(i+1));
        }
      }
    }
  }

  /// Create the data structure used to partition blocks
  BlocksPartitioning create_partitioning_data()
  {
    BlocksPartitioning result;

    const Table<Real>& points_tbl = *points;
    result.dimension = points->row_size();

    const Table<Uint>& blocks_tbl = *blocks;
    const Uint nb_blocks = blocks_tbl.size();

    result.block_distribution = block_distribution;

    const Uint nb_points = points_tbl.size();
    result.points.reserve(nb_points);
    for(Uint i = 0; i != nb_points; ++i)
    {
      detail::add_row(result.points, points_tbl[i]);
    }

    const Table<Real>& gradings_tbl = *block_gradings;
    const Table<Uint>& subdivs_tbl = *block_subdivisions;
    result.block_gradings.reserve(nb_blocks);
    result.block_points.reserve(nb_blocks);
    result.block_subdivisions.reserve(nb_blocks);
    for(Uint i = 0; i != nb_blocks; ++i)
    {
      detail::add_row(result.block_gradings, gradings_tbl[i]);
      detail::add_row(result.block_points, blocks_tbl[i]);
      detail::add_row(result.block_subdivisions, subdivs_tbl[i]);
    }

    BOOST_FOREACH(const Table<Uint>& patch, find_components< Table<Uint> >(*patches))
    {
      result.patch_names.push_back(patch.name());
      const Uint patch_row_size = patch.row_size();
      const Uint patch_size = patch.size();
      result.patch_points.push_back(BlocksPartitioning::IndicesT(patch_row_size*patch_size));
      BlocksPartitioning::IndicesT::iterator insert_it = result.patch_points.back().begin();
      for(Uint i = 0; i != patch_size; ++i)
      {
        std::copy(patch[i].begin(), patch[i].end(), insert_it);
        insert_it += patch_row_size;
      }
    }

    return result;
  }

  /// Copy back data from a partitioning structure
  void update_blocks(const BlocksPartitioning& blocks_partitioning)
  {
    print_vector(CFdebug << "Printing partitioning data for block distribution ", blocks_partitioning.block_distribution);
    CFdebug << CFendl;
    const Uint nb_points = blocks_partitioning.points.size();
    points->resize(nb_points);
    CFdebug << "Partitioned points:" << CFendl;
    for(Uint i = 0; i != nb_points; ++i)
    {
      points->set_row(i, blocks_partitioning.points[i]);

      print_vector(CFdebug << "  " << i << ": ", blocks_partitioning.points[i]);
      CFdebug << CFendl;
    }

    const Uint nb_blocks = blocks_partitioning.block_points.size();
    blocks->resize(nb_blocks);
    block_gradings->resize(nb_blocks);
    block_subdivisions->resize(nb_blocks);
    CFdebug << "Partitioned blocks (subdivisions):" << CFendl;
    for(Uint i = 0; i != nb_blocks; ++i)
    {
      blocks->set_row(i, blocks_partitioning.block_points[i]);
      block_subdivisions->set_row(i, blocks_partitioning.block_subdivisions[i]);
      block_gradings->set_row(i, blocks_partitioning.block_gradings[i]);

      print_vector(CFdebug << "  " << i << ": ", blocks_partitioning.block_points[i]);
      print_vector(CFdebug << " (", blocks_partitioning.block_subdivisions[i]);
      CFdebug  << ")" << CFendl;
    }

    const Uint nb_patches = blocks_partitioning.patch_names.size();
    CFdebug << "Partitioned patches:" << CFendl;
    for(Uint i = 0; i != nb_patches; ++i)
    {
      Handle< Table<Uint> > patch_tbl(patches->get_child(blocks_partitioning.patch_names[i]));
      const Uint patch_nb_elems = blocks_partitioning.patch_points[i].size();
      patch_tbl->resize(patch_nb_elems/patch_tbl->row_size());
      patch_tbl->seekp(0);
      for(Uint j = 0; j != patch_nb_elems; ++j)
        (*patch_tbl) << blocks_partitioning.patch_points[i][j];
      patch_tbl->seekp(0);

      print_vector(CFdebug << "  " << blocks_partitioning.patch_names[i] << ": ", blocks_partitioning.patch_points[i]);
      CFdebug << CFendl;
    }

    block_distribution = blocks_partitioning.block_distribution;
  }

  /// Represents a single layer across the mesh
  struct BlockLayer
  {
    /// Size of a single layer of cells
    Uint segment_size;
    /// Number of cell layers for the block layer
    Uint nb_segments;
    /// Block layer across all original partitions
    std::vector<Uint> global_layer;
    /// Block layer filtered to only contain the blocks for the current original partition
    std::vector<Uint> local_layer;
    /// Accumulation of all added layers
    std::vector<Uint> added_blocks;
    /// Reamaining number of segments when multiple partitions fit in one block layer
    Uint remaining_nb_segments;
  };

  /// Recursive function to build a list of block indices that constitutes a single layer
  bool build_block_layer(const Uint block_idx, const Uint direction, const std::vector<Uint>& transverse_directions, const std::vector<Uint>& output_block_layer, const std::vector<Uint>& added_blocks, std::set<Uint>& recursed_blocks)
  {
    if(!recursed_blocks.insert(block_idx).second)
      return true;

    // Block was already found or is not part of the search
    if(std::count(output_block_layer.begin(), output_block_layer.end(), block_idx) != 0 ||
       std::count(added_blocks.begin(), added_blocks.end(), block_idx) != 0)
      return true;

    cf3_assert(face_connectivity->has_adjacent_element(block_idx, direction));
    const CFaceConnectivity::ElementReferenceT adj_elem = face_connectivity->adjacent_element(block_idx, direction);
    // If we have a volume element as neighbor in the main negative direction
    // and it is not in the previous layer, then we are not in the current layer.
    if(adj_elem.first->element_type().dimensionality() == adj_elem.first->element_type().dimension() &&
       std::count(added_blocks.begin(), added_blocks.end(), adj_elem.second) == 0)
      return false;

    // Recurse through the transverse directions
    BOOST_FOREACH(const Uint transverse_direction, transverse_directions)
    {
      const CFaceConnectivity::ElementReferenceT transverse_elem = face_connectivity->adjacent_element(block_idx, transverse_direction);
      if(transverse_elem.first->element_type().dimensionality() < transverse_elem.first->element_type().dimension()) // skip faces
        continue;
      if(!build_block_layer(transverse_elem.second, direction, transverse_directions, output_block_layer, added_blocks, recursed_blocks))
        return false;
    }

    return true;
  }

  void build_block_layer(const Uint direction, const Uint start_direction, const std::vector<Uint>& transverse_directions, const Uint partition, BlockLayer& layer)
  {
    layer.added_blocks.insert(layer.added_blocks.end(), layer.global_layer.begin(), layer.global_layer.end());
    layer.global_layer.clear();
    const Uint nb_blocks = blocks->size();
    for(Uint block_idx = 0; block_idx != nb_blocks; ++block_idx)
    {
      if(std::count(layer.added_blocks.begin(), layer.added_blocks.end(), block_idx) != 0)
        continue;

      std::set<Uint> recursed_blocks;
      if(build_block_layer(block_idx, start_direction, transverse_directions, layer.global_layer, layer.added_blocks, recursed_blocks))
        layer.global_layer.push_back(block_idx);
    }

    const Uint partition_begin = block_distribution[partition];
    const Uint partition_end = block_distribution[partition+1];
    layer.local_layer.clear();
    layer.segment_size = 0;
    layer.nb_segments = 0;
    Table<Uint>& subdivs = *block_subdivisions;
    const Uint dimensions = points->row_size();
    BOOST_FOREACH(const Uint block_idx, layer.global_layer)
    {
      if(layer.nb_segments == 0)
      {
        layer.nb_segments = subdivs[block_idx][direction];
      }
      else
      {
        cf3_assert(layer.nb_segments == subdivs[block_idx][direction]);
      }

      if(dimensions == 2)
      {
        layer.segment_size += subdivs[block_idx][direction == 0 ? 1 : 0];
      }
      else
      {
        Uint i, j;
        if(direction == 0) { i = 1; j = 2; }
        if(direction == 1) { i = 0; j = 2; }
        if(direction == 2) { i = 0; j = 1; }
        layer.segment_size += subdivs[block_idx][i]*subdivs[block_idx][j];
      }

      if(block_idx >= partition_begin && block_idx < partition_end)
        layer.local_layer.push_back(block_idx);
    }
    layer.remaining_nb_segments = layer.nb_segments;
  }

  void partition_blocks(const Uint nb_partitions, const Uint direction)
  {
    const Uint dimensions = points->row_size();
    const Uint mapped_stride = dimensions == 3 ? 4 : 2;
    const BlocksPartitioning blocks_in = create_partitioning_data();

    const Uint nb_blocks = blocks_in.block_points.size();

    const CFaceConnectivity& volume_to_face_connectivity = *face_connectivity;

    // Numbering of the faces
     const Uint XNEG_2D = 3;
     const Uint XPOS_2D = 1;
     const Uint YNEG_2D = 0;
     const Uint YPOS_2D = 2;

    // Direction to search from
    const Uint start_direction = dimensions == 3 ? ( direction == XX ? Hexa::KSI_NEG : (direction == YY ? Hexa::ETA_NEG : Hexa::ZTA_NEG) ) : (direction == XX ? XNEG_2D : YNEG_2D);
    const Uint end_direction = dimensions == 3 ? ( direction == XX ? Hexa::KSI_POS : (direction == YY ? Hexa::ETA_POS : Hexa::ZTA_POS) ) : (direction == XX ? XPOS_2D : YPOS_2D);

    const ElementTypeFaceConnectivity& etype_faces = dimensions == 3 ? Hexa3D::faces() : Quad2D::faces();

    // Cache local node indices in the start direction
    BlocksPartitioning::IndicesT start_face_nodes;
    BOOST_FOREACH(const Uint face_node_idx, etype_faces.nodes_range(start_direction))
    {
      start_face_nodes.push_back(face_node_idx);
    }

    // Cache local node indices in the opposite direction
    BlocksPartitioning::IndicesT end_face_nodes;
    BOOST_FOREACH(const Uint face_node_idx, etype_faces.nodes_range(end_direction))
    {
      end_face_nodes.push_back(face_node_idx);
    }

    // Transverse directions
    BlocksPartitioning::IndicesT transverse_directions;
    BlocksPartitioning::IndicesT transverse_axes;
    if(dimensions == 2)
    {
      transverse_directions = direction == XX ? boost::assign::list_of(YNEG_2D)(YPOS_2D) : boost::assign::list_of(XNEG_2D)(XPOS_2D);
      transverse_axes.assign(1, direction == XX ? YY : XX);
    }
    else
    {
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
    }

    // map patch names to their patch index
    std::map<std::string, Uint> patch_idx_map;
    for(Uint patch_idx = 0; patch_idx != blocks_in.patch_names.size(); ++patch_idx)
      patch_idx_map[blocks_in.patch_names[patch_idx]] = patch_idx;

    // Init output data
    BlocksPartitioning blocks_out;
    blocks_out.points = blocks_in.points;
    blocks_out.patch_names = blocks_in.patch_names;
    blocks_out.dimension = blocks_in.dimension;
    blocks_out.patch_points.resize(blocks_in.patch_points.size());

    const Uint nb_nodes = blocks_in.points.size();
    std::vector< std::vector<Uint> > node_mapping(nb_partitions+1, std::vector<Uint>(nb_nodes));
    std::vector< std::vector<bool> > node_is_mapped(nb_partitions+1, std::vector<bool>(nb_nodes, false));
    for(Uint node_idx = 0; node_idx != nb_nodes; ++node_idx)
    {
      for(Uint i = 0; i != (nb_partitions+1); ++i)
      {
        node_mapping[i][node_idx] = node_idx;
      }
      node_is_mapped[0][node_idx] = true;
      node_is_mapped[nb_partitions][node_idx] = true;
    }

    // total number of elements
    Uint global_nb_elements = 0;
    for(Uint block = 0; block != nb_blocks; ++block)
      global_nb_elements += block_list[block].nb_elems;

    // All the blocks at the start of the direction to partition in
    const Uint nb_existing_partitions = block_distribution.size()-1;
    for(Uint existing_partition = 0; existing_partition != nb_existing_partitions; ++existing_partition)
    {
      const common::Table<Real>::ArrayT& block_coordinates = points->array();
      BlocksPartitioning blocks_to_partition = blocks_in;
      BlockLayer layer;
      build_block_layer(direction, start_direction, transverse_directions, existing_partition, layer);

      print_vector(CFdebug << "Examining block layer: ", layer.local_layer); CFdebug << CFendl;

      // Size of one partition
      const Uint partition_size = static_cast<Uint>( ceil( static_cast<Real>(global_nb_elements) / static_cast<Real>(nb_partitions) ) );

      Uint nb_partitioned = 0;
      Uint block_layer_offset = 0;
      for(Uint partition = 0; partition != nb_partitions; ++partition)
      {
        // Update block distribution
        blocks_out.block_distribution.push_back(blocks_out.block_points.size());

        Uint partition_remaining_size = partition == (nb_partitions-1) ? global_nb_elements-nb_partitioned : partition_size;
        while(partition_remaining_size)
        {
          // Get the total size of a slice of elements in the current direction
          cf3_assert(layer.segment_size);
          Uint partition_nb_slices = static_cast<Uint>( ceil( static_cast<Real>(partition_remaining_size) / static_cast<Real>(layer.segment_size) ) );
          cf3_assert(partition_nb_slices);
          if((nb_partitioned + (partition_nb_slices * layer.segment_size)) >= global_nb_elements)
          {
            const Uint nb_remaining_elements = global_nb_elements - nb_partitioned;
            partition_nb_slices = nb_remaining_elements / layer.segment_size;
            partition_remaining_size = global_nb_elements - nb_partitioned;
          }

          bool is_first_layer = false;
          bool is_last_layer = false;

          if(block_layer_offset == 0)
            is_first_layer = true;

          if(layer.remaining_nb_segments > partition_nb_slices) // block is larger than the remaining number of slices
          {
            block_layer_offset += partition_nb_slices;
            BOOST_FOREACH(const Uint block_idx, layer.local_layer)
            {
              common::Table<Real>::ArrayT mapped_coords;
              detail::create_mapped_coords(blocks_in.block_subdivisions[block_idx][direction],
                                           &blocks_in.block_gradings[block_idx][mapped_stride*direction], mapped_coords, mapped_stride);

              //Adjust gradings and nodes
              BlocksPartitioning::GradingT new_gradings = blocks_in.block_gradings[block_idx];
              for(Uint i = 0; i != mapped_stride; ++i)
              {
                const Uint original_end_node_idx = blocks_in.block_points[block_idx][end_face_nodes[i]];
                const Uint original_start_node_idx = blocks_in.block_points[block_idx][start_face_nodes[dimensions == 3 ? ( (i == 0 || i == 2) ? i : (i == 3 ? 1 : 3) ) : (i == 0 ? 1 : 0)]];
                const Uint grading_idx = dimensions == 3 ? ( (end_direction != Hexa::ETA_POS || i == 0 || i == 3) ? i : (i == 1 ? 3 : 2) ) : (end_direction != YPOS_2D) ? i : (i == 0 ? 1 : 0);

                if(!node_is_mapped[partition+1][original_end_node_idx])
                {
                  node_is_mapped[partition+1][original_end_node_idx] = true;
                  node_is_mapped[partition+1][original_start_node_idx] = true;
                  CFdebug << "mapping node " << original_end_node_idx << " to " << blocks_out.points.size() << CFendl;
                  node_mapping[partition+1][original_end_node_idx] = blocks_out.points.size();
                  node_mapping[partition+1][original_start_node_idx] = blocks_out.points.size();
                  // Get new block node coords
                  Line1D::MappedCoordsT mapped_coord;
                  mapped_coord << mapped_coords[block_layer_offset][grading_idx];

                  const BlocksPartitioning::PointT& old_node = blocks_in.points[original_end_node_idx];

                  Line1D::NodesT block_nodes;
                  block_nodes(0, XX) = block_coordinates[original_start_node_idx][direction];
                  block_nodes(1, XX) = block_coordinates[original_end_node_idx][direction];
                  Line1D::SF::ValueT sf_1d;
                  Line1D::SF::compute_value(mapped_coord, sf_1d);
                  const Line1D::CoordsT node_1d = sf_1d * block_nodes;

                  blocks_out.points.push_back(BlocksPartitioning::PointT(dimensions));
                  for(Uint i = 0; i != dimensions; ++i)
                  {
                    blocks_out.points.back()[i] = direction == i ? node_1d[XX] : old_node[i];
                  }
                }

                // Adjust the gradings
                new_gradings[mapped_stride*direction + i] =   (mapped_coords[partition_nb_slices][grading_idx] - mapped_coords[partition_nb_slices - 1][grading_idx])
                / (mapped_coords[1][grading_idx] - mapped_coords[0][grading_idx]);
                blocks_to_partition.block_gradings[block_idx][mapped_stride*direction + i] = (mapped_coords[layer.nb_segments][grading_idx] - mapped_coords[layer.nb_segments - 1][grading_idx])
                / (mapped_coords[partition_nb_slices + 1][grading_idx] - mapped_coords[partition_nb_slices][grading_idx]);
              }

              // Adjust number of segments
              BlocksPartitioning::CountsT new_block_subdivisions = blocks_to_partition.block_subdivisions[block_idx];
              new_block_subdivisions[direction] = partition_nb_slices;
              blocks_to_partition.block_subdivisions[block_idx][direction] -= partition_nb_slices;

              // append data to the output
              blocks_out.block_gradings.push_back(new_gradings);
              blocks_out.block_subdivisions.push_back(new_block_subdivisions);
            }

            // All slices are immediatly accounted for
            nb_partitioned += partition_nb_slices * layer.segment_size;
            partition_remaining_size = 0;
            layer.remaining_nb_segments -= partition_nb_slices;
          }
          else // blocks fits entirely into the partition
          {
            is_last_layer = true;
            const Uint nb_added = layer.segment_size * (layer.remaining_nb_segments);
            nb_partitioned += nb_added;
            cf3_assert(nb_added <= partition_remaining_size);
            partition_remaining_size -= nb_added;
            BOOST_FOREACH(const Uint block_idx, layer.local_layer)
            {
              blocks_out.block_gradings.push_back(blocks_to_partition.block_gradings[block_idx]);
              blocks_out.block_subdivisions.push_back(blocks_to_partition.block_subdivisions[block_idx]);
            }
          }

          // Set the block nodes of the previous layer
          BOOST_FOREACH(const Uint block_idx, layer.local_layer)
          {
            CFdebug << "start node mapping for block " << blocks_out.block_points.size() << ":" << CFendl;
            std::vector<Uint> new_block_points(2*mapped_stride);
            BOOST_FOREACH(const Uint i, start_face_nodes)
            {
              new_block_points[i] = is_first_layer ? blocks_in.block_points[block_idx][i] : node_mapping[partition][blocks_in.block_points[block_idx][i]];
              CFdebug << "  " << blocks_in.block_points[block_idx][i] << " --> " << new_block_points[i] << CFendl;
            }
            CFdebug << "end node mapping for block " << blocks_out.block_points.size() << ":" << CFendl;
            BOOST_FOREACH(const Uint i, end_face_nodes)
            {
              new_block_points[i] = is_last_layer ? blocks_in.block_points[block_idx][i] : node_mapping[partition+1][blocks_in.block_points[block_idx][i]];
              CFdebug << "  " << blocks_in.block_points[block_idx][i] << " --> " << new_block_points[i] << CFendl;
            }

            blocks_out.block_points.push_back(new_block_points);

            // Add new patches
            BOOST_FOREACH(const Uint transverse_direction, transverse_directions)
            {
              const CFaceConnectivity::ElementReferenceT adjacent_element = volume_to_face_connectivity.adjacent_element(block_idx, transverse_direction);
              if(adjacent_element.first->element_type().dimensionality() < dimensions)
              {
                const Uint patch_idx = patch_idx_map[adjacent_element.first->parent()->name()];
                BOOST_FOREACH(const Uint i, etype_faces.nodes_range(transverse_direction))
                {
                  blocks_out.patch_points[patch_idx].push_back(new_block_points[i]);
                }
              }
            }
          }

          if(is_last_layer)
          {
            block_layer_offset = 0;
            build_block_layer(direction, start_direction, transverse_directions, existing_partition, layer);
            print_vector(CFdebug << "Examining block layer: ", layer.local_layer); CFdebug << CFendl;
          }
        }
      }
    }

    blocks_out.block_distribution.push_back(blocks_out.block_points.size());

    // Preserve original start and end patches
    const BlocksPartitioning::IndicesT start_end_directions = boost::assign::list_of(start_direction)(end_direction);
    for(Uint block_idx = 0; block_idx != nb_blocks; ++block_idx)
    {
      BOOST_FOREACH(const Uint lengthwise_direction, start_end_directions)
      {
        const CFaceConnectivity::ElementReferenceT adjacent_element = volume_to_face_connectivity.adjacent_element(block_idx, lengthwise_direction);
        if(adjacent_element.first->element_type().dimensionality() < dimensions)
        {
          CFdebug << "adding end patch " << adjacent_element.first->parent()->name() << " to block " << block_idx << CFendl;
          const Uint patch_idx = patch_idx_map[adjacent_element.first->parent()->name()];
          BOOST_FOREACH(const Uint i, etype_faces.nodes_range(lengthwise_direction))
          {
            blocks_out.patch_points[patch_idx].push_back(blocks_in.block_points[block_idx][i]);
          }
        }
      }
    }

    Uint nb_elems_out = 0;
    BOOST_FOREACH(const std::vector<Uint> segments, blocks_out.block_subdivisions)
    {
      if(dimensions == 2)
        nb_elems_out += segments[0]*segments[1];
      else
        nb_elems_out += segments[0]*segments[1]*segments[2];
    }
    cf3_assert(nb_elems_out == global_nb_elements);

    update_blocks(blocks_out);
  }

  /// Helper data to construct the mesh connectivity
  std::vector<Block> block_list;
  std::map<std::string, boost::ptr_vector<Patch> > patch_map;
  /// Distribution of nodes across the CPUs
  std::vector<Uint> nodes_dist;
  Uint local_nodes_begin;
  Uint local_nodes_end;
  Uint ghost_counter;
  typedef std::map<Uint, Uint> IndexMapT;
  IndexMapT global_to_local;
  std::vector<Uint> block_distribution;
};

BlockArrays::BlockArrays(const std::string& name) :
  Component(name),
  m_implementation(new Implementation())
{
  m_implementation->patches = create_static_component<Group>("Patches");

  regist_signal( "create_points" )
    .connect( boost::bind( &BlockArrays::signal_create_points, this, _1 ) )
    .description("Create an array holding the points")
    .pretty_name("Create Points")
    .signature( boost::bind ( &BlockArrays::signature_create_points, this, _1) );

  regist_signal( "create_blocks" )
    .connect( boost::bind( &BlockArrays::signal_create_blocks, this, _1 ) )
    .description("Create an array holding the block definitions (node connectivity")
    .pretty_name("Create Blocks")
    .signature( boost::bind ( &BlockArrays::signature_create_blocks, this, _1) );

  regist_signal( "create_block_subdivisions" )
    .connect( boost::bind( &BlockArrays::signal_create_block_subdivisions, this, _1 ) )
    .description("Create an array holding the block subdivisions")
    .pretty_name("Create Block Subdivisions");

  regist_signal( "create_block_gradings" )
    .connect( boost::bind( &BlockArrays::signal_create_block_gradings, this, _1 ) )
    .description("Create an array holding the block gradings")
    .pretty_name("Create Block Gradings");

  regist_signal( "create_patch_nb_faces" )
    .connect( boost::bind( &BlockArrays::signal_create_patch_nb_faces, this, _1 ) )
    .description("Create an array holding the faces for a patch")
    .pretty_name("Create Patch")
    .signature( boost::bind ( &BlockArrays::signature_create_patch_nb_faces, this, _1) );

  regist_signal( "create_patch_face_list" )
    .connect( boost::bind( &BlockArrays::signal_create_patch_face_list, this, _1 ) )
    .description("Create an array holding the faces for a patch")
    .pretty_name("Create Patch From Faces")
    .signature( boost::bind ( &BlockArrays::signature_create_patch_face_list, this, _1) );

  regist_signal( "create_block_mesh" )
    .connect( boost::bind( &BlockArrays::signal_create_block_mesh, this, _1 ) )
    .description("Create a mesh that only contains the inner blocks. Surface patches are in a single region and numbered for passing to create_patch.")
    .pretty_name("Create Inner Blocks");

  regist_signal( "partition_blocks" )
    .connect( boost::bind( &BlockArrays::signal_partition_blocks, this, _1 ) )
    .description("Partition the blocks for parallel mesh generation")
    .pretty_name("Partition Blocks")
    .signature( boost::bind(&BlockArrays::signature_partition_blocks, this, _1) );

  regist_signal( "extrude_blocks" )
    .connect( boost::bind( &BlockArrays::signal_extrude_blocks, this, _1 ) )
    .description("Extrude a 2D mesh in a number of spanwise (Z-direction) blocks. The number of spanwise blocks is determined by the size of the passed arguments")
    .pretty_name("Extrude Blocks")
    .signature( boost::bind(&BlockArrays::signature_extrude_blocks, this, _1) );

  regist_signal( "create_mesh" )
    .connect( boost::bind( &BlockArrays::signal_create_mesh, this, _1 ) )
    .description("Create the final mesh.")
    .pretty_name("Create Mesh")
    .signature( boost::bind(&BlockArrays::signature_create_mesh, this, _1) );

  options().add_option("block_distribution", std::vector<Uint>())
    .pretty_name("Block Distribution")
    .description("The distribution of the blocks among CPUs in a parallel simulation")
    .link_to(&m_implementation->block_distribution)
    .attach_trigger(boost::bind(&Implementation::trigger_block_distribution, m_implementation.get()));

  options().add_option("overlap", 1u).pretty_name("Overlap")
    .description("Number of cell layers to overlap across parallel partitions. Ignored in serial runs");
}

BlockArrays::~BlockArrays()
{
}

Handle< Table< Real > > BlockArrays::create_points(const Uint dimensions, const Uint nb_points)
{
  cf3_assert(is_null(m_implementation->points));
  if(dimensions != 2 && dimensions != 3)
    throw BadValue(FromHere(), "BlockArrays dimension must be 2 or 3, but " + to_str(dimensions) + " was given");
  m_implementation->points = create_component< Table<Real> >("Points");
  m_implementation->points->set_row_size(dimensions);
  m_implementation->points->resize(nb_points);

  return m_implementation->points;
}

Handle< Table< Uint > > BlockArrays::create_blocks(const Uint nb_blocks)
{
  cf3_assert(is_null(m_implementation->blocks));
  m_implementation->blocks = create_component< Table<Uint> >("Blocks");

  const Uint dimensions = m_implementation->points->row_size();

  m_implementation->blocks->set_row_size(dimensions == 3 ? 8 : 4);
  m_implementation->blocks->resize(nb_blocks);

  // Make sure we update the default block distribution, if needed
  m_implementation->trigger_block_distribution();

  return m_implementation->blocks;
}

Handle< Table< Uint > > BlockArrays::create_block_subdivisions()
{
  cf3_assert(is_null(m_implementation->block_subdivisions));
  m_implementation->block_subdivisions = create_component< Table<Uint> >("BlockSubdivisions");

  const Uint dimensions = m_implementation->points->row_size();
  const Uint nb_blocks = m_implementation->blocks->size();

  m_implementation->block_subdivisions->set_row_size(dimensions);
  m_implementation->block_subdivisions->resize(nb_blocks);

  return m_implementation->block_subdivisions;
}

Handle< Table< Real > > BlockArrays::create_block_gradings()
{
  cf3_assert(is_null(m_implementation->block_gradings));
  m_implementation->block_gradings = create_component< Table<Real> >("BlockGradings");

  const Uint dimensions = m_implementation->points->row_size();
  const Uint nb_blocks = m_implementation->blocks->size();

  m_implementation->block_gradings->set_row_size(dimensions == 3 ? 12 : 4);
  m_implementation->block_gradings->resize(nb_blocks);

  return m_implementation->block_gradings;
}

Handle< Table<Uint> > BlockArrays::create_patch(const std::string& name, const Uint nb_faces)
{
  Handle< Table<Uint> > result = m_implementation->patches->create_component< Table<Uint> >(name);

  const Uint dimensions = m_implementation->points->row_size();
  result->set_row_size(dimensions == 3 ? 4 : 2);
  result->resize(nb_faces);

  return result;
}

Handle< Table< Uint > > BlockArrays::create_patch(const std::string& name, const std::vector< Uint >& face_indices)
{
  if(is_null(m_implementation->default_shell_connectivity))
    throw SetupError(FromHere(), "Adding a patch using face indices requires a default patch. Call the create_block_mesh signal first.");

  const Uint nb_faces = face_indices.size();
  Handle< Table<Uint> > result = create_patch(name, nb_faces);
  Table<Uint>& patch = *result;

  const Table<Uint>& default_shell = *m_implementation->default_shell_connectivity;
  for(Uint i = 0; i != nb_faces; ++i)
  {
    patch[i] = default_shell[face_indices[i]];
  }

  return result;
}

Handle< Mesh > BlockArrays::create_block_mesh()
{
  m_implementation->block_mesh = create_component<Mesh>("InnerBlockMesh");

  const Uint nb_nodes = m_implementation->points->size();
  const Uint dimensions = m_implementation->points->row_size();
  const Uint nb_blocks = m_implementation->blocks->size();

  // root region and coordinates
  Region& block_mesh_region = m_implementation->block_mesh->topology().create_region("block_mesh_region");
  m_implementation->block_mesh->initialize_nodes(nb_nodes, dimensions);
  Dictionary& geometry_dict = m_implementation->block_mesh->geometry_fields();
  geometry_dict.coordinates().array() = m_implementation->points->array();

  // Define the volume cells, i.e. the blocks
  Elements& block_elements = block_mesh_region.create_region("blocks").create_elements(dimensions == 3 ? "cf3.mesh.LagrangeP1.Hexa3D" : "cf3.mesh.LagrangeP1.Quad2D", geometry_dict);
  block_elements.resize(nb_blocks);
  block_elements.geometry_space().connectivity().array() = m_implementation->blocks->array();

  // Define surface patches
  Region& boundary = m_implementation->block_mesh->topology().create_region("boundary");
  boost_foreach(const Table<Uint>& patch_connectivity_table, find_components< Table<Uint> >(*m_implementation->patches))
  {
    Elements& patch_elems = boundary.create_region(patch_connectivity_table.name()).create_elements(dimensions == 3 ? "cf3.mesh.LagrangeP1.Quad3D" : "cf3.mesh.LagrangeP1.Line2D", geometry_dict);
    patch_elems.resize(patch_connectivity_table.size());
    patch_elems.geometry_space().connectivity().array() = patch_connectivity_table.array();
  }

  // Create connectivity data
  CNodeConnectivity& node_connectivity = *m_implementation->block_mesh->create_component<CNodeConnectivity>("node_connectivity");
  node_connectivity.initialize(find_components_recursively<Elements>(*m_implementation->block_mesh));
  m_implementation->face_connectivity = block_elements.create_component<CFaceConnectivity>("face_connectivity");
  m_implementation->face_connectivity->initialize(node_connectivity);

  const Uint nb_faces = dimensions == 3 ? LagrangeP1::Hexa3D::nb_faces : LagrangeP1::Quad2D::nb_faces;
  const ElementType::FaceConnectivity& faces = dimensions == 3  ? LagrangeP1::Hexa3D::faces() : LagrangeP1::Quad2D::faces();
  const Uint face_stride = dimensions == 3 ? 4 : 2;

  // Region for default the shell, i.e. all non-defined patches
  Elements& default_shell_elems = boundary.create_region("default_patch").create_elements(dimensions == 3 ? "cf3.mesh.LagrangeP1.Quad3D" : "cf3.mesh.LagrangeP1.Line2D", geometry_dict);
  Connectivity& default_shell_connectivity = default_shell_elems.geometry_space().connectivity();
  m_implementation->default_shell_connectivity = default_shell_connectivity.handle<Connectivity>();

  Uint nb_shell_faces = 0;

  // Count number of shell faces
  for(Uint block_idx = 0; block_idx != nb_blocks; ++block_idx)
  {
    for(Uint face_idx = 0; face_idx != nb_faces; ++face_idx)
    {
      if(!m_implementation->face_connectivity->has_adjacent_element(block_idx, face_idx))
        ++nb_shell_faces;
    }
  }

  default_shell_elems.resize(nb_shell_faces);
  const Connectivity& cell_connectivity = block_elements.geometry_space().connectivity();

  // Fill the default shell connectivity
  Uint shell_idx = 0;
  for(Uint block_idx = 0; block_idx != nb_blocks; ++block_idx)
  {
    for(Uint face_idx = 0; face_idx != nb_faces; ++face_idx)
    {
      if(!m_implementation->face_connectivity->has_adjacent_element(block_idx, face_idx))
      {
        Table<Uint>::Row conn_row = default_shell_connectivity[shell_idx++];
        for(Uint i  = 0; i != face_stride; ++i)
        {
          conn_row[i] = cell_connectivity[block_idx][faces.nodes[face_idx*face_stride+i]];
        }
      }
    }
  }

  // Create a field containing the indices of the unassigned patches
  Dictionary& elems_P0 = m_implementation->block_mesh->create_discontinuous_space("elems_P0","cf3.mesh.LagrangeP0", std::vector< Handle<Entities> >(1, default_shell_elems.handle<Entities>()));
  Field& shell_face_indices = elems_P0.create_field("shell_face_index");
  const Space& shell_space = elems_P0.space(default_shell_elems);
  for(Uint i =0; i != nb_shell_faces; ++i)
  {
    const Uint field_idx = shell_space.connectivity()[i][0];
    shell_face_indices[field_idx][0] = i;
  }

  // Create a field containing the ranks of the blocks
  Dictionary& block_elems_P0 = m_implementation->block_mesh->create_discontinuous_space("blocks_P0","cf3.mesh.LagrangeP0", std::vector< Handle<Entities> >(1, block_elements.handle<Entities>()));
  Field& block_ranks = block_elems_P0.create_field("block_rank");
  const Space& block_space = block_elems_P0.space(block_elements);
  const Uint nb_partitions = m_implementation->block_distribution.size()-1;
  for(Uint part = 0; part != nb_partitions; ++part)
  {
    const Uint blocks_begin = m_implementation->block_distribution[part];
    const Uint blocks_end = m_implementation->block_distribution[part+1];
    for(Uint i = blocks_begin; i != blocks_end; ++i)
    {
      const Uint field_idx = block_space.connectivity()[i][0];
      block_ranks[field_idx][0] = part;
    }
  }

  return m_implementation->block_mesh;
}

void BlockArrays::partition_blocks(const Uint nb_partitions, const Uint direction)
{
  // Check user-supplied data
  m_implementation->check_handle(m_implementation->points, "create_points", "Points definition");
  m_implementation->check_handle(m_implementation->blocks, "create_blocks", "Blocks definition");
  m_implementation->check_handle(m_implementation->block_subdivisions, "create_block_subdivisions", "Block subdivisions");
  m_implementation->check_handle(m_implementation->block_gradings, "create_block_gradings", "Block gradings");

  create_block_mesh();
  m_implementation->create_blocks();
  m_implementation->partition_blocks(nb_partitions, direction);

  // The algorithm just modifies the linked parameter, so we need to reset the option to keep it in sync
  options().configure_option("block_distribution", m_implementation->block_distribution);
}

void BlockArrays::extrude_blocks(const std::vector<Real>& positions, const std::vector< Uint >& nb_segments, const std::vector< Real >& gradings)
{
  if(m_implementation->points->row_size() != 2)
    throw SetupError(FromHere(), "Only 2D block data cn be extuded");
  const Uint nb_layers = positions.size();
  if(nb_segments.size() != nb_layers || gradings.size() != nb_layers)
    throw SetupError(FromHere(), "Arguments passed to extrude don't have the same size");

  Table<Real>& points_3d = *m_implementation->points;
  Table<Uint>& block_points_3d = *m_implementation->blocks;
  Table<Uint>& subdivisions_3d = *m_implementation->block_subdivisions;
  Table<Real>& gradings_3d = *m_implementation->block_gradings;
  Table<Real>::ArrayT gradings_2d = gradings_3d.array(); // copy, since the ordering is not preserved

  // Resize the original points
  const Uint nb_points_2d = points_3d.size();
  const Uint nb_points_3d = (nb_layers+1)*nb_points_2d;
  points_3d.array().resize(boost::extents[nb_points_3d][3]);

  // Convert original points to 3D
  for(Uint i = 0; i != nb_points_2d; ++i)
  {
    points_3d[i][2] = 0.; // Old layer defaults at position 0
  }

  // Add new points
  for(Uint layer = 0; layer != nb_layers; ++layer)
  {
    for(Uint i = 0; i != nb_points_2d; ++i)
    {
      const Uint new_point_idx = i+(layer+1)*nb_points_2d;
      points_3d.set_row(new_point_idx, points_3d[i]);
      points_3d[new_point_idx][2] = positions[layer];
    }
  }

  // resize blocks
  const Uint nb_blocks_2d = block_points_3d.size();
  block_points_3d.array().resize(boost::extents[nb_blocks_2d*nb_layers][8]);
  subdivisions_3d.array().resize(boost::extents[nb_blocks_2d*nb_layers][3]);
  gradings_3d.array().resize(boost::extents[nb_blocks_2d*nb_layers][12]);

  // Create new blocks
  for(Uint layer = 0; layer != nb_layers; ++layer)
  {
    for(Uint i = 0; i != nb_blocks_2d; ++i)
    {
      const Uint new_block_idx = layer*nb_blocks_2d+i;
      const Table<Uint>::ConstRow original_block_row = block_points_3d[i];
      Table<Uint>::Row new_block_row = block_points_3d[new_block_idx];
      const Uint first_layer_offset = nb_points_2d*layer;
      const Uint second_layer_offset = first_layer_offset + nb_points_2d;
      new_block_row[0] = original_block_row[0]+first_layer_offset;
      new_block_row[1] = original_block_row[1]+first_layer_offset;
      new_block_row[2] = original_block_row[2]+first_layer_offset;
      new_block_row[3] = original_block_row[3]+first_layer_offset;
      new_block_row[4] = original_block_row[0]+second_layer_offset;
      new_block_row[5] = original_block_row[1]+second_layer_offset;
      new_block_row[6] = original_block_row[2]+second_layer_offset;
      new_block_row[7] = original_block_row[3]+second_layer_offset;

      const Table<Uint>::ConstRow original_subdiv_row = subdivisions_3d[i];
      Table<Uint>::Row new_subdiv_row = subdivisions_3d[new_block_idx];
      new_subdiv_row[0] = original_subdiv_row[0];
      new_subdiv_row[1] = original_subdiv_row[1];
      new_subdiv_row[2] = nb_segments[layer];

      const Table<Real>::ConstRow original_grading_row = gradings_2d[i];
      Table<Real>::Row new_grading_row = gradings_3d[new_block_idx];
      new_grading_row[0] = original_grading_row[0];
      new_grading_row[1] = original_grading_row[0];
      new_grading_row[2] = original_grading_row[1];
      new_grading_row[3] = original_grading_row[1];
      new_grading_row[4] = original_grading_row[2];
      new_grading_row[5] = original_grading_row[3];
      new_grading_row[6] = original_grading_row[2];
      new_grading_row[7] = original_grading_row[3];
      new_grading_row[8] = gradings[layer];
      new_grading_row[9] = gradings[layer];
      new_grading_row[10] = gradings[layer];
      new_grading_row[11] = gradings[layer];
    }
  }

  // Adjust side patches
  BOOST_FOREACH(Table<Uint>& patch, find_components< Table<Uint> >(*m_implementation->patches))
  {
    const Uint patch_size_2d = patch.size();
    patch.array().resize(boost::extents[patch_size_2d*nb_layers][4]);
    for(Uint layer = 0; layer != nb_layers; ++layer)
    {
      for(Uint i = 0; i != patch_size_2d; ++i)
      {
        const Table<Uint>::ConstRow original_patch_row = patch[i];
        Table<Uint>::Row new_patch_row = patch[patch_size_2d*layer + i];
        const Uint first_layer_offset = nb_points_2d*layer;
        const Uint second_layer_offset = first_layer_offset + nb_points_2d;
        new_patch_row[0] = original_patch_row[0]+first_layer_offset;
        new_patch_row[1] = original_patch_row[1]+first_layer_offset;
        new_patch_row[2] = original_patch_row[1]+second_layer_offset;
        new_patch_row[3] = original_patch_row[0]+second_layer_offset;
      }
    }
  }

  // add front and back patches
  Table<Uint>& front_patch = *create_patch("front", nb_blocks_2d);
  Table<Uint>& back_patch = *create_patch("back", nb_blocks_2d);
  for(Uint i = 0; i != nb_blocks_2d; ++i)
  {
    std::reverse_copy(block_points_3d[i].begin(), block_points_3d[i].begin()+4, front_patch[i].begin());
    std::copy(block_points_3d[(nb_layers-1)*nb_blocks_2d+i].begin()+4, block_points_3d[(nb_layers-1)*nb_blocks_2d+i].begin()+8, back_patch[i].begin());
  }

  m_implementation->trigger_block_distribution();
}

void BlockArrays::create_mesh(Mesh& mesh)
{
  // Check user-supplied data
  m_implementation->check_handle(m_implementation->points, "create_points", "Points definition");
  m_implementation->check_handle(m_implementation->blocks, "create_blocks", "Blocks definition");
  m_implementation->check_handle(m_implementation->block_subdivisions, "create_block_subdivisions", "Block subdivisions");
  m_implementation->check_handle(m_implementation->block_gradings, "create_block_gradings", "Block gradings");

  const Table<Real>& points = *m_implementation->points;
  const Table<Uint>& blocks = *m_implementation->blocks;
  const Table<Uint>& block_subdivisions =  *m_implementation->block_subdivisions;

  common::Timer timer;

  // Make sure the block connectivity mesh is up-to-date
  create_block_mesh();

  const Uint nb_procs = PE::Comm::instance().size();
  const Uint rank = PE::Comm::instance().rank();
  const Uint dimensions = points.row_size();

  // Block connectivity helper data
  m_implementation->create_blocks();

  m_implementation->create_nodes_distribution(nb_procs, rank);

  // Element distribution among CPUs
  std::vector<Uint> elements_dist;
  elements_dist.reserve(nb_procs+1);
  elements_dist.push_back(0);
  for(Uint proc = 0; proc != nb_procs; ++proc)
  {
    const Uint proc_begin = m_implementation->block_distribution[proc];
    const Uint proc_end = m_implementation->block_distribution[proc+1];
    Uint nb_elements = 0;
    for(Uint block = proc_begin; block != proc_end; ++block)
    {
      nb_elements += m_implementation->block_list[block].nb_elems;
    }
    elements_dist.push_back(elements_dist.back() + nb_elements);
  }

  const Uint blocks_begin = m_implementation->block_distribution[rank];
  const Uint blocks_end = m_implementation->block_distribution[rank+1];

  Dictionary& geometry_dict = mesh.geometry_fields();
  Elements& volume_elements = mesh.topology().create_region("interior").create_elements(dimensions == 3 ? "cf3.mesh.LagrangeP1.Hexa3D" : "cf3.mesh.LagrangeP1.Quad2D", geometry_dict);
  volume_elements.resize(elements_dist[rank+1]-elements_dist[rank]);

  // Set the connectivity, this also updates ghost node indices
  Uint element_idx = 0; // global element index
  for(Uint block_idx = blocks_begin; block_idx != blocks_end; ++block_idx)
  {
    m_implementation->add_block(block_subdivisions[block_idx], block_idx, volume_elements.geometry_space().connectivity(), element_idx);
  }

  const Uint nodes_begin = m_implementation->nodes_dist[rank];
  const Uint nodes_end = m_implementation->nodes_dist[rank+1];
  const Uint nb_nodes_local = nodes_end - nodes_begin;

  // Initialize coordinates
  mesh.initialize_nodes(nb_nodes_local + m_implementation->ghost_counter, dimensions);
  Field& coordinates = mesh.geometry_fields().coordinates();

  // Fill the coordinate array
  for(Uint block_idx = blocks_begin; block_idx != blocks_end; ++block_idx)
  {
    if(dimensions == 3)
      m_implementation->fill_block_coordinates_3d<Hexa3D>(coordinates, block_idx);
    if(dimensions == 2)
      m_implementation->fill_block_coordinates_2d<Quad2D>(coordinates, block_idx);
  }

  // Add surface patches
  boost_foreach(const Component& patch_description, *m_implementation->patches)
  {
    m_implementation->add_patch
    (
      patch_description.name(),
      mesh.topology().create_region(patch_description.name()).create_elements(dimensions == 3 ? "cf3.mesh.LagrangeP1.Quad3D" : "cf3.mesh.LagrangeP1.Line2D", geometry_dict)
    );
  }

  // surface patches shouldn't have introduced new ghosts
  cf3_assert(coordinates.size() == nb_nodes_local + m_implementation->ghost_counter);

  if(PE::Comm::instance().is_active())
  {
    common::List<Uint>& gids = mesh.geometry_fields().glb_idx(); gids.resize(nb_nodes_local + m_implementation->ghost_counter);
    common::List<Uint>& ranks = mesh.geometry_fields().rank(); ranks.resize(nb_nodes_local + m_implementation->ghost_counter);

    // Local nodes
    for(Uint i = 0; i != nb_nodes_local; ++i)
    {
      gids[i] = i + nodes_begin;
      ranks[i] = rank;
    }

    // Ghosts
    for(Implementation::IndexMapT::const_iterator ghost_it = m_implementation->global_to_local.begin(); ghost_it != m_implementation->global_to_local.end(); ++ghost_it)
    {
      const Uint global_id = ghost_it->first;
      const Uint local_id = ghost_it->second;
      gids[local_id] = global_id;
      ranks[local_id] = std::upper_bound(m_implementation->nodes_dist.begin(), m_implementation->nodes_dist.end(), global_id) - 1 - m_implementation->nodes_dist.begin();
    }

    mesh.geometry_fields().coordinates().parallelize_with(mesh.geometry_fields().comm_pattern());
    mesh.geometry_fields().coordinates().synchronize();
  }
  else
  {
    cf3_assert(m_implementation->ghost_counter == 0);
  }

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

  const Uint overlap = options().option("overlap").value<Uint>();
  if(overlap != 0 && PE::Comm::instance().size() > 1)
  {
    MeshTransformer& global_conn = *Handle<MeshTransformer>(create_component("GlobalConnectivity", "cf3.mesh.actions.GlobalConnectivity"));
    global_conn.transform(mesh);

    MeshTransformer& grow_overlap = *Handle<MeshTransformer>(create_component("GrowOverlap", "cf3.mesh.actions.GrowOverlap"));
    for(Uint i = 0; i != overlap; ++i)
      grow_overlap.transform(mesh);

    mesh.geometry_fields().remove_component("CommPattern");
  }

  // Raise an event to indicate that a mesh was loaded happened
  mesh.raise_mesh_loaded();
}

void BlockArrays::signature_partition_blocks(SignalArgs& args)
{
  SignalOptions options(args);
  options.add_option("nb_partitions", PE::Comm::instance().size()).pretty_name("Nb. Partitions").description("Number of partitions");
  options.add_option("direction", 0u).pretty_name("Direction").description("Partitioning direction (0=X, 1=Y, 2=Z)");
}

void BlockArrays::signal_partition_blocks(SignalArgs& args)
{
  SignalOptions options(args);
  partition_blocks(options["nb_partitions"].value<Uint>(), options["direction"].value<Uint>());
}



void BlockArrays::signature_create_points(SignalArgs& args)
{
  SignalOptions options(args);
  options.add_option("dimensions", 3u).pretty_name("Dimensions").description("The physical dimensions for the mesh (must be 2 or 3)");
  options.add_option("nb_points", 0u).pretty_name("Number of points").description("The number of points needed to define the blocks");
}

void BlockArrays::signal_create_points(SignalArgs& args)
{
  SignalOptions options(args);
  create_points(options.option("dimensions").value<Uint>(), options.option("nb_points").value<Uint>());
  detail::create_reply(args, *m_implementation->points);
}

void BlockArrays::signature_create_blocks(SignalArgs& args)
{
  SignalOptions options(args);
  options.add_option("nb_blocks", 0u).pretty_name("Number of blocks").description("The number of blocks that are needed");
}

void BlockArrays::signal_create_blocks(SignalArgs& args)
{
  SignalOptions options(args);
  create_blocks(options.option("nb_blocks").value<Uint>());
  detail::create_reply(args, *m_implementation->blocks);
}

void BlockArrays::signal_create_block_subdivisions(SignalArgs& args)
{
  create_block_subdivisions();
  detail::create_reply(args, *m_implementation->block_subdivisions);
}

void BlockArrays::signal_create_block_gradings(SignalArgs& args)
{
  create_block_gradings();
  detail::create_reply(args, *m_implementation->block_gradings);
}

void BlockArrays::signature_create_patch_nb_faces(SignalArgs& args)
{
  SignalOptions options(args);
  options.add_option("name", "Default").pretty_name("Patch Name").description("The name for the created patch");
  options.add_option("nb_faces", 0u).pretty_name("Number of faces").description("The number of faces (of individual blocks) that make up the patch");
}

void BlockArrays::signal_create_patch_nb_faces(SignalArgs& args)
{
  SignalOptions options(args);
  const Handle< Table<Uint> > result = create_patch(options.option("name").value<std::string>(), options.option("nb_faces").value<Uint>());
  detail::create_reply(args, *result);
}

void BlockArrays::signature_create_patch_face_list(SignalArgs& args)
{
  SignalOptions options(args);
  options.add_option("name", "Default").pretty_name("Patch Name").description("The name for the created patch");
  options.add_option("face_list", std::vector<Uint>()).pretty_name("Face List").description("The list of faces that make up the patch. Numbers are as given in the default patch");
}

void BlockArrays::signal_create_patch_face_list(SignalArgs& args)
{
  SignalOptions options(args);
  const Handle< Table<Uint> > result = create_patch(options.option("name").value<std::string>(), options.option("face_list").value< std::vector<Uint> >());
  detail::create_reply(args, *result);
}

void BlockArrays::signal_create_block_mesh(SignalArgs& args)
{
  detail::create_reply(args, *create_block_mesh());
}

void BlockArrays::signature_extrude_blocks(SignalArgs& args)
{
  SignalOptions options(args);

  options.add_option("positions", std::vector<Real>())
    .pretty_name("Positions")
    .description("Spanwise coordinate for each new spanwise layer of points. Values must ne greater than 0");

  options.add_option("nb_segments", std::vector<Uint>())
    .pretty_name("Nb Segments")
    .description("Number of spanwise segments for each block");

  options.add_option("gradings", std::vector<Real>())
    .pretty_name("Gradings")
    .description("Uniform grading definition in the spanwise direction for each block");
}

void BlockArrays::signal_extrude_blocks(SignalArgs& args)
{
  SignalOptions options(args);
  extrude_blocks(options["positions"].value< std::vector<Real> >(),
                 options["nb_segments"].value< std::vector<Uint> >(),
                 options["gradings"].value< std::vector<Real> >());
}

void BlockArrays::signature_create_mesh(SignalArgs& args)
{
  SignalOptions options(args);
  options.add_option("output_mesh", URI())
    .supported_protocol(cf3::common::URI::Scheme::CPATH)
    .pretty_name("Output Mesh")
    .description("URI to a mesh in which to create the output");
}

void BlockArrays::signal_create_mesh(SignalArgs& args)
{
  SignalOptions options(args);
  Handle<Mesh> mesh(access_component(options["output_mesh"].value<URI>()));
  if(is_null(mesh))
    throw SetupError(FromHere(), "Mesh passed to the create_mesh signal of " + uri().string() + " is invalid");
  create_mesh(*mesh);
}

} // BlockMesh
} // mesh
} // cf3
