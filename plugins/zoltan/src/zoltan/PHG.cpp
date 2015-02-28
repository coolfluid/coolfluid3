// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <set>

#include <boost/scoped_ptr.hpp>

#include <zoltan_cpp.h>

#include "common/Builder.hpp"
#include "common/FindComponents.hpp"
#include "common/List.hpp"

#include "math/Consts.hpp"

#include "mesh/ConnectivityData.hpp"
#include "mesh/Elements.hpp"
#include "mesh/Functions.hpp"
#include "mesh/Mesh.hpp"

#include "zoltan/PHG.hpp"

////////////////////////////////////////////////////////////////////////////////

using namespace cf3;

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace zoltan {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < PHG, mesh::MeshTransformer, LibZoltan> PHG_builder;

////////////////////////////////////////////////////////////////////////////////

// Type for a mapping of global ID to local ID
typedef std::map<Uint, Uint> GidMapT;

class ArrayElementRemover
{
public:
  ArrayElementRemover(const Uint size) :
    m_size(size)
  {
  }
  
  /// Insert a new index to remove and return true if that index was NOT inserted before
  bool insert(const Uint idx)
  {
    m_mapping.reset();
    cf3_assert(idx < m_size);
    return m_indices_to_remove.insert(idx).second; 
  }
  
  /// Number of indices that were registered for removal so far
  Uint size() const
  {
    return m_indices_to_remove.size();
  }

  Uint new_idx(const Uint old_idx)
  {
    return mapping()[old_idx];
  }
  
  /// Removes the array items located at the indices inserted so far
  template<typename ArrayT>
  void remove_rows(ArrayT& array)
  {
    cf3_assert(m_size == array.size());
    const Uint nb_to_remove = m_indices_to_remove.size();

    if(nb_to_remove == 0)
      return;

    const Uint first_idx = *(m_indices_to_remove.begin());
    if(m_size - first_idx == nb_to_remove) // The indices to remove are all at the tail of the array
    {
      array.resize(first_idx);
      return;
    }

    const IndexMapping& idx_mapping = mapping();

    const Uint nb_remaining = m_size - nb_to_remove;

    for(Uint i = nb_remaining; i != m_size; ++i)
    {
      if(m_indices_to_remove.count(i) != 0)
        continue;

      array[idx_mapping[i]] = array[i];
    }

    array.resize(nb_remaining);
  }
  
private:
  std::set<Uint> m_indices_to_remove;
  
  // Maps old indices to new ones
  struct IndexMapping
  {
    IndexMapping(const std::set<Uint>& indices_to_remove, const Uint array_size)
    {
      const Uint nb_to_remove = indices_to_remove.size();
        
      m_index_map.resize(nb_to_remove);
      
      std::vector<bool> needs_remove(nb_to_remove, false);
      const Uint nb_remaining = array_size - nb_to_remove;
      const std::set<Uint>::const_iterator tail_begin = indices_to_remove.lower_bound(nb_remaining);
      const std::set<Uint>::const_iterator tail_end = indices_to_remove.end();
      for(std::set<Uint>::const_iterator tail_it = tail_begin; tail_it != tail_end; ++tail_it)
      {
        needs_remove[*tail_it - nb_remaining] = true;
      }

      std::set<Uint>::const_iterator remove_it = indices_to_remove.begin();

      for(Uint i = nb_remaining; i != array_size; ++i)
      {
        if(!needs_remove[i - nb_remaining])
        {
          const Uint target_idx = *remove_it;
          cf3_assert(target_idx < nb_remaining);
          m_index_map[i - nb_remaining] = target_idx;
          ++remove_it;
        }
      }
      
      m_nb_remaining = nb_remaining;
    }
    
    Uint operator[](const Uint src_idx) const
    {
      if(src_idx >= m_nb_remaining)
      {
        cf3_assert(!m_index_map.empty());
        const Uint map_idx = src_idx - m_nb_remaining;
        cf3_assert(map_idx < m_index_map.size());
        return m_index_map[map_idx];
      }
      
      return src_idx;
    }
    
    std::vector<Uint> m_index_map;
    Uint m_nb_remaining;
  };
  
  const IndexMapping& mapping()
  {
    if(is_null(m_mapping.get()))
      m_mapping.reset(new IndexMapping(m_indices_to_remove, m_size));
    
    return *m_mapping;
  }
  
  boost::scoped_ptr<IndexMapping> m_mapping;
  const Uint m_size;
};

struct PeriodicData
{
  PeriodicData(mesh::Dictionary& dict)
  {
    periodic_links_nodes = Handle< common::List<Uint> >(dict.get_child("periodic_links_nodes")).get();
    periodic_links_active = Handle< common::List<bool> >(dict.get_child("periodic_links_active")).get();

    build_inverse_links();
  }

  /// Return true if the given node has a periodic link
  bool is_periodic(const Uint node_idx)
  {
    if(is_null(periodic_links_active))
      return false;

    return (*periodic_links_active)[node_idx];
  }

  /// Return the direct periodic link of the given node, or the node itself if there is none
  Uint target_node(const Uint node_idx)
  {
    if(is_periodic(node_idx))
      return (*periodic_links_nodes)[node_idx];

    return node_idx;
  }

  /// The final periodic link, obtained after following all periodic links that start from the given node.
  /// Returns the node itself if not periodic.
  Uint final_target_node(const Uint node_idx)
  {
    Uint result = target_node(node_idx);
    cf3_assert(!is_periodic(result));
    return result;
  }

  std::vector<Uint> inverse_links(const Uint node_idx)
  {
    if(is_null(periodic_links_active))
      return std::vector<Uint>();

    if(is_periodic(node_idx))
      cf3_assert(inverse_periodic_links[node_idx].empty());

    return inverse_periodic_links[node_idx];
  }

  void build_inverse_links()
  {
    if(is_null(periodic_links_active))
      return;

    const Uint nb_nodes = periodic_links_active->size();
    inverse_periodic_links.clear();
    inverse_periodic_links.resize(nb_nodes);
    for(Uint node_idx = 0; node_idx != nb_nodes; ++node_idx)
    {
      if(is_periodic(node_idx))
      {
        inverse_periodic_links[final_target_node(node_idx)].push_back(node_idx);
      }
    }
  }

  common::List<Uint>* periodic_links_nodes;
  common::List<bool>* periodic_links_active;

  std::vector< std::vector<Uint> > inverse_periodic_links;
};

/// Helper class to pack nodes in a char buffer, including all related fields. Packing is as follows:
/// - Node global ID
/// - Node rank
/// For all fields, ordered according to dict.fields():
/// - packed field row
class NodePacker
{
  typedef std::vector<char> BufferT;
public:
  
  typedef BufferT::iterator PosT;
  
  NodePacker(const mesh::Dictionary& dict, BufferT& buffer) :
    m_dict(dict),
    m_buffer(buffer),
    m_nb_fields(dict.fields().size()),
    m_position(buffer.begin())
  {
    // Compute the size of a single entry
    m_entry_size = gid_size + rank_size;
    m_field_sizes.reserve(m_nb_fields);
    for(Uint field_idx = 0; field_idx != m_nb_fields; ++field_idx)
    {
      m_field_sizes.push_back(sizeof(Real) * dict.fields()[field_idx]->row_size());
      m_entry_size += m_field_sizes.back();      
    }
  }
  
  /// Return the size needed for the given number of entries
  std::ptrdiff_t size(const Uint nb_entries)
  {
    return nb_entries*m_entry_size;
  }
  
  /// Reset the internal pointer to the given position
  void reset(const PosT position)
  {
    m_position = position;
  }

  /// Return the current position
  PosT position() const
  {
    return m_position;
  }
  
  void pack_local_id(const Uint local_id)
  {
    pack_number(m_dict.glb_idx()[local_id]);
    pack_number(m_dict.rank()[local_id]);
    
    for(Uint field_idx = 0; field_idx != m_nb_fields; ++field_idx)
    {
      const char* data_ptr = reinterpret_cast<const char*>(&((*m_dict.fields()[field_idx])[local_id][0]));
      std::copy(data_ptr, data_ptr+m_field_sizes[field_idx], m_position);
      m_position += m_field_sizes[field_idx];
    }
  }
  
  void pack_global_id(const Uint global_id)
  {
    const mesh::Dictionary::GlbToLocT::const_iterator gid_it = m_dict.glb_to_loc().find(global_id);
    cf3_assert(gid_it != m_dict.glb_to_loc().end());
    pack_local_id(gid_it->second);
  }
  
  /// Pack a single number
  template<typename NumberT>
  void pack_number(const NumberT& number)
  {
    const char* number_ptr = reinterpret_cast<const char*>(&number);
    std::copy(number_ptr, number_ptr+sizeof(NumberT), m_position);
    m_position += sizeof(NumberT);
  }
  
private:
  typedef Uint GidT;
  typedef Uint RankT;
  
  static const Uint gid_size = sizeof(GidT);
  static const Uint rank_size = sizeof(RankT);
  
  
  const mesh::Dictionary& m_dict;
  BufferT& m_buffer;
  const Uint m_nb_fields;
  
  PosT m_position;
  std::ptrdiff_t m_entry_size;
  std::vector<std::ptrdiff_t> m_field_sizes;
};

/// Helper class to unpack nodes packed using NodePacker
class NodeUnpacker
{
  typedef std::vector<char> BufferT;
  typedef BufferT::const_iterator PosT;
public:
  NodeUnpacker(const BufferT& buffer) :
    m_buffer(buffer),
    m_position(buffer.begin())
  {
  }
  
  /// Unpack the next node and store it in the dictionary, using the provided data structure to
  /// look up the local ID
  template<typename GidToLocalT>
  void unpack_next_node(mesh::Dictionary& dict, const GidToLocalT& gid_to_local)
  {
    cf3_assert(m_position != m_buffer.end());

    const Uint global_id = unpack_number<GidT>();
    const Uint local_id = gid_to_local[global_id];
    dict.rank()[local_id] = unpack_number<RankT>();
    dict.glb_idx()[local_id] = global_id;

    const Uint nb_fields = dict.fields().size();
    for(Uint field_idx = 0; field_idx != nb_fields; ++field_idx)
    {
      const Real* data_ptr = reinterpret_cast<const Real*>(&(*m_position));
      mesh::Field& field = *dict.fields()[field_idx];
      std::copy(data_ptr, data_ptr+field.row_size(), field[local_id].begin());

      const std::ptrdiff_t increment = field.row_size() * sizeof(Real);
      cf3_assert(std::distance(m_position, m_buffer.end()) >= increment);
      m_position += increment;
    }
  }

  /// Unpack a number and move the buffer position
  template<typename NumberT>
  NumberT unpack_number()
  {
    const NumberT result = *reinterpret_cast<NumberT const*>(&(*m_position));
    m_position += sizeof(NumberT);
    return result;
  }

  /// True if the position is at the end of the buffer
  bool eof() const
  {
    return m_position == m_buffer.end();
  }
  
private:
  typedef Uint GidT;
  typedef Uint RankT;
  
  const BufferT& m_buffer;
  
  PosT m_position;
};

/// Helper class to convert global to local indices
class GidToLocal
{
public:
  GidToLocal(const mesh::Dictionary::GlbToLocT& mesh_glb_to_loc, const Uint local_id_start) :
    m_mesh_glb_to_loc(&mesh_glb_to_loc),
    m_local_id(local_id_start)
  {
  }
  
  /// Create a new local ID for the given global ID, or return the previous one if it existed
  /// If a new ID was created, the second part of the return value is true
  std::pair<Uint, bool> create_local_id(const Uint global_id)
  {
    const mesh::Dictionary::GlbToLocT::const_iterator mesh_found = m_mesh_glb_to_loc->find(global_id);
    if(mesh_found != m_mesh_glb_to_loc->end())
    {
      return std::make_pair(mesh_found->second, false);
    }
    
    const GidMapT::iterator new_found = new_glb_to_loc.find(global_id);
    if(new_found != new_glb_to_loc.end())
    {
      return std::make_pair(new_found->second, false);
    }
    
    new_glb_to_loc[global_id] = m_local_id++;
    return std::make_pair(m_local_id - 1, true);
  }
  
  /// Check if the given global id is registered
  bool exists(const Uint global_id)
  {
    return (new_glb_to_loc.count(global_id) != 0) || m_mesh_glb_to_loc->exists(global_id);
  }
  
  /// Read only access, requiring that the element exists
  Uint operator[](const Uint global_id) const
  {
    const mesh::Dictionary::GlbToLocT::const_iterator mesh_found = m_mesh_glb_to_loc->find(global_id);
    if(mesh_found != m_mesh_glb_to_loc->end())
    {
      return mesh_found->second;
    }
    
    const GidMapT::const_iterator new_found = new_glb_to_loc.find(global_id);
    cf3_assert(new_found != new_glb_to_loc.end());
    return new_found->second;
  }
  
private:
  // The mapping obtained from e.g. the mesh
  const mesh::Dictionary::GlbToLocT* m_mesh_glb_to_loc;
  
  // The mapping for any newly found
  GidMapT new_glb_to_loc;
  
  // The current local id to assign
  Uint m_local_id;
};

/// Remove all ghost elements from a mesh
void remove_ghost_elements(const mesh::Mesh& mesh)
{
  const Uint nb_entities = mesh.elements().size();
  for(Uint entity_idx = 0; entity_idx != nb_entities; ++entity_idx)
  {
    mesh::Entities& entities = *mesh.elements()[entity_idx];
    const Uint nb_elems = entities.size();
    ArrayElementRemover elem_remover(nb_elems);
    for(Uint elem_idx = 0; elem_idx != nb_elems; ++elem_idx)
    {
      if(entities.is_ghost(elem_idx))
        elem_remover.insert(elem_idx);
    }

    elem_remover.remove_rows(entities.glb_idx());
    elem_remover.remove_rows(entities.rank());
    const Uint nb_spaces = entities.spaces().size();
    for(Uint space_idx = 0; space_idx != nb_spaces; ++space_idx)
    {
      elem_remover.remove_rows(entities.spaces()[space_idx]->connectivity());
    }
  }
}

/// Remove nodes not used by any element in the mesh
void remove_unused_nodes(const mesh::Mesh& mesh)
{
  const Uint nb_dicts = mesh.dictionaries().size();
  for(Uint dict_idx = 0; dict_idx != nb_dicts; ++dict_idx)
  {
    mesh::Dictionary& dict = *mesh.dictionaries()[dict_idx];
    const Uint nb_nodes = dict.size();

    boost::shared_ptr< common::List< Uint > > used_nodes = mesh::build_used_nodes_list(mesh, dict, true, true);
    std::vector<bool> is_used(nb_nodes, false);
    BOOST_FOREACH(const Uint used_node_idx, used_nodes->array())
    {
      is_used[used_node_idx] = true;
    }

    ArrayElementRemover elem_remover(nb_nodes);

    for(Uint node_idx = 0; node_idx != nb_nodes; ++node_idx)
    {
      if(!is_used[node_idx])
      {
        elem_remover.insert(node_idx);
      }
    }

    elem_remover.remove_rows(dict.glb_idx());
    elem_remover.remove_rows(dict.rank());

    const Uint nb_fields = dict.fields().size();
    for(Uint field_idx = 0; field_idx != nb_fields; ++field_idx)
    {
      elem_remover.remove_rows(*dict.fields()[field_idx]);
    }

    const Uint nb_entities = mesh.elements().size();
    for(Uint entities_idx = 0; entities_idx != nb_entities; ++entities_idx)
    {
      mesh::Entities& entities = *mesh.elements()[entities_idx];
      mesh::Connectivity& conn = entities.space(dict).connectivity();
      const Uint row_size = conn.row_size();
      const Uint nb_elements = entities.size();
      for(Uint elem_idx = 0; elem_idx != nb_elements; ++elem_idx)
      {
        mesh::Connectivity::Row row = conn[elem_idx];
        for(Uint i = 0; i != row_size; ++i)
        {
          row[i] = elem_remover.new_idx(row[i]);
        }
      }
    }

    // Remove nodes from the periodic structures as well
    common::List<Uint>* periodic_links_nodes = Handle< common::List<Uint> >(dict.get_child("periodic_links_nodes")).get();
    common::List<bool>* periodic_links_active = Handle< common::List<bool> >(dict.get_child("periodic_links_active")).get();
    if(is_not_null(periodic_links_nodes))
    {
      elem_remover.remove_rows(*periodic_links_active);
      elem_remover.remove_rows(*periodic_links_nodes);

      const Uint new_nb_nodes = periodic_links_nodes->size();
      cf3_assert(new_nb_nodes == dict.size());
      for(Uint i = 0; i != new_nb_nodes; ++i)
      {
        if((*periodic_links_active)[i])
        {
          (*periodic_links_nodes)[i] = elem_remover.new_idx((*periodic_links_nodes)[i]);
        }
      }
    }
  }
}

/// Completes the elements, ensuring each rank has access to all the elements that use each of its local nodes
void complete_elements(const mesh::Mesh& mesh, const GidMapT& periodic_links_gid_map)
{
  common::PE::Comm& comm = common::PE::Comm::instance();
  
  boost::shared_ptr<mesh::NodeConnectivity> node_connectivity = common::allocate_component<mesh::NodeConnectivity>("NodeConnectivity");
  node_connectivity->initialize(mesh.elements());
  
  const Uint nb_geom_nodes = mesh.geometry_fields().size();
  common::List<Uint>& geom_node_ranks = mesh.geometry_fields().rank();

  std::vector<Uint> send_sizes(comm.size(), 0); // Number of Uint values to send to each CPU
  std::vector< std::set<mesh::NodeConnectivity::ElementReferenceT> > element_sets(comm.size()); // Sets of elements to send
  
  // Packed elements to send. Packing is as follows:
  // - total packed size
  // - entities index into mesh.entities();
  // - element GID
  // then for each space:
  // - Space index (in the entities.spaces() vector)
  // - GIDs of the nodes
  std::vector< std::vector<Uint> > send_elements(comm.size());
  
  // Find the elements to pack
  for(Uint node_idx = 0; node_idx != nb_geom_nodes; ++node_idx)
  {
    const Uint destination_rank = geom_node_ranks[node_idx];
    
    if(destination_rank == comm.rank())
      continue;
    
    BOOST_FOREACH(const mesh::NodeConnectivity::ElementReferenceT& element_ref, node_connectivity->node_element_range(node_idx))
    {
      if(element_sets[destination_rank].insert(element_ref).second)
      {
        const mesh::Entities& entities = *mesh.elements()[element_ref.first];
        send_sizes[destination_rank] += 3;
        const Uint nb_spaces = entities.spaces().size();
        for(Uint space_idx = 0; space_idx != nb_spaces; ++space_idx)
        {
          send_sizes[destination_rank] += 1 + entities.spaces()[space_idx]->shape_function().nb_nodes();
        }
      }
    }
  }
  
  // Pack the elements
  mesh::Dictionary& geom_dict = mesh.geometry_fields();
  common::List<Uint>& geom_node_gids = geom_dict.glb_idx();
  for(Uint rank = 0; rank != comm.size(); ++rank)
  {
    std::vector<Uint>& send_elements_for_rank = send_elements[rank];
    send_elements_for_rank.reserve(send_sizes[rank]);
    BOOST_FOREACH(const mesh::NodeConnectivity::ElementReferenceT& element_ref, element_sets[rank])
    {
      const Uint entities_idx = element_ref.first;
      const Uint element_local_id = element_ref.second;
      const mesh::Entities& entities = *mesh.elements()[entities_idx];

      // Build the "header"
      send_elements_for_rank.push_back(3); // 3 numbers for packed size, entities idx and element GID
      Uint& packed_size = send_elements_for_rank.back();
      send_elements_for_rank.push_back(entities_idx);
      send_elements_for_rank.push_back(entities.glb_idx()[element_local_id]);

      // Add the spaces
      const Uint nb_spaces = entities.spaces().size();
      for(Uint space_idx = 0; space_idx != nb_spaces; ++space_idx)
      {
        send_elements_for_rank.push_back(space_idx);
        ++packed_size;
        const mesh::Space& space = *entities.spaces()[space_idx];
        BOOST_FOREACH(const Uint node_local_id, space.connectivity()[element_local_id])
        {
          send_elements_for_rank.push_back(space.dict().glb_idx()[node_local_id]);
          ++packed_size;
        }
      }
    }
  }

  // Buffers for elements received from other ranks
  std::vector< std::vector<Uint> > recv_elements(comm.size());

  // Do the communication
  comm.all_to_all(send_elements, recv_elements);

  // Build an index of the owned elements so far
  const Uint nb_entities = mesh.elements().size();
  std::vector< std::set<Uint> > element_gid_sets(nb_entities); // Sets to check if an element GID is already added
  for(Uint entities_idx = 0; entities_idx != nb_entities; ++entities_idx)
  {
    BOOST_FOREACH(const Uint element_gid, mesh.elements()[entities_idx]->glb_idx().array())
    {
      element_gid_sets[entities_idx].insert(element_gid);
    }
  }

  // Indicates if the received elements starting at a given index need to be unpacked
  std::vector< std::vector<bool> > needs_unpack(comm.size());

  // Number of elements to unpack for each Entities in the mesh
  std::vector<Uint> nb_new_elements(nb_entities, 0);

  // Count the number of elements to unpack
  for(Uint rank = 0; rank != comm.size(); ++rank)
  {
    const std::vector<Uint>& recv_elements_for_rank = recv_elements[rank];
    needs_unpack[rank].resize(recv_elements_for_rank.size(), true); // Unpack everything by default
    Uint recv_idx = 0;
    const Uint recv_size = recv_elements_for_rank.size();
    while(recv_idx != recv_size)
    {
      const Uint entry_size = recv_elements_for_rank[recv_idx];
      const Uint entities_idx = recv_elements_for_rank[recv_idx+1];
      const Uint new_gid = recv_elements_for_rank[recv_idx+2];
      
      cf3_assert(entities_idx < element_gid_sets.size());

      // If we already owned the element, it doesn't need to be unpacked
      if(!element_gid_sets[entities_idx].insert(new_gid).second)
      {
        needs_unpack[rank][recv_idx] = false;
      }
      else
      {
        ++nb_new_elements[entities_idx];
      }

      recv_idx += entry_size;
      cf3_assert(recv_idx <= recv_size);
    }
  }

  // Starting indices for the new elements
  std::vector<Uint> new_elements_starts(nb_entities);

  // Allocate new storage
  for(Uint entities_idx = 0; entities_idx != nb_entities; ++entities_idx)
  {
    mesh::Entities& entities = *mesh.elements()[entities_idx];
    new_elements_starts[entities_idx] = entities.size();
    entities.resize(entities.size() + nb_new_elements[entities_idx]);
  }

  // Current position in the entities tables
  std::vector<Uint> current_entities_idx = new_elements_starts;

  // Create local index generators
  const Uint nb_dicts = mesh.dictionaries().size();
  std::vector<GidToLocal> glb_to_loc;
  for(Uint dict_idx = 0; dict_idx != nb_dicts; ++dict_idx)
  {
    mesh::Dictionary& dict = *mesh.dictionaries()[dict_idx];
    if(&dict != &geom_dict)
    {
      dict.rebuild_map_glb_to_loc();
    }
    glb_to_loc.push_back(GidToLocal(dict.glb_to_loc(), dict.size()));
  }

  std::vector<GidMapT> new_node_gids(nb_dicts); // For each dictionary, the new nodes to request, required to complete the elements that we receive
  std::vector<Uint> nb_nodes_to_request(comm.size(), 0); // Per rank, the number of nodes to request

  GidMapT new_periodic_links;

  // Unpack the elements
  for(Uint rank = 0; rank != comm.size(); ++rank)
  {
    const std::vector<Uint>& recv_elements_for_rank = recv_elements[rank];
    Uint recv_idx = 0;
    const Uint recv_size = recv_elements_for_rank.size();
    while(recv_idx != recv_size)
    {
      const Uint entry_size = recv_elements_for_rank[recv_idx];
      if(!needs_unpack[rank][recv_idx])
      {
        recv_idx += entry_size;
        continue;
      }
      ++recv_idx;

      const Uint entities_idx = recv_elements_for_rank[recv_idx++];
      const Uint element_gid = recv_elements_for_rank[recv_idx++];
      const Uint element_idx = current_entities_idx[entities_idx]++;
      
      mesh::Entities& entities = *mesh.elements()[entities_idx];
      entities.glb_idx()[element_idx] = element_gid;
      
      const Uint nb_spaces = entities.spaces().size();
      for(Uint space_idx = 0; space_idx != nb_spaces; ++space_idx)
      {
        cf3_assert(recv_elements_for_rank[recv_idx++] == space_idx);
        mesh::Space& space = *entities.spaces()[space_idx];
        mesh::Connectivity::Row conn_row = space.connectivity()[element_idx];
        
        const Uint element_start = recv_idx;
        const Uint element_end = recv_idx + space.shape_function().nb_nodes();
        const Uint dict_idx = space.dict_idx();
        const mesh::Dictionary* dict = mesh.dictionaries()[dict_idx].get();
        
        for(; recv_idx != element_end; ++recv_idx)
        {
          const Uint node_gid = recv_elements_for_rank[recv_idx];
          const std::pair<Uint, bool> local_id = glb_to_loc[dict_idx].create_local_id(node_gid);

          cf3_assert(node_gid != math::Consts::uint_max());
          
          if(local_id.second) // We have a new node to request
          {
            new_node_gids[dict_idx][node_gid] = rank;
            nb_nodes_to_request[rank]++;
          }
          conn_row[recv_idx - element_start] = local_id.first;

          // Check for a periodic link...
          if(dict == &geom_dict)
          {
            GidMapT::const_iterator periodic_it = periodic_links_gid_map.find(node_gid);
            if(periodic_it != periodic_links_gid_map.end())
            {
              const Uint periodic_gid = periodic_it->second;
              cf3_assert(periodic_gid != math::Consts::uint_max());
              const std::pair<Uint, bool> periodic_local_id = glb_to_loc[dict_idx].create_local_id(periodic_gid);
              if(local_id.second) // If it was a new node, we need to add a new periodic link
              {
                new_periodic_links[local_id.first] = periodic_local_id.first;
              }
              if(periodic_local_id.second) // If the periodic node is new, we need to fetch it
              {
                cf3_assert(periodic_links_gid_map.count(periodic_gid) == 0);
                new_node_gids[dict_idx][periodic_gid] = rank;
                nb_nodes_to_request[rank]++;
              }
            }
          }
        }
      }
      
      cf3_assert(recv_idx <= recv_size);
    }
  }

  // Request new nodes
  // Packing: dict idx, nb nodes for dict, node gids...
  std::vector< std::vector<Uint> > send_required_gids(comm.size());
  for(Uint rank = 0; rank != comm.size(); ++rank)
  {
    send_required_gids[rank].reserve(nb_nodes_to_request[rank] + nb_dicts*2);
  }

  // Pack the nodes to request
  for(Uint dict_idx = 0; dict_idx != nb_dicts; ++dict_idx)
  {
    std::vector<Uint> count_indices(comm.size());
    for(Uint rank = 0; rank != comm.size(); ++rank)
    {
      send_required_gids[rank].push_back(dict_idx);
      count_indices[rank] = send_required_gids[rank].size();
      send_required_gids[rank].push_back(0);
    }
    for(GidMapT::const_iterator new_node_it = new_node_gids[dict_idx].begin(); new_node_it != new_node_gids[dict_idx].end(); ++new_node_it)
    {
      const Uint rank = new_node_it->second;
      send_required_gids[rank].push_back(new_node_it->first);
      ++send_required_gids[rank][count_indices[rank]];
    }
  }

  // Let the sending nodes know what GIDs need to be transmitted
  std::vector< std::vector<Uint> > recv_required_gids(comm.size());
  comm.all_to_all(send_required_gids, recv_required_gids);

  // Packed buffers for sending the nodes, as follows:
  // - dict index
  // - nb nodes for dict
  // - NodePacker packed data
  std::vector< std::vector<char> > send_nodes(comm.size());
  // Buffer sizes (to be computed)
  std::vector<Uint> buffer_sizes(comm.size());

  // Set the correct size for the buffers
  for(Uint rank = 0; rank != comm.size(); ++rank)
  {
    const std::vector<Uint>& recv_gids_for_rank = recv_required_gids[rank];
    const Uint recv_size = recv_gids_for_rank.size();
    for(Uint recv_idx = 0; recv_idx != recv_size;)
    {
      const Uint dict_idx = recv_gids_for_rank[recv_idx++];
      const Uint dict_nb_entries = recv_gids_for_rank[recv_idx++];
      buffer_sizes[rank] += NodePacker(*mesh.dictionaries()[dict_idx], send_nodes[rank]).size(dict_nb_entries)
                            + 2*sizeof(Uint);
      recv_idx += dict_nb_entries;
      cf3_assert(recv_idx <= recv_gids_for_rank.size());
    }
  }
  
  // Pack the nodes
  for(Uint rank = 0; rank != comm.size(); ++rank)
  {
    const std::vector<Uint>& recv_gids_for_rank = recv_required_gids[rank];
    std::vector<char>& send_nodes_for_rank = send_nodes[rank];
    
    // Allocate the storage
    send_nodes_for_rank.resize(buffer_sizes[rank]);
    
    const Uint recv_size = recv_gids_for_rank.size();
    NodePacker::PosT last_pos = send_nodes_for_rank.begin();
    for(Uint recv_idx = 0; recv_idx != recv_size;)
    {
      const Uint dict_idx = recv_gids_for_rank[recv_idx++];
      const Uint dict_nb_entries = recv_gids_for_rank[recv_idx++];
      
      NodePacker packer(*mesh.dictionaries()[dict_idx], send_nodes_for_rank);
      packer.reset(last_pos);
      packer.pack_number(dict_idx);
      packer.pack_number(dict_nb_entries);
      for(Uint i = 0; i != dict_nb_entries; ++i)
      {
        cf3_assert(recv_idx < recv_gids_for_rank.size());
        packer.pack_global_id(recv_gids_for_rank[recv_idx++]);
      }
      
      last_pos = packer.position();
    }
  }
  
  // Communicate the new nodes
  std::vector< std::vector<char> > recv_nodes(comm.size());
  comm.all_to_all(send_nodes, recv_nodes);
  
  // Reserve storage for the new nodes
  for(Uint dict_idx = 0; dict_idx != nb_dicts; ++dict_idx)
  {
    mesh::Dictionary& dict = *mesh.dictionaries()[dict_idx];
    const Uint old_size = dict.size();
    const Uint new_size = old_size + new_node_gids[dict_idx].size();
    dict.resize(new_size);
    if(&dict == &geom_dict) // Assuming periodicity is defined on the geometry dict
    {
      common::List<Uint>* periodic_links_nodes = Handle< common::List<Uint> >(dict.get_child("periodic_links_nodes")).get();
      common::List<bool>* periodic_links_active = Handle< common::List<bool> >(dict.get_child("periodic_links_active")).get();
      if(is_not_null(periodic_links_active))
      {
        periodic_links_nodes->resize(new_size);
        periodic_links_active->resize(new_size);
        std::fill(periodic_links_active->array().begin()+old_size, periodic_links_active->array().end(), false);
        std::fill(periodic_links_nodes->array().begin()+old_size, periodic_links_nodes->array().end(), math::Consts::uint_max());

        // Update the new links
        for(GidMapT::const_iterator periodic_it = new_periodic_links.begin(); periodic_it != new_periodic_links.end(); ++periodic_it)
        {
          (*periodic_links_active)[periodic_it->first] = true;
          (*periodic_links_nodes)[periodic_it->first] = periodic_it->second;
        }
      }
    }
  }

  // Unpack nodes
  std::vector<Uint> nb_unpacked(nb_dicts, 0);
  for(Uint rank = 0; rank != comm.size(); ++rank)
  {
    NodeUnpacker unpacker(recv_nodes[rank]);
    while(!unpacker.eof())
    {
      const Uint dict_idx = unpacker.unpack_number<Uint>();
      const Uint dict_nb_recv_nodes = unpacker.unpack_number<Uint>();
      mesh::Dictionary& dict = *mesh.dictionaries()[dict_idx];
      for(Uint new_node_idx = 0; new_node_idx != dict_nb_recv_nodes; ++new_node_idx)
      {
        unpacker.unpack_next_node(dict, glb_to_loc[dict_idx]);
      }
      nb_unpacked[dict_idx] += dict_nb_recv_nodes;
    }
  }
  for(Uint dict_idx = 0; dict_idx != nb_dicts; ++dict_idx)
  {
    cf3_assert(nb_unpacked[dict_idx] == new_node_gids[dict_idx].size());
  }

  // Update element ranks and other dictionary ranks
  BOOST_FOREACH(const Handle<mesh::Entities>& entities, mesh.elements())
  {
    const mesh::Connectivity& geom_connectivity = entities->geometry_space().connectivity();
    common::List<Uint>& elem_ranks = entities->rank();
    const Uint nb_elements = entities->size();

    const RealMatrix& support_local_coordinates = entities->geometry_space().shape_function().local_coordinates();
    
    for(Uint elem_idx = 0; elem_idx != nb_elements; ++elem_idx)
    {
      elem_ranks[elem_idx] = geom_node_ranks[geom_connectivity[elem_idx][0]];
      cf3_assert(elem_ranks[elem_idx] != math::Consts::uint_max());
    }
    BOOST_FOREACH(const Handle<mesh::Space>& space, entities->spaces())
    {
      if(space.get() == &(entities->geometry_space()))
        continue;
      
      const mesh::Connectivity& dict_connectivity = space->connectivity();
      const RealMatrix& space_local_coordinates = space->shape_function().local_coordinates();
      std::vector<Uint> node_mapping;
      std::vector<bool> is_interior;
      mesh::nearest_node_mapping(support_local_coordinates, space_local_coordinates, node_mapping, is_interior);
      const Uint nb_conn_nodes = dict_connectivity.row_size();
      cf3_assert(node_mapping.size() == dict_connectivity.row_size());
      common::List<Uint>& dict_ranks = space->dict().rank();
      for(Uint elem_idx = 0; elem_idx != nb_elements; ++elem_idx)
      {
        const mesh::Connectivity::ConstRow dict_row = dict_connectivity[elem_idx];
        const mesh::Connectivity::ConstRow support_row = geom_connectivity[elem_idx];
        for(Uint node_idx = 0; node_idx != nb_conn_nodes; ++node_idx)
        {
          if(is_interior[node_idx])
          {
            dict_ranks[dict_row[node_idx]] = elem_ranks[elem_idx];
          }
          else
          {
            dict_ranks[dict_row[node_idx]] = geom_node_ranks[support_row[node_mapping[node_idx]]];
          }
        }
      }
    }
  }
}

/// Remove elements that only refer to ghost nodes
void remove_full_ghost_elements(mesh::Mesh& mesh)
{
  const Uint my_rank = common::PE::Comm::instance().rank();
  const common::List<Uint>& node_ranks = mesh.geometry_fields().rank();

  const Uint nb_entities = mesh.elements().size();
  for(Uint entities_idx = 0; entities_idx != nb_entities; ++entities_idx)
  {
    mesh::Entities& entities = *mesh.elements()[entities_idx];
    const mesh::Connectivity& conn = entities.geometry_space().connectivity();
    const Uint nb_elements = entities.size();
    ArrayElementRemover elem_remover(nb_elements);
    for(Uint elem_idx = 0; elem_idx != nb_elements; ++elem_idx)
    {
      const mesh::Connectivity::ConstRow row = conn[elem_idx];
      bool all_ghosts = true;
      BOOST_FOREACH(const Uint node_idx, row)
      {
        if(node_ranks[node_idx] == my_rank)
        {
          all_ghosts = false;
          break;
        }
      }
      if(all_ghosts)
      {
        elem_remover.insert(elem_idx);
      }
    }

    elem_remover.remove_rows(entities.glb_idx());
    elem_remover.remove_rows(entities.rank());
    const Uint nb_spaces = entities.spaces().size();
    for(Uint space_idx = 0; space_idx != nb_spaces; ++space_idx)
    {
      elem_remover.remove_rows(entities.spaces()[space_idx]->connectivity());
    }
  }
}

struct HGCollection
{
  HGCollection(mesh::Mesh& mesh) :
    dimension(mesh.dimension()),
    coordinates(mesh.geometry_fields().coordinates()),
    node_gids(mesh.geometry_fields().glb_idx()),
    node_ranks(mesh.geometry_fields().rank()),
    periodic_data(mesh.geometry_fields()),
    geometry_dict(mesh.geometry_fields())
  {
    common::PE::Comm& comm = common::PE::Comm::instance();
    const Uint my_rank = comm.rank();

    const Uint nb_all_nodes = mesh.geometry_fields().size();

    // Global and local IDs for the graph vertices
    vertex_gids.reserve(nb_all_nodes);
    vertex_lids.reserve(nb_all_nodes);
    for(Uint node_idx = 0; node_idx != nb_all_nodes; ++node_idx)
    {
      if(node_ranks[node_idx] != my_rank)
      {
        my_ghost_nodes[node_gids[node_idx]] = node_idx;
        continue;
      }

      if(periodic_data.is_periodic(node_idx))
        continue;

      vertex_gids.push_back(node_gids[node_idx]);
      vertex_lids.push_back(node_idx);
    }
    
    // Build the hypergraph structure
    Uint total_nb_elems = 0;
    Uint total_nb_pins = 0.;
    BOOST_FOREACH(const mesh::Elements& elems, common::find_components_recursively_with_filter<mesh::Elements>(mesh, mesh::IsElementsVolume()))
    {
      total_nb_elems += elems.size();
      total_nb_pins += elems.size() * elems.geometry_space().connectivity().row_size();
    }
    
    hyperedge_gids.reserve(total_nb_elems);
    hyperedge_ptr.reserve(total_nb_elems);
    pin_gids.reserve(total_nb_pins);
    
    BOOST_FOREACH(const mesh::Elements& elems, common::find_components_recursively_with_filter<mesh::Elements>(mesh, mesh::IsElementsVolume()))
    {
      hyperedge_gids.insert(hyperedge_gids.end(), elems.glb_idx().array().begin(), elems.glb_idx().array().end());
      const Uint nb_elems = elems.size();
      const mesh::Connectivity& conn = elems.geometry_space().connectivity();
      const Uint row_size = conn.row_size();
      for(Uint elem_idx = 0; elem_idx != nb_elems; ++elem_idx)
      {
        hyperedge_ptr.push_back(pin_gids.size());
        for(Uint elem_node_idx = 0; elem_node_idx != row_size; ++elem_node_idx)
        {
          const mesh::Connectivity::ConstRow row = conn[elem_idx];
          pin_gids.push_back(node_gids[periodic_data.final_target_node(row[elem_node_idx])]);
        }
      }
    }

    nodes_to_remove.resize(node_gids.size(), false);
  }

  // Updates the ranks of the old ghost nodes, i.e. nodes that don't stay on the current rank and are not removed
  void update_ranks()
  {
    common::PE::Comm& comm = common::PE::Comm::instance();
    const Uint nb_nodes = node_ranks.size();

    std::vector< std::vector<Uint> > send_gids(comm.size()); // Nodes for which we request a rank update
    std::vector< std::vector<Uint> > recv_gids(comm.size()); // Nodes that we have to send rank updates for (received after communication)

    // Request an update for all ghost nodes that have now become a local node
    for(GidMapT::const_iterator gid_it = my_ghost_nodes.begin(); gid_it != my_ghost_nodes.end(); ++gid_it)
    {
      send_gids[node_ranks[gid_it->second]].push_back(gid_it->first);
    }

    // Do the communication for the ghost GIDs
    comm.all_to_all(send_gids, recv_gids);

    std::vector< std::vector<Uint> > send_ranks(comm.size()); // Ranks to send to other ranks, packed as (gid1, rank1, gid2,rank2, ...)
    std::vector< std::vector<Uint> > recv_ranks(comm.size()); // Received ranks after communication

    const mesh::Dictionary::GlbToLocT& global_to_local = geometry_dict.glb_to_loc();

    for(Uint rank = 0; rank != comm.size(); ++rank)
    {
      send_ranks[rank].reserve(2*recv_gids[rank].size());
      BOOST_FOREACH(const Uint global_id, recv_gids[rank])
      {
        const mesh::Dictionary::GlbToLocT::const_iterator gid_it = global_to_local.find(global_id);
        cf3_assert(gid_it != global_to_local.end());
        const Uint local_id = gid_it->second;
        cf3_assert(node_ranks[local_id] == comm.rank() || (local_id < nodes_to_remove.size() && nodes_to_remove[local_id]));
        send_ranks[rank].push_back(global_id);
        send_ranks[rank].push_back(node_ranks[local_id]);
      }
    }

    // Communicate the new ranks
    comm.all_to_all(send_ranks, recv_ranks);

    BOOST_FOREACH(const std::vector<Uint>& new_ranks, recv_ranks)
    {
      cf3_assert_desc("new_ranks.size(): " + common::to_str(new_ranks.size()), new_ranks.size() % 2 == 0);
      const Uint nb_entries = new_ranks.size() / 2;
      for(Uint i = 0; i != nb_entries; ++i)
      {
        const Uint global_id = new_ranks[2*i];
        const Uint new_rank = new_ranks[2*i + 1];
        const mesh::Dictionary::GlbToLocT::const_iterator gid_it = global_to_local.find(global_id);
        cf3_assert(global_to_local.exists(global_id));
        node_ranks[global_to_local[global_id]] = new_rank;
      }
    }
  }

  static int get_number_of_vertices(void* data, int* ierr)
  {
    HGCollection* coll = static_cast<HGCollection*>(data);
    *ierr = ZOLTAN_OK;
    return coll->vertex_gids.size();
  }

  static void get_vertex_list(void* data, int size_gid, int size_lid,
              ZOLTAN_ID_PTR global_id, ZOLTAN_ID_PTR local_id,
                    int wgt_dim, float* obj_wgts, int* ierr)
  {
    HGCollection* coll = static_cast<HGCollection*>(data);
    *ierr = ZOLTAN_OK;

    const Uint nb_vertices = coll->vertex_gids.size();
    for(Uint i = 0; i != nb_vertices; ++i)
    {
      global_id[i] = coll->vertex_gids[i];
      local_id[i] = coll->vertex_lids[i];
    }
  }

  static void get_hypergraph_size(void* data, int* num_edges, int* num_pins,
                                  int* format, int* ierr)
  {
    HGCollection* coll = static_cast<HGCollection*>(data);
    *num_edges = coll->hyperedge_gids.size();
    *num_pins = coll->pin_gids.size();
    *format = ZOLTAN_COMPRESSED_EDGE;
    *ierr = ZOLTAN_OK;
  }

  static void get_hypergraph(void* data, int size_gid, int num_edges, int num_pins,
                             int format, ZOLTAN_ID_PTR edgeGID, int* edgePtr,
                             ZOLTAN_ID_PTR pinGID, int* ierr)
  {
    HGCollection* coll = static_cast<HGCollection*>(data);

    if ( (num_edges != coll->hyperedge_gids.size()) || (num_pins != coll->pin_gids.size()) ||
         (format != ZOLTAN_COMPRESSED_EDGE))
    {
      *ierr = ZOLTAN_FATAL;
      return;
    }

    std::copy(coll->hyperedge_gids.begin(), coll->hyperedge_gids.end(), edgeGID);
    std::copy(coll->hyperedge_ptr.begin(), coll->hyperedge_ptr.end(), edgePtr);
    std::copy(coll->pin_gids.begin(), coll->pin_gids.end(), pinGID);
  }
  
  static int node_migration_size(void* data, int size_gid, int size_lid, ZOLTAN_ID_PTR global_id, ZOLTAN_ID_PTR local_id, int* ierr)
  {
    *ierr = ZOLTAN_OK;

    HGCollection* coll = static_cast<HGCollection*>(data);

    // At least send the coordinates and the number of periodic links
    int result = coll->dimension * sizeof(Real) + sizeof(Uint)
        + coll->periodic_data.inverse_links(*local_id).size() * (sizeof(Uint) + coll->dimension * sizeof(Real)); // Also send each node (GID + coordinates) that has a periodic link to this one

    return result;
  }

  static void node_migration_pack(void* data, int size_gid, int size_lid, ZOLTAN_ID_PTR global_id, ZOLTAN_ID_PTR local_id, int dest, int size, char* buffer, int* ierr)
  {
    *ierr = ZOLTAN_OK;

    HGCollection* coll = static_cast<HGCollection*>(data);

    char* out_ptr = buffer;

    const std::ptrdiff_t coord_size = coll->dimension*sizeof(Real);

    // Coordinates of the node
    const char* in_start = reinterpret_cast<const char*>(&coll->coordinates[*local_id][0]);
    out_ptr = std::copy(in_start, in_start + coord_size, out_ptr);

    // Mark node for removal
    coll->nodes_to_remove[*local_id] = true;
    coll->node_ranks[*local_id] = dest;

    // The periodic links
    const std::vector<Uint> inverse_links = coll->periodic_data.inverse_links(*local_id);
    const Uint nb_links = inverse_links.size();
    const char* nb_links_ptr = reinterpret_cast<const char*>(&nb_links);
    out_ptr = std::copy(nb_links_ptr, nb_links_ptr + sizeof(Uint), out_ptr);
    BOOST_FOREACH(const Uint inv_link, inverse_links)
    {
      // Copy the GID
      const char* gid_start = reinterpret_cast<const char*>(&coll->node_gids[inv_link]);
      out_ptr = std::copy(gid_start, gid_start + sizeof(Uint), out_ptr);

      // Copy the coordinates
      const char* coord_start = reinterpret_cast<const char*>(&coll->coordinates[inv_link][0]);
      out_ptr = std::copy(coord_start, coord_start + coord_size, out_ptr);

      coll->nodes_to_remove[inv_link] = true;
      coll->node_ranks[inv_link] = dest;
    }
    const std::ptrdiff_t filled_size = out_ptr - buffer;
    cf3_assert_desc("incorrect fill on node " + common::to_str(*local_id) + ": " + common::to_str(filled_size) + " <= " + common::to_str(size), filled_size <= size);
  }

  static void node_migration_unpack(void* data, int size_gid, int num_ids, ZOLTAN_ID_PTR global_ids, int* sizes, int* idx, char* buffer, int* ierr)
  {
    *ierr = ZOLTAN_OK;

    HGCollection* coll = static_cast<HGCollection*>(data);

    common::PE::Comm& comm = common::PE::Comm::instance();

    const std::ptrdiff_t coord_size = coll->dimension*sizeof(Real);

    // Compute the number of coordinates to receive, including periodic nodes
    Uint nb_to_receive = 0;
    for(Uint i = 0; i != num_ids; ++i)
    {
      if(coll->my_ghost_nodes.count(global_ids[i])) // Node was a ghost already, so it doesn't need to be added
      {
        continue;
      }

      Uint nb_links;
      char* buf_start = &buffer[idx[i]] + coord_size;
      std::copy(buf_start, buf_start + sizeof(Uint), reinterpret_cast<char*>(&nb_links));

      nb_to_receive += 1 + nb_links;
    }

    const Uint new_nodes_start = coll->coordinates.size();
    const Uint new_nb_nodes = new_nodes_start + nb_to_receive;
    coll->geometry_dict.resize(new_nb_nodes);
    if(is_not_null(coll->periodic_data.periodic_links_active))
    {
      coll->periodic_data.periodic_links_active->resize(new_nb_nodes);
      coll->periodic_data.periodic_links_nodes->resize(new_nb_nodes);
      std::fill(coll->periodic_data.periodic_links_active->array().begin()+new_nodes_start, coll->periodic_data.periodic_links_active->array().end(), false);
      std::fill(coll->periodic_data.periodic_links_nodes->array().begin()+new_nodes_start, coll->periodic_data.periodic_links_nodes->array().end(), math::Consts::uint_max());
    }
    // Initial new nodes to an invalid value, to catch omissions
    if(nb_to_receive != 0)
    {
      for(Uint node_idx = new_nodes_start; node_idx != new_nb_nodes; ++node_idx)
        coll->node_ranks[node_idx] = math::Consts::uint_max();
    }

    // Actually unpack the nodes
    Uint node_idx = new_nodes_start;
    for(Uint i = 0; i != num_ids; ++i)
    {
      const Uint node_gid = global_ids[i];
      const GidMapT::iterator ghost_it = coll->my_ghost_nodes.find(node_gid);
      if(ghost_it != coll->my_ghost_nodes.end()) // Node was already a ghost, so no unpacking is needed
      {
        const Uint local_id = ghost_it->second;
        coll->my_ghost_nodes.erase(ghost_it);
        coll->node_ranks[local_id] = comm.rank();
        BOOST_FOREACH(const Uint inverse_link, coll->periodic_data.inverse_links(local_id))
        {
          const Uint inverse_gid = coll->node_gids[inverse_link];
          cf3_assert_desc("periodic inverse link " + common::to_str(inverse_link) + " (GID " + common::to_str(inverse_gid) + ") was not found in the ghosts list", coll->my_ghost_nodes.count(inverse_gid));
          coll->my_ghost_nodes.erase(inverse_gid);
          coll->node_ranks[inverse_link] = comm.rank();
          cf3_assert(coll->periodic_data.is_periodic(inverse_link));
          cf3_assert(coll->periodic_data.final_target_node(inverse_link) == local_id);
        }

        continue;
      }

      const int buf_size = sizes[i];
      char* buf_start = &buffer[idx[i]];

      // Unpack the node itself
      cf3_assert_desc(common::to_str(node_idx) + " < " + common::to_str(coll->coordinates.size()), node_idx < coll->coordinates.size());
      std::copy(buf_start, buf_start + coord_size, reinterpret_cast<char*>(&coll->coordinates[node_idx][0]));
      coll->node_gids[node_idx] = global_ids[i];
      coll->node_ranks[node_idx] = comm.rank();

      const Uint node_local_id = node_idx; // Store this for the periodic links

      ++node_idx;

      // Unpack the periodic links
      char* buf_ptr = buf_start + coord_size;
      const char* buf_end = buf_start + static_cast<std::ptrdiff_t>(buf_size); // This is actually the maximum buffer end
      Uint nb_links = 0;
      std::copy(buf_ptr, buf_ptr + sizeof(Uint), reinterpret_cast<char*>(&nb_links));
      buf_ptr += sizeof(Uint);
      for(Uint link_idx = 0; link_idx != nb_links; ++link_idx)
      {
        cf3_assert_desc(common::to_str(node_idx) + " < " + common::to_str(coll->coordinates.size()), node_idx < coll->coordinates.size());
        std::copy(buf_ptr, buf_ptr + sizeof(Uint), reinterpret_cast<char*>(&coll->node_gids[node_idx]));
        buf_ptr += sizeof(Uint);
        std::copy(buf_ptr, buf_ptr + coord_size, reinterpret_cast<char*>(&coll->coordinates[node_idx][0]));
        buf_ptr += coord_size;
        coll->node_ranks[node_idx] = comm.rank();

        (*coll->periodic_data.periodic_links_active)[node_idx] = true;
        (*coll->periodic_data.periodic_links_nodes)[node_idx] = node_local_id;

        ++node_idx;
        const std::ptrdiff_t nb_left = buf_end - buf_ptr;
        cf3_assert(nb_left >= 0);
      }
    }

    coll->periodic_data.build_inverse_links();
  }

  /// Check if the structures are still sound
  void check()
  {
    const Uint nb_nodes = geometry_dict.size();
    for(Uint i = 0; i != nb_nodes; ++i)
    {
      cf3_assert_desc("Invalid node GID at index " + common::to_str(i), node_gids[i] != math::Consts::uint_max());
    }

    if(is_not_null(periodic_data.periodic_links_active))
    {
      cf3_assert(geometry_dict.size() == periodic_data.periodic_links_active->size());
      for(Uint i = 0; i != nb_nodes; ++i)
      {
        if((*periodic_data.periodic_links_active)[i])
        {
          const Uint target_node = (*periodic_data.periodic_links_nodes)[i];
          cf3_assert_desc("rank mismatch on link from " + common::to_str(i) + " [" + common::to_str(node_ranks[i]) + "] to " + common::to_str(target_node) + " [" + common::to_str(node_ranks[target_node]) + "]", node_ranks[i] == node_ranks[(*periodic_data.periodic_links_nodes)[i]]);
        }
      }
    }
  }

  // Graph vertices are the mesh nodes (without periodic nodes)
  std::vector<ZOLTAN_ID_TYPE> vertex_gids;
  std::vector<ZOLTAN_ID_TYPE> vertex_lids;

  // Graph hyperedges are the mesh elements
  std::vector<Uint> hyperedge_gids;

  // For each hyperedge, the first index in the pins array
  std::vector<Uint> hyperedge_ptr;

  // Pins, i.e. the mesh node GID linked to each element
  std::vector<Uint> pin_gids;

  // Dimension of the mesh
  const Uint dimension;

  // Node coordinates
  mesh::Field& coordinates;

  // Mesh node GIDs
  common::List<Uint>& node_gids;

  // Mesh node ranks
  common::List<Uint>& node_ranks;
  
  // Dict for the geometry
  mesh::Dictionary& geometry_dict;

  // Helper data to handle mesh periodicity
  PeriodicData periodic_data;

  // For each node, mark if it needs to be removed
  std::vector<bool> nodes_to_remove;

  GidMapT my_ghost_nodes;
};

PHG::PHG(const std::string& name) : mesh::MeshTransformer(name)
{
  float version;
  int error_code = Zoltan_Initialize(common::Core::instance().argc(), common::Core::instance().argv(),&version);
  if(error_code != ZOLTAN_OK)
    throw common::SetupError(FromHere(), "Failed to initialize Zoltan");
}

PHG::~PHG()
{
}

void PHG::execute()
{
  common::PE::Comm& comm = common::PE::Comm::instance();
  if(!comm.is_initialized())
    throw common::SetupError(FromHere(), "MPI must be initiailized before using the Zoltan plugin");
  
  Zoltan zz(comm.communicator());

  zz.Set_Param("IMBALANCE_TOL", "1.005");
  zz.Set_Param("DEBUG_LEVEL", "1");
  zz.Set_Param("REMAP", "1");
  zz.Set_Param("LB_METHOD", "HYPERGRAPH");   // partitioning method
  zz.Set_Param("HYPERGRAPH_PACKAGE", "PHG"); // version of method
  zz.Set_Param("NUM_GID_ENTRIES", "1");// global IDs are integers
  zz.Set_Param("NUM_LID_ENTRIES", "1");// local IDs are integers
  zz.Set_Param("RETURN_LISTS", "ALL"); // export AND import lists
  zz.Set_Param("OBJ_WEIGHT_DIM", "0"); // use Zoltan default vertex weights
  zz.Set_Param("EDGE_WEIGHT_DIM", "0");// use Zoltan default hyperedge weights
  zz.Set_Param("PHG_CUT_OBJECTIVE", "CONNECTIVITY");
  zz.Set_Param("LB_APPROACH", "PARTITION");
  zz.Set_Param("PHG_MULTILEVEL", "1");
  zz.Set_Param("CHECK_HYPERGRAPH", "1");
  zz.Set_Param("PHG_OUTPUT_LEVEL", "2");
  zz.Set_Param("PHG_COARSENING_METHOD", "AGG");
  zz.Set_Param("PHG_EDGE_SIZE_THRESHOLD", "0.5");
  zz.Set_Param("PHG_COARSEPARTITION_METHOD", "GREEDY"); 

	// After linking periodic nodes, each rank has a full view of the mesh periodicity. We store this here based on GIDs
	GidMapT periodic_link_gid_map;
	const common::List<Uint>* periodic_links_nodes = Handle< common::List<Uint> const >(mesh().geometry_fields().get_child("periodic_links_nodes")).get();
	const common::List<bool>* periodic_links_active = Handle< common::List<bool> const >(mesh().geometry_fields().get_child("periodic_links_active")).get();
	if(is_not_null(periodic_links_nodes))
	{
		const Uint nb_nodes = periodic_links_nodes->size();
		for(Uint i = 0; i != nb_nodes; ++i)
		{
			if((*periodic_links_active)[i])
				periodic_link_gid_map[mesh().geometry_fields().glb_idx()[i]] = mesh().geometry_fields().glb_idx()[(*periodic_links_nodes)[i]];
		}
	}

  remove_ghost_elements(mesh());
  remove_unused_nodes(mesh());

  HGCollection collection(mesh());
  collection.check();

  zz.Set_Num_Obj_Fn(HGCollection::get_number_of_vertices, &collection);
  zz.Set_Obj_List_Fn(HGCollection::get_vertex_list, &collection);
  zz.Set_HG_Size_CS_Fn(HGCollection::get_hypergraph_size, &collection);
  zz.Set_HG_CS_Fn(HGCollection::get_hypergraph, &collection);

  int changes, numGidEntries, numLidEntries, numImport, numExport;
  ZOLTAN_ID_PTR importGlobalGids, importLocalGids, exportGlobalGids, exportLocalGids;
  int *importProcs, *importToPart, *exportProcs, *exportToPart;

  zz.LB_Partition(changes,        // 1 if partitioning was changed, 0 otherwise
    numGidEntries,  // Number of integers used for a global ID
    numLidEntries,  // Number of integers used for a local ID
    numImport,      // Number of vertices to be sent to me
    importGlobalGids,  // Global IDs of vertices to be sent to me
    importLocalGids,   // Local IDs of vertices to be sent to me
    importProcs,    // Process rank for source of each incoming vertex
    importToPart,   // New partition for each incoming vertex
    numExport,      // Number of vertices I must send to other processes
    exportGlobalGids,  // Global IDs of the vertices I must send
    exportLocalGids,   // Local IDs of the vertices I must send
    exportProcs,    // Process to which I send each of the vertices
    exportToPart);  // Partition to which each vertex will belong

  zz.Set_Obj_Size_Fn(HGCollection::node_migration_size, &collection);
  zz.Set_Pack_Obj_Fn(HGCollection::node_migration_pack, &collection);
  zz.Set_Unpack_Obj_Multi_Fn(HGCollection::node_migration_unpack, &collection);

  zz.Migrate(numImport, importGlobalGids, importLocalGids, importProcs, importToPart, numExport, exportGlobalGids, exportLocalGids, exportProcs, exportToPart);

  collection.check();

  mesh().geometry_fields().rebuild_map_glb_to_loc();
  
  collection.update_ranks();
  collection.check();

	complete_elements(mesh(), periodic_link_gid_map);

  collection.check();

  remove_full_ghost_elements(mesh());

  collection.check();

  remove_unused_nodes(mesh());

  collection.check();

  mesh().update_structures();
  mesh().update_statistics();
  mesh().check_sanity();
}

////////////////////////////////////////////////////////////////////////////////

} // namespace zoltan
} // namespace cf3

////////////////////////////////////////////////////////////////////////////////
