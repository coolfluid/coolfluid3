// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/filesystem/fstream.hpp>

#include "common/BoostAssign.hpp"
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
    reply_options.add("created_component", created_component.uri());
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
      mapped_coords[0][edge] = -1.;
      const Real end = mapped_coords[segments][edge];
      cf3_assert(fabs(end-1.) < eps);
      mapped_coords[segments][edge] = 1.;
    }
  }

  // Helper function to convert from a table to the old-style storage
  template<typename ValT, typename RowT>
  void add_row(std::vector<ValT>& output, const RowT& input)
  {
    output.push_back(ValT(input.size()));
    std::copy(input.begin(), input.end(), output.back().begin());
  }

  // Helper function to distribute a count evenly among processes
  void distribute(const Uint begin, const Uint count, const Uint nb_procs, std::vector<Uint>& distribution)
  {
    distribution.clear();
    distribution.reserve(nb_procs+1);
    distribution.push_back(begin);
    const Uint divided = count / nb_procs;
    const Uint remainder = count % nb_procs;
    for(Uint i = 0; i != nb_procs; ++i)
    {
      distribution.push_back(distribution.back() + divided + (i==0 ? remainder : 0));
    }
  }
}

ComponentBuilder < BlockArrays, Component, LibBlockMesh > BlockArrays_Builder;

struct BlockArrays::Implementation
{
  Handle< common::Table<Real> > points;
  Handle< common::Table<Uint> > blocks;
  Handle< common::Table<Uint> > block_subdivisions;
  Handle< common::Table<Real> > block_gradings;

  Handle< common::Group > patches;

  Handle<Mesh> block_mesh;
  Handle<Connectivity> default_shell_connectivity;
  Handle<FaceConnectivity> face_connectivity;

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
      node_strides(dim),
      element_strides(dim)
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
        for(Uint j = 0; j != search_direction; ++j)
          neighbor = neighbor[search_indices[j]];
        neighbor.search_indices.push_back(0);
        return neighbor;
      }

      // We have the data here
      Block result = *this;
      result.search_indices.push_back(i);
      return result;
    }

    /// Get the global node index. Available after "dimension" subsequent calls to operator[]
    Uint global_node_idx() const
    {
      cf3_assert(search_indices.size() == dimensions);
      Uint result = nodes_distribution.front();
      for(Uint i = 0; i != dimensions; ++i)
      {
        result += node_strides[i]*search_indices[i];
      }
      return result;
    }

    /// Get the global element index. Available after "dimension" subsequent calls to operator[]
    Uint global_element_idx() const
    {
      cf3_assert(search_indices.size() == dimensions);
      Uint result = elements_distribution.front();
      for(Uint i = 0; i != dimensions; ++i)
      {
        result += element_strides[i]*search_indices[i];
      }
      return result;
    }

    /// True if the element at the given location is local to the current rank
    bool is_local_element(const Uint i, const Uint j, const Uint k) const
    {
      cf3_assert(dimensions == 3);
      cf3_assert(i < segments[0]);
      cf3_assert(j < segments[1]);
      cf3_assert(k < segments[2]);

      const Uint rank = common::PE::Comm::instance().rank();
      const Uint element_gid = elements_distribution.front() + element_strides[0]*i + element_strides[1]*j + element_strides[2]*k;
      return element_gid >= elements_distribution[rank] && element_gid < elements_distribution[rank+1];
    }

    /// 2D version
    bool is_local_element(const Uint i, const Uint j) const
    {
      cf3_assert(dimensions == 2);
      cf3_assert(i < segments[0]);
      cf3_assert(j < segments[1]);

      const Uint rank = common::PE::Comm::instance().rank();
      const Uint element_gid = elements_distribution.front() + element_strides[0]*i + element_strides[1]*j;
      return element_gid >= elements_distribution[rank] && element_gid < elements_distribution[rank+1];
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
    /// Point strides in each direction
    std::vector<Uint> node_strides;
    /// Element strides in each direction
    std::vector<Uint> element_strides;
    /// Parallel distribution of nodes and elements
    std::vector<Uint> nodes_distribution;
    std::vector<Uint> elements_distribution;
    /// Start and end local IDs
    Uint local_nodes_start;
    Uint local_nodes_end;
  };

  struct Patch
  {
    /// Create a new patch
    /// @param a_block The block that is adjacent to the patch
    /// @param fixed_dir THe component of corrdinates in which all points of the patch lie
    /// @param idx Index of the patch layer in the adjacent block
    /// @param orientation Direction of the patch normal
    Patch(const Block& a_block, const Uint fixed_dir, const Uint idx, const Uint orientation) :
      block(a_block),
      fixed_direction(fixed_dir),
      fixed_idx(idx),
      m_orientation(orientation)
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

    /// Access to the adjacent block, with search indices set correctly. 1D version
    Block adjacent_block(Uint i) const
    {
      cf3_assert(block.dimensions == 2);
      i = fixed_idx ? i : segments[0]-i;
      return block[fixed_direction == 0 ? fixed_idx : i][fixed_direction == 1 ? fixed_idx : i];
    }

    /// Access to the adjacent block, with search indices set correctly. 2D version
    Block adjacent_block(Uint i, Uint j) const
    {
      cf3_assert(block.dimensions == 3);
      if(fixed_direction != 2)
        i = fixed_idx ? i : segments[0]-i;
      else
        i = fixed_idx ? segments[0]-i : i;
      switch(fixed_direction)
      {
        case 0:
          return block[fixed_idx][i][j];
        case 1:
          return block[i][fixed_idx][j];
        case 2:
          return block[i][j][fixed_idx];
      }
      return 0;
    }

    /// True if the patch element is local to the current rank (2D)
    bool is_local_element(Uint i) const
    {
      cf3_assert(block.dimensions == 2);
      i = fixed_idx ? i : segments[0]-i-1;
      const Uint end_idx = fixed_idx != 0 ? fixed_idx - 1 : fixed_idx;
      return block.is_local_element(fixed_direction == 0 ? end_idx : i, fixed_direction == 1 ? end_idx : i);
    }

    /// True if the patch element is local to the current rank (3D)
    bool is_local_element(Uint i, Uint j) const
    {
      cf3_assert(block.dimensions == 3);
      if(fixed_direction != 2)
        i = fixed_idx ? i : segments[0]-i-1;
      else
        i = fixed_idx ? segments[0]-i-1 : i;

      const Uint end_idx = fixed_idx != 0 ? fixed_idx - 1 : fixed_idx;

      switch(fixed_direction)
      {
        case 0:
          return block.is_local_element(end_idx, i, j);
        case 1:
          return block.is_local_element(i, end_idx, j);
        case 2:
          return block.is_local_element(i, j, end_idx);
      }
      return false;
    }

    const Block& block;
    Uint nb_elems;
    std::vector<Uint> segments;
    Uint fixed_direction;
    Uint fixed_idx;
    Uint m_orientation;
  };

  void trigger_block_regions()
  {
    const Uint nb_blocks = blocks->size();
    if(block_regions.size() && block_regions.size() != nb_blocks)
    {
      const Uint nb_block_regions = block_regions.size();
      block_regions.clear();
      throw SetupError(FromHere(), "Wrong number of regions for blocks, expected: " + boost::lexical_cast<std::string>(nb_blocks) + ", obtained: " + boost::lexical_cast<std::string>(nb_block_regions));
    }
    if(block_regions.empty())
      block_regions.assign(nb_blocks, "interior");
  }

  /// Create a list of blocks, initialized based on the blockmesh structure
  void create_blocks()
  {
    trigger_block_regions();
    ghost_counter = 0;

    const Uint nb_blocks = blocks->size();
    const Uint dimensions = points->row_size();

    FaceConnectivity& face_conn = *face_connectivity;

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
    needed_nodes_ijk.clear();
    needed_nodes_ijk.resize(nb_blocks);

    patch_map.clear();
    const Table<Uint>& block_subdivs = *block_subdivisions;
    Uint block_nodes_start = 0;
    Uint block_elements_start = 0;
    Uint local_nodes_start = 0;
    for(Uint block_idx = 0; block_idx != nb_blocks; ++block_idx)
    {
      Block& block = block_list[block_idx];

      const Table<Uint>::ConstRow row = (*blocks)[block_idx];
      const Table<Uint>::ConstRow subdiv_row = block_subdivs[block_idx];

      Uint node_stride = 1;
      Uint element_stride = 1;
      Uint nb_points = 1;
      block.nb_elems = 1;
      for(Uint i = 0; i != dimensions; ++i)
      {
        // Add the block
        if(!face_conn.has_adjacent_element(block_idx, positive_faces[i]) || !face_conn.has_adjacent_element(block_idx, negative_faces[i]))
        {
          std::stringstream error_str;
          error_str << "Block " << block_idx << " has no adjacent element for patch [ ";
          const ElementType::FaceConnectivity& faces = dimensions == 3 ? LagrangeP1::Hexa3D::faces() : LagrangeP1::Quad2D::faces();
          const Uint bad_face_idx = !face_conn.has_adjacent_element(block_idx, positive_faces[i]) ? positive_faces[i] : negative_faces[i];
          BOOST_FOREACH(const Uint i, faces.nodes_range(bad_face_idx))
          {
            error_str << row[i] << " ";
          }
          error_str << "]. Did you flip the ordering of patch nodes?";
          throw common::SetupError(FromHere(), error_str.str());
        }
        FaceConnectivity::ElementReferenceT adj_elem = face_conn.adjacent_element(block_idx, positive_faces[i]);
        block.node_strides[i] = node_stride;
        block.element_strides[i] = element_stride;
        block.bounded[i] = face_conn.node_connectivity().entities()[adj_elem.first]->element_type().dimensionality() != dimensions;
        block.nb_points[i] = subdiv_row[i] + (block.bounded[i] ? 1 : 0);
        block.segments[i] = subdiv_row[i];
        block.nb_elems *= subdiv_row[i];
        node_stride *= block.nb_points[i];
        element_stride *= block.segments[i];
        nb_points *= block.nb_points[i];
        block.neighbors[i] = block.bounded[i] ? nullptr : &block_list[adj_elem.second];
      }

      // Add patches
      const std::string& my_region = block_regions[block_idx];
      for(Uint i = 0; i != dimensions; ++i)
      {
        FaceConnectivity::ElementReferenceT adj_elems[2];
        adj_elems[0] = face_conn.adjacent_element(block_idx, negative_faces[i]);
        adj_elems[1] = face_conn.adjacent_element(block_idx, positive_faces[i]);
        // check for a patch both in the positive and negative direction
        for(Uint dir = 0; dir != 2; ++dir)
        {
          const Uint patch_orientation = dir == 0 ? negative_faces[i] : positive_faces[i];
          if(face_conn.node_connectivity().entities()[adj_elems[dir].first]->element_type().dimensionality() == (dimensions-1))
          {
            patch_map[face_conn.node_connectivity().entities()[adj_elems[dir].first]->parent()->name()].push_back(new Patch(block, i, dir * (block.nb_points[i]-1), patch_orientation));
          }
          else
          {
            cf3_assert(face_conn.node_connectivity().entities()[adj_elems[dir].first]->element_type().dimensionality() == dimensions);
            cf3_assert(adj_elems[dir].second < block_regions.size());
            const std::string& other_region = block_regions[adj_elems[dir].second];
            if(other_region != my_region)
            {
              const std::string internal_patch_name = my_region + std::string("_interface_to_") + other_region;
              patch_map[internal_patch_name].push_back(new Patch(block, i, dir * (block.nb_points[i]), patch_orientation));
            }
          }
        }
      }

      // Distribution of nodes and elements among processed
      const Uint nb_procs = PE::Comm::instance().size();
      const Uint rank = PE::Comm::instance().rank();
      detail::distribute(block_elements_start, block.nb_elems, nb_procs, block.elements_distribution);
      detail::distribute(block_nodes_start, nb_points, nb_procs, block.nodes_distribution);

      block.local_nodes_start = local_nodes_start;
      local_nodes_start += block.nodes_distribution[rank+1] - block.nodes_distribution[rank];
      block.local_nodes_end = local_nodes_start;

      // Make sure all local nodes are added to the needed nodes list
      const Uint block_local_begin = block.nodes_distribution[rank];
      const Uint block_local_end = block.nodes_distribution[rank+1];
      for(Uint gid = block_local_begin; gid != block_local_end; ++gid)
      {
        std::array<Uint, 3> ijk = {0,0,0};
        Uint current_remainder = gid - block_nodes_start;
        for(Uint d = 0; d != dimensions; ++d)
        {
          const Uint idx = dimensions-d-1;
          ijk[idx] = (current_remainder / block.node_strides[idx]);
          current_remainder = current_remainder % block.node_strides[idx];
        }
        needed_nodes_ijk[block_idx][gid - block_local_begin + block.local_nodes_start] = ijk;
      }

      block_nodes_start += nb_points;
      block_elements_start += block.nb_elems;
    }
    nb_local_nodes = local_nodes_start;
  }

  /// Convert a global index to a local one, creating a ghost node if needed
  Uint to_local(const Block& block)
  {
    const Uint rank = common::PE::Comm::instance().rank();
    const Uint block_local_begin = block.nodes_distribution[rank];
    const Uint block_local_end = block.nodes_distribution[rank+1];
    const Uint gid = block.global_node_idx();
    if(gid >= block_local_begin && gid < block_local_end) // Local node, compute local index
    {
      return gid - block_local_begin + block.local_nodes_start;
    }

    // Non-local node, add a ghost
    const Uint lid = nb_local_nodes + ghost_counter;
    const Uint ghost_rank = std::upper_bound(block.nodes_distribution.begin(), block.nodes_distribution.end(), gid) - 1 - block.nodes_distribution.begin();
    std::pair<IndexMapT::iterator, bool> stored_gid = global_to_local.insert(std::make_pair(gid, std::make_pair(lid, ghost_rank)));

    // increment the number of ghosts if we didn't add a ghost for this gid before
    if(stored_gid.second)
    {
      ++ghost_counter;
    }

    return stored_gid.first->second.first;
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
        for(Uint j =0; j != segments[YY]; ++j)
        {
          for(Uint i =0; i != segments[XX]; ++i)
          {
            if(!block_list[block_idx].is_local_element(i,j,k))
              continue;

            common::Table<Uint>::Row element_connectivity = volume_connectivity[element_idx++];
            element_connectivity[0] = to_local(block_list[block_idx][i  ][j  ][k  ]);
            element_connectivity[1] = to_local(block_list[block_idx][i+1][j  ][k  ]);
            element_connectivity[2] = to_local(block_list[block_idx][i+1][j+1][k  ]);
            element_connectivity[3] = to_local(block_list[block_idx][i  ][j+1][k  ]);
            element_connectivity[4] = to_local(block_list[block_idx][i  ][j  ][k+1]);
            element_connectivity[5] = to_local(block_list[block_idx][i+1][j  ][k+1]);
            element_connectivity[6] = to_local(block_list[block_idx][i+1][j+1][k+1]);
            element_connectivity[7] = to_local(block_list[block_idx][i  ][j+1][k+1]);

            needed_nodes_ijk[block_idx][element_connectivity[0]] = {i,j,k};
            needed_nodes_ijk[block_idx][element_connectivity[1]] = {i+1,j,k};
            needed_nodes_ijk[block_idx][element_connectivity[2]] = {i+1,j+1,k};
            needed_nodes_ijk[block_idx][element_connectivity[3]] = {i,j+1,k};
            needed_nodes_ijk[block_idx][element_connectivity[4]] = {i,j,k+1};
            needed_nodes_ijk[block_idx][element_connectivity[5]] = {i+1,j,k+1};
            needed_nodes_ijk[block_idx][element_connectivity[6]] = {i+1,j+1,k+1};
            needed_nodes_ijk[block_idx][element_connectivity[7]] = {i,j+1,k+1};
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
          if(!block_list[block_idx].is_local_element(i,j))
          {
            continue;
          }

          common::Table<Uint>::Row element_connectivity = volume_connectivity[element_idx++];
          element_connectivity[0] = to_local(block_list[block_idx][i  ][j  ]);
          element_connectivity[1] = to_local(block_list[block_idx][i+1][j  ]);
          element_connectivity[2] = to_local(block_list[block_idx][i+1][j+1]);
          element_connectivity[3] = to_local(block_list[block_idx][i  ][j+1]);

          needed_nodes_ijk[block_idx][element_connectivity[0]] = {i,j,0};
          needed_nodes_ijk[block_idx][element_connectivity[1]] = {i+1,j,0};
          needed_nodes_ijk[block_idx][element_connectivity[2]] = {i+1,j+1,0};
          needed_nodes_ijk[block_idx][element_connectivity[3]] = {i,j+1,0};
        }
      }
    }
  }

  template<typename ET>
  struct ApplySF
  {
    using SF = typename ET::SF;
    using NodesT = typename ET::NodesT;
    using CoordsT = typename ET::CoordsT;
    using MappedCoordsT = typename ET::MappedCoordsT;

    explicit ApplySF(const NodesT& nodes) : m_nodes(nodes)
    {
    }

    template<typename MappedCoordsT>
    CoordsT operator()(const MappedCoordsT& mapped_coords) const
    {
      typename SF::ValueT sf;
      SF::compute_value(mapped_coords, sf);

      // Transform to real coordinates
      return sf * m_nodes;
    }

    const NodesT m_nodes;
  };

  template<typename ET>
  struct RadialSF
  {
  };

  template<typename ET>
  struct ApplySF<RadialSF<ET>>
  {
    using SF = typename ET::SF;
    using NodesT = typename ET::NodesT;
    using CoordsT = typename ET::CoordsT;
    using MappedCoordsT = typename ET::MappedCoordsT;

    explicit ApplySF(const NodesT& block_nodes, const CoordsT& center = CoordsT(0,0,0)) : m_center(center)
    {
      // Store nodes in radial format
      for(int i = 0; i != NodesT::RowsAtCompileTime; ++i)
      {
        m_nodes(i, 0) = atan2(block_nodes(i, 2), block_nodes(i, 0));
        m_nodes(i, 1) = block_nodes(i, 1);
        const Real dx = block_nodes(i,0)-m_center[0];
        const Real dz = block_nodes(i,2)-m_center[2];
        m_nodes(i, 2) = sqrt(dx*dx+dz*dz);
      }
    }

    template <typename MappedCoordsT>
    CoordsT operator()(const MappedCoordsT &mapped_coords) const
    {
      typename SF::ValueT sf;
      SF::compute_value(mapped_coords, sf);

      // Result in radial coordinates
      CoordsT result = sf * m_nodes;
      // Transform back to cartesian
      const Real theta = result[0];
      const Real r = result[2];
      result[0] = r*cos(theta) + m_center[0];
      result[2] = r*sin(theta) + m_center[2];
      return result;
    }

    const CoordsT m_center;
    NodesT m_nodes;
  };

  /// Create the block coordinates
  template<typename ApplyT>
  void fill_block_coordinates_3d(Table<Real>& mesh_coords, const Uint block_idx, const ApplyT& apply_sf)
  {
    const Uint rank = common::PE::Comm::instance().rank();
    const Table<Uint>::ConstRow& segments = (*block_subdivisions)[block_idx];
    const Table<Real>::ConstRow& gradings = (*block_gradings)[block_idx];

    common::Table<Real>::ArrayT ksi, eta, zta; // Mapped coordinates along each edge
    detail::create_mapped_coords(segments[XX], &gradings[0], ksi, 4);
    detail::create_mapped_coords(segments[YY], &gradings[4], eta, 4);
    detail::create_mapped_coords(segments[ZZ], &gradings[8], zta, 4);

    Real w[4][3]; // weights for each edge
    Real w_mag[3]; // Magnitudes of the weights

    for (const auto &ijk : needed_nodes_ijk[block_idx])
    {
      const Uint i = ijk.second[0];
      const Uint j = ijk.second[1];
      const Uint k = ijk.second[2];
      // Weights are calculating according to the BlockMesh algorithm from OpenFoam
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
      typename ApplyT::MappedCoordsT mapped_coords;
      mapped_coords[KSI] = (w[0][KSI]*ksi[i][0] + w[1][KSI]*ksi[i][1] + w[2][KSI]*ksi[i][2] + w[3][KSI]*ksi[i][3]) / w_mag[KSI];
      mapped_coords[ETA] = (w[0][ETA]*eta[j][0] + w[1][ETA]*eta[j][1] + w[2][ETA]*eta[j][2] + w[3][ETA]*eta[j][3]) / w_mag[ETA];
      mapped_coords[ZTA] = (w[0][ZTA]*zta[k][0] + w[1][ZTA]*zta[k][1] + w[2][ZTA]*zta[k][2] + w[3][ZTA]*zta[k][3]) / w_mag[ZTA];

      auto coords = apply_sf(mapped_coords);

      // Store the result
      const Uint node_idx = ijk.first;
      cf3_assert(node_idx < mesh_coords.size());
      mesh_coords[node_idx][XX] = coords[XX];
      mesh_coords[node_idx][YY] = coords[YY];
      mesh_coords[node_idx][ZZ] = coords[ZZ];
    }
  }

  /// Create the block coordinates
  template <typename ApplyT>
  void fill_block_coordinates_2d(Table<Real> &mesh_coords, const Uint block_idx, const ApplyT &apply_sf)
  {
    const Table<Uint>::ConstRow& segments = (*block_subdivisions)[block_idx];
    const Table<Real>::ConstRow& gradings = (*block_gradings)[block_idx];

    common::Table<Real>::ArrayT ksi, eta; // Mapped coordinates along each edge
    detail::create_mapped_coords(segments[XX], &gradings[0], ksi, 2);
    detail::create_mapped_coords(segments[YY], &gradings[2], eta, 2);

    Real w[2][2]; // weights for each edge
    Real w_mag[2]; // Magnitudes of the weights
    for(const auto& ijk : needed_nodes_ijk[block_idx])
    {
      const Uint i = ijk.second[0];
      const Uint j = ijk.second[1];

      // Weights are calculating according to the BlockMesh algorithm
      w[0][KSI] = (1. - ksi[i][0])*(1. - eta[j][0]) + (1. + ksi[i][0])*(1. - eta[j][1]);
      w[1][KSI] = (1. - ksi[i][1])*(1. + eta[j][0]) + (1. + ksi[i][1])*(1. + eta[j][1]);
      w_mag[KSI] = (w[0][KSI] + w[1][KSI]);

      w[0][ETA] = (1. - eta[j][0])*(1. - ksi[i][0]) + (1. + eta[j][0])*(1. - ksi[i][1]);
      w[1][ETA] = (1. - eta[j][1])*(1. + ksi[i][0]) + (1. + eta[j][1])*(1. + ksi[i][1]);
      w_mag[ETA] = (w[0][ETA] + w[1][ETA]);

      // Get the mapped coordinates of the node to add
      typename ApplyT::MappedCoordsT mapped_coords;
      mapped_coords[KSI] = (w[0][KSI]*ksi[i][0] + w[1][KSI]*ksi[i][1]) / w_mag[KSI];
      mapped_coords[ETA] = (w[0][ETA]*eta[j][0] + w[1][ETA]*eta[j][1]) / w_mag[ETA];

      // Transform to real coordinates
      auto coords = apply_sf(mapped_coords);

      // Store the result
      const Uint node_idx = ijk.first;
      cf3_assert(node_idx < mesh_coords.size());
      mesh_coords[node_idx][XX] = coords[XX];
      mesh_coords[node_idx][YY] = coords[YY];
    }
  }

  void add_patch(const std::string& name, Elements& patch_elems)
  {
    const Uint dimensions = points->row_size();
    Connectivity& patch_conn = patch_elems.geometry_space().connectivity();

    if(dimensions == 3)
    {
      const Uint idx_offsets[6][4][2] = {
        {{0,0},{0,1},{1,1},{1,0}},
        {{0,0},{0,1},{1,1},{1,0}},
        {{0,0},{0,1},{1,1},{1,0}},
        {{0,0},{1,0},{1,1},{0,1}},
        {{0,0},{0,1},{1,1},{1,0}},
        {{0,0},{1,0},{1,1},{0,1}}
      };

      // Count number of elements
      Uint nb_elems = 0;
      for(const Patch& patch : patch_map[name])
      {
        for(Uint i = 0; i != patch.segments[0]; ++i)
        {
          for(Uint j = 0; j != patch.segments[1]; ++j)
          {
            if(patch.is_local_element(i, j))
              ++nb_elems;
          }
        }
      }
      patch_elems.resize(nb_elems);

      // add elements
      Uint elem_idx = 0;
      for(const Patch& patch : patch_map[name])
      {
        for(Uint i = 0; i != patch.segments[0]; ++i)
        {
          for(Uint j = 0; j != patch.segments[1]; ++j)
          {
            if(patch.is_local_element(i, j))
            {
              Connectivity::Row elem_row = patch_conn[elem_idx++];
              elem_row[0] = to_local(patch.adjacent_block(i + idx_offsets[patch.m_orientation][0][0], j + idx_offsets[patch.m_orientation][0][1]));
              elem_row[1] = to_local(patch.adjacent_block(i + idx_offsets[patch.m_orientation][1][0], j + idx_offsets[patch.m_orientation][1][1]));
              elem_row[2] = to_local(patch.adjacent_block(i + idx_offsets[patch.m_orientation][2][0], j + idx_offsets[patch.m_orientation][2][1]));
              elem_row[3] = to_local(patch.adjacent_block(i + idx_offsets[patch.m_orientation][3][0], j + idx_offsets[patch.m_orientation][3][1]));
            }
          }
        }
      }
    }
    else
    {
      cf3_assert(dimensions == 2);
      // count elements
      Uint nb_elems = 0;
      for(const Patch& patch : patch_map[name])
      {
        for(Uint i = 0; i != patch.segments[0]; ++i)
        {
          if(patch.is_local_element(i))
            ++nb_elems;
        }
      }
      patch_elems.resize(nb_elems);

      // add elements
      Uint elem_idx = 0;
      for(const Patch& patch : patch_map[name])
      {
        const Uint first_offset = patch.fixed_direction == 0 ? 1 : 0;
        const Uint second_offset = patch.fixed_direction == 0 ? 0 : 1;
        for(Uint i = 0; i != patch.segments[0]; ++i)
        {
          if(patch.is_local_element(i))
          {
            Connectivity::Row elem_row = patch_conn[elem_idx++];
            elem_row[1] = to_local(patch.adjacent_block(i + first_offset));
            elem_row[0] = to_local(patch.adjacent_block(i + second_offset));
          }
        }
      }
    }
  }

  /// Helper data to construct the mesh connectivity
  std::vector<Block> block_list;
  typedef std::map<std::string, boost::ptr_vector<Patch> > PatchMapT;
  PatchMapT patch_map;
  /// Distribution of the local nodes among blocks
  Uint nb_local_nodes;
  Uint ghost_counter;
  typedef std::map<Uint, std::pair<Uint,Uint> > IndexMapT; // second pair is <lid, rank>
  IndexMapT global_to_local;
  std::vector<std::string> block_regions;
  /// Keeps the indices that are needed on this rank, for each block
  std::vector< std::map < Uint, std::array<Uint, 3> > > needed_nodes_ijk;
  std::vector<bool> block_is_arc;
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

  options().add("block_regions", std::vector<std::string>())
    .pretty_name("Block Regions")
    .description("For each block, the region it belongs to. Leave empty to assign each block to the region \"interior\"")
    .link_to(&m_implementation->block_regions)
    .attach_trigger(boost::bind(&Implementation::trigger_block_regions, m_implementation.get()))
    .mark_basic();

  options().add("block_is_arc", std::vector<bool>())
    .pretty_name("Block Arcs")
    .description("For each block, indicate if it is an arc with y-axis as normal and (0,0,0) as centroid.")
    .link_to(&m_implementation->block_is_arc)
    .mark_basic();

  options().add("periodic_x", std::vector<std::string>())
    .pretty_name("Periodic X")
    .description("Make the mesh periodic in the X direction?")
    .mark_basic();

  options().add("periodic_y", std::vector<std::string>())
    .pretty_name("Periodic Y")
    .description("Make the mesh periodic in the Y direction?")
    .mark_basic();

  options().add("periodic_z", std::vector<std::string>())
    .pretty_name("Periodic Z")
    .description("Make the mesh periodic in the Z direction?")
    .mark_basic();

  options().add("autopartition", true)
    .pretty_name("Autopartition")
    .description("Autopartition the mesh using the PHG partitioner")
    .mark_basic();
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
  m_implementation->block_is_arc.resize(nb_blocks, false);

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
//  if (is_not_null(m_implementation->block_mesh))
  {
//    remove_component("InnerBlockMesh");
  }
  m_implementation->block_mesh = create_component<Mesh>("InnerBlockMesh");

  const Uint nb_nodes   = m_implementation->points->size();
  const Uint dimensions = m_implementation->points->row_size();
  const Uint nb_blocks  = m_implementation->blocks->size();

  // root region and coordinates
  m_implementation->block_mesh->initialize_nodes(nb_nodes, dimensions);
  Dictionary& geometry_dict = m_implementation->block_mesh->geometry_fields();
  geometry_dict.coordinates().array() = m_implementation->points->array();

  for(Uint i = 0; i != nb_nodes; ++i)
  {
    geometry_dict.glb_idx()[i] = i;
  }

  Region &block_mesh_region = m_implementation->block_mesh->topology().create_region("block_mesh_region");

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

  m_implementation->block_mesh->update_structures();

  // Create connectivity data
  NodeConnectivity& node_connectivity = *m_implementation->block_mesh->create_component<NodeConnectivity>("node_connectivity");
  node_connectivity.initialize(m_implementation->block_mesh->elements());
  m_implementation->face_connectivity = block_elements.create_component<FaceConnectivity>("face_connectivity");
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


  const Uint blocks_begin = 0;
  const Uint blocks_end = nb_blocks;
  for(Uint i = blocks_begin; i != blocks_end; ++i)
  {
    block_elements.rank()[i] = 0;
    block_elements.glb_idx()[i] = i;
  }

  // m_implementation->block_mesh->update_structures();
  // m_implementation->block_mesh->raise_mesh_loaded();
  //m_implementation->block_mesh->check_sanity();
  return m_implementation->block_mesh;
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

  m_implementation->trigger_block_regions();

  typedef std::map< std::string, Uint > ElementsPerRegionT;
  ElementsPerRegionT elements_per_region; // Number of local elements per region
  const Uint nb_blocks = m_implementation->block_regions.size();
  for(Uint block_idx = 0; block_idx != nb_blocks; ++block_idx)
  {
    elements_per_region[m_implementation->block_regions[block_idx]] = 0;
  }
  for(Uint block_idx = 0; block_idx != nb_blocks; ++block_idx)
  {
    const Implementation::Block& block = m_implementation->block_list[block_idx];
    elements_per_region[m_implementation->block_regions[block_idx]] += block.elements_distribution[rank+1] - block.elements_distribution[rank];
  }

  Dictionary& geometry_dict = mesh.geometry_fields();

  std::map<std::string, Elements*> elements_map;
  for(ElementsPerRegionT::const_iterator it = elements_per_region.begin(); it != elements_per_region.end(); ++it)
  {
    Elements& volume_elements = mesh.topology().create_region(it->first).create_elements(dimensions == 3 ? "cf3.mesh.LagrangeP1.Hexa3D" : "cf3.mesh.LagrangeP1.Quad2D", geometry_dict);
    volume_elements.resize(it->second);
    elements_map[it->first] = &volume_elements;
  }

  // Set the connectivity, this also updates ghost node indices
  std::map<std::string, Uint> element_idx_map; // global element index per region
  const Uint blocks_begin = 0;
  const Uint blocks_end = m_implementation->block_list.size();
  for(Uint block_idx = blocks_begin; block_idx != blocks_end; ++block_idx)
  {
    m_implementation->add_block(block_subdivisions[block_idx], block_idx, elements_map[m_implementation->block_regions[block_idx]]->geometry_space().connectivity(), element_idx_map[m_implementation->block_regions[block_idx]]);
  }

  // Initialize coordinates
  mesh.initialize_nodes(m_implementation->nb_local_nodes + m_implementation->ghost_counter, dimensions);
  Field& coordinates = mesh.geometry_fields().coordinates();

  // Fill the coordinate array
  for(Uint block_idx = blocks_begin; block_idx != blocks_end; ++block_idx)
  {
    if(dimensions == 3)
    {
      Hexa3D::NodesT block_nodes;
      fill(block_nodes, *m_implementation->points, (*m_implementation->blocks)[block_idx]);
      if(m_implementation->block_is_arc[block_idx])
      {
        m_implementation->fill_block_coordinates_3d(coordinates, block_idx, Implementation::ApplySF<Implementation::RadialSF<Hexa3D>>(block_nodes));
      }
      else
      {
        m_implementation->fill_block_coordinates_3d(coordinates, block_idx, Implementation::ApplySF<Hexa3D>(block_nodes));
      }
    }
    if(dimensions == 2)
    {
      Quad2D::NodesT block_nodes;
      fill(block_nodes, *m_implementation->points, (*m_implementation->blocks)[block_idx]);
      m_implementation->fill_block_coordinates_2d(coordinates, block_idx, Implementation::ApplySF<Quad2D>(block_nodes));
    }
  }

  // Add surface patches
  for(Implementation::PatchMapT::const_iterator it = m_implementation->patch_map.begin(); it != m_implementation->patch_map.end(); ++it)
  {
    m_implementation->add_patch
    (
      it->first,
      mesh.topology().create_region(it->first).create_elements(dimensions == 3 ? "cf3.mesh.LagrangeP1.Quad3D" : "cf3.mesh.LagrangeP1.Line2D", geometry_dict)
    );
  }

  // surface patches shouldn't have introduced new ghosts
  cf3_assert(coordinates.size() == m_implementation->nb_local_nodes + m_implementation->ghost_counter);

  common::List<Uint>& gids = mesh.geometry_fields().glb_idx(); gids.resize(m_implementation->nb_local_nodes + m_implementation->ghost_counter);
  common::List<Uint>& ranks = mesh.geometry_fields().rank(); ranks.resize(m_implementation->nb_local_nodes + m_implementation->ghost_counter);

  // Local nodes
  Uint node_idx = 0;
  for(Uint block_idx = 0; block_idx != nb_blocks; ++block_idx)
  {
    const Implementation::Block& block = m_implementation->block_list[block_idx];
    const Uint nodes_begin = block.nodes_distribution[rank];
    const Uint nodes_end = block.nodes_distribution[rank+1];
    for(Uint gid = nodes_begin; gid != nodes_end; ++gid)
    {
      gids[node_idx] = gid;
      ranks[node_idx] = rank;
      ++node_idx;
    }
  }

  // Ghosts
  for(Implementation::IndexMapT::const_iterator ghost_it = m_implementation->global_to_local.begin(); ghost_it != m_implementation->global_to_local.end(); ++ghost_it)
  {
    const Uint global_id = ghost_it->first;
    const Uint local_id = ghost_it->second.first;
    const Uint ghost_rank = ghost_it->second.second;
    gids[local_id] = global_id;
    ranks[local_id] = ghost_rank;
  }
  if(PE::Comm::instance().is_active())
  {
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

    const mesh::Connectivity& conn = elements.geometry_space().connectivity();
    for (Uint elem=0; elem != nb_elems; ++elem)
    {
      elements.rank()[elem] = rank;
      elements.glb_idx()[elem] = elem + element_offset;
    }
    element_offset += nb_elems;
  }

  mesh.update_structures();

  mesh.raise_mesh_loaded();
  mesh.check_sanity();

  auto periodic_x = options().value<std::vector<std::string>>("periodic_x");
  auto periodic_y = options().value<std::vector<std::string>>("periodic_y");
  auto periodic_z = options().value<std::vector<std::string>>("periodic_z");

  const std::vector<std::vector<std::string>> periodic = {periodic_x, periodic_y, periodic_z};
  if(!periodic_x.empty() || !periodic_y.empty() || !periodic_z.empty())
  {
    common::build_component_abstract_type<mesh::MeshTransformer>("cf3.mesh.actions.MakeBoundaryGlobal", "globalbnd")->transform(mesh);
  }
  for(Uint i = 0; i != 3; ++i)
  {
    if(!periodic[i].empty())
    {
      auto average_pos = [&](const Table<Uint>& conn)
      {
        RealVector result(dimensions); result.setZero();
        Real count = 0.;
        for(auto&& row : conn.array())
        {
          for(const Uint idx : row)
          {
            result += Eigen::Map<RealVector const>(&points[idx][0], dimensions);
            count += 1.;
          }
        }
        result /= count;
        return result;
      };

      const RealVector translation = average_pos(*Handle<Table<Uint> const>(m_implementation->patches->get_child(periodic[i][0]))) - average_pos(*Handle<Table<Uint> const>(m_implementation->patches->get_child(periodic[i][1])));

      CFdebug << "computed translation vector: " << translation.transpose() << CFendl;
      auto periodic_linker = common::build_component_abstract_type<mesh::MeshTransformer>("cf3.mesh.actions.LinkPeriodicNodes", "link");
      periodic_linker->options().set("destination_region", Handle<Region>(mesh.topology().get_child(periodic[i][0])));
      periodic_linker->options().set("source_region", Handle<Region>(mesh.topology().get_child(periodic[i][1])));
      periodic_linker->options().set("translation_vector", std::vector<Real>(translation.data(), translation.data() + dimensions));
      periodic_linker->transform(mesh);
      mesh.add_component(periodic_linker);
    }
  }

  if(nb_procs == 1 || !options().value<bool>("autopartition"))
    return;

  try
  {
    auto partitioner = common::build_component_abstract_type<mesh::MeshTransformer>("cf3.zoltan.PHG", "load_balancer");
    partitioner->transform(mesh);

    SignalOptions options;
    options.add("mesh_uri", mesh.uri());
    options.add("mesh_rebalanced", true);
    SignalArgs args = options.create_frame();
    Core::instance().event_handler().raise_event( "mesh_changed", args);
    mesh.check_sanity();
  }
  catch(const std::exception& e)
  {
    throw common::SetupError(FromHere(), std::string("BlockMesher could not partition mesh: cf3.zoltan.PHG failed to load with error:\n") + e.what());
  }
}

void BlockArrays::signature_create_points(SignalArgs& args)
{
  SignalOptions options(args);
  options.add("dimensions", 3u).pretty_name("Dimensions").description("The physical dimensions for the mesh (must be 2 or 3)");
  options.add("nb_points", 0u).pretty_name("Number of points").description("The number of points needed to define the blocks");
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
  options.add("nb_blocks", 0u).pretty_name("Number of blocks").description("The number of blocks that are needed");
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
  options.add("name", "Default").pretty_name("Patch Name").description("The name for the created patch");
  options.add("nb_faces", 0u).pretty_name("Number of faces").description("The number of faces (of individual blocks) that make up the patch");
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
  options.add("name", "Default").pretty_name("Patch Name").description("The name for the created patch");
  options.add("face_list", std::vector<Uint>()).pretty_name("Face List").description("The list of faces that make up the patch. Numbers are as given in the default patch");
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

  options.add("positions", std::vector<Real>())
    .pretty_name("Positions")
    .description("Spanwise coordinate for each new spanwise layer of points. Values must ne greater than 0");

  options.add("nb_segments", std::vector<Uint>())
    .pretty_name("Nb Segments")
    .description("Number of spanwise segments for each block");

  options.add("gradings", std::vector<Real>())
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
  options.add("output_mesh", URI())
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
