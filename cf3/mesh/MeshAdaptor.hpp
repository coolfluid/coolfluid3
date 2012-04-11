// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_Manipulations_hpp
#define cf3_mesh_Manipulations_hpp

////////////////////////////////////////////////////////////////////////////////

#include <set>
#include <boost/cstdint.hpp>

#include "common/PE/Buffer.hpp"

#include "common/List.hpp"
#include "common/Table.hpp"
#include "common/DynTable.hpp"

namespace cf3 {
namespace mesh {

  class Dictionary;
  class Entities;
  class Mesh;
  class MeshAdaptor;
  class PackedNode;
  class PackedElement;

////////////////////////////////////////////////////////////////////////////////

/// @brief Class to adapt the mesh
///
/// First call prepare(). This puts mesh in inconsistent state, but allows it to be
/// changed more easily.
/// Then indicate all the changes with functions like
///  - add_element()
///  - remove_element()
///  - add_node()
///  - remove_node()
/// The changes are NOT applied until finally the function finish() is called.
/// This also puts the mesh back in a consistent state, and updates mesh statistics
/// @author Willem Deconinck
class MeshAdaptor
{
public:

  /// Constructor
  MeshAdaptor(mesh::Mesh& mesh);

  /// Destructor
  virtual ~MeshAdaptor();

  /// @brief Prepare the mesh adaptor for changes
  ///
  /// Prepare the mesh adaptor for changes to add/remove nodes and elements
  /// The element-node connectivity tables are replaced with global node indices instead
  /// of local indices, so that elements from other processes can be added.
  /// @note All changes that are given (remove_node(), add_element(), ...)
  ///       are not applied until finish() or flush_nodes() and/or flush_elements() is called.
  /// @post Mesh is in inconsistent state after this! Call finish() to fix it, after modifications
  void prepare();

  /// @brief Apply the changes the mesh adaptor for changes and fix inconsistent state
  ///
  /// @post Mesh is back in consistent state!
  void finish();

  /// @name Elementary operations, should not be called usually
  //@{

  /// @brief Add element to the mesh
  /// @note Changes are only applied after flush_elements() or finish() is called
  /// @pre make_element_node_connectivity_global() must be called
  /// @pre create_element_buffers() must have been called before
  void add_element(const PackedElement& packed_element);

  /// @brief Remove element from the mesh
  /// @note Changes are only applied after flush_elements() or finish() is called
  /// @pre create_element_buffers() must have been called before
  void remove_element(const PackedElement& packed_element);

  /// @brief Remove element from the mesh
  /// @note Changes are only applied after flush_elements() or finish() is called
  /// @pre create_element_buffers() must have been called before
  void remove_element(const Uint entities_idx, const Uint elem_loc_idx);

  /// @brief Apply all changes to elements
  void flush_elements();

  /// @brief Add node from the mesh
  /// @note Changes are only applied after flush_nodes() or finish() is called
  /// @pre make_element_node_connectivity_global() must be called
  /// @pre create_node_buffers() must have been called before
  void add_node(const PackedNode& packed_node);

  /// @brief Remove node from the mesh
  /// @note Changes are only applied after flush_nodes() or finish() is called
  /// @pre create_node_buffers() must have been called before
  void remove_node(const PackedNode& packed_node);

  /// @brief Remove node from the mesh
  /// @note Changes are only applied after flush_nodes() or finish() is called
  /// @pre create_node_buffers() must have been called before
  void remove_node(const Uint dict_idx, const Uint node_loc_idx);

  /// @brief Apply all changes to nodes
  void flush_nodes();

  //@}

  /// @name Fine Level functions, should not be called usually
  //@{

  /// @brief Creates buffers for element changes
  void create_element_buffers();

  /// @brief Creates buffers for node changes
  void create_node_buffers();

  /// @brief Element-node connectivity is replaced with global indices
  ///
  /// This is to allow elements from other ranks to be added, in which case local indices are meaningless
  /// @post Mesh is in inconsistent state! Call restore_element_node_connectivity() to fix it,
  ///       after element modifications are done.
  void make_element_node_connectivity_global();

  /// @brief Element-node connectivity is restored with local indices
  ///
  /// @post Mesh is in inconsistent state! Call restore_element_node_connectivity() to fix it,
  ///       after element modifications are done.
  void restore_element_node_connectivity();

  /// @brief rebuild dictionary.glb_to_loc() map with flushed nodes included
  void rebuild_node_glb_to_loc_map();

  /// @brief rebuild dictionary.connectivity() map with flushed nodes and elements included
  void rebuild_node_to_element_connectivity();

  //@} End Fine Level functions


  /// @name High-level API
  //@{

  /// @brief Move elements and attached nodes between processors, according to an elements_changeset
  ///
  /// @param [in] exported_elements_loc_id  A set with 3 indices: move_element[to_pid][from_entities_idx][local_elem_idx]
  /// @post nodes and elements are flushed, and node-ranks are uniquely defined in all pid's.
  ///       Call finish() to notify the mesh of updates.
  void move_elements(const std::vector< std::vector< std::vector<Uint> > >& exported_elements_loc_id);

  /// @brief Create an additional cell-layer of overlap between pid's
  ///
  /// @post nodes and elements are flushed, and node-ranks are uniquely defined in all pid's.
  ///       Call finish() to notify the mesh of updates.
  void grow_overlap();

  //@}

  /// @name Low-level API
  //@{

  /// @brief Assemble a change-set of nodes to be sent together with elements
  /// @param [in]  exported_elements_loc_id  A set with 3 indices: send_element[to_pid][from_entities_idx][local_elem_idx]
  /// @param [out] exported_nodes_loc_id     A set with 3 indices: send_node[to_pid][from_dict_idx][local_node_idx]
  void find_nodes_to_export(const std::vector< std::vector< std::vector<Uint> > >& exported_elements_loc_id,
                            std::vector< std::vector< std::vector<Uint> > >&       exported_nodes_loc_id);

  /// @brief Send/Receive elements according to an elements-changeset
  /// @param [in]  exported_elements_loc_id  A set with 3 indices: send_element[to_pid][from_entities_idx][local_elem_idx]
  /// @param [out] imported_elements_glb_id  A set with 3 indices: received_element[from_pid][from_entities_idx][glb_elem_idx]
  /// @post Elements are not flushed yet, so additional operations can be performed
  void send_elements(const std::vector< std::vector< std::vector<Uint> > >&       exported_elements_loc_id,
                     std::vector< std::vector< std::vector<boost::uint64_t> > >&  imported_elements_glb_id);

  /// @brief Send/Receive nodes according to an nodes-changeset
  /// @param [in]  exported_nodes_loc_id  A set with 3 indices: send_node[to_pid][from_dict_idx][local_node_idx]
  /// @param [out] imported_nodes_glb_id  A set with 3 indices: received_node[from_pid][from_dict_idx][glb_node_idx]
  /// @post Nodes are not flushed yet, so additional operations can be performed
  void send_nodes(const std::vector< std::vector< std::vector<Uint> > >&      exported_nodes_loc_id,
                  std::vector< std::vector< std::vector<boost::uint64_t> > >& imported_nodes_glb_id);

  /// @brief Correct ranks of nodes to be unique in all pid's
  void fix_node_ranks();

  /// @brief remove ghost nodes
  /// @post Nodes are not flushed yet, so additional operations can be performed
  void remove_ghost_nodes();

  /// @brief remove ghost elements
  /// @post Elements are not flushed yet, so additional operations can be performed
  void remove_ghost_elements();

  // @}


private:

  /// @brief Handle to the mesh
  Handle<Mesh> m_mesh;

  /// @brief boolean that says if mesh has element-node connectivities with global node-indices
  bool is_node_connectivity_global;

  /// @brief Element buffers for global index
  std::vector< boost::shared_ptr<common::List<Uint>::Buffer> > element_glb_idx;

  /// @brief Element buffers for rank
  std::vector< boost::shared_ptr<common::List<Uint>::Buffer> > element_rank;

  /// @brief Element buffers for element-node connectivity
  std::vector< std::vector< boost::shared_ptr<common::Table<Uint>::Buffer> > > element_connected_nodes;

  /// @brief Node buffers for global index
  std::vector< boost::shared_ptr<common::List<Uint>::Buffer> > node_glb_idx;

  /// @brief Node buffers for rank
  std::vector< boost::shared_ptr<common::List<Uint>::Buffer> > node_rank;

  /// @brief Node buffers for field values
  std::vector< std::vector< boost::shared_ptr<common::Table<Real>::Buffer> > > node_field_values;

  /// @brief flag if dictionary.glb_to_loc() must be rebuilt
  bool node_glb_to_loc_needs_rebuild;

  /// @brief flag if dictionary.connectivity() must be rebuilt
  bool node_elem_connectivity_needs_rebuild;

  /// @brief flag if there are still elements that need to be flush
  bool elem_flush_required;

  /// @brief flag if there are still nodes that need to be flush
  bool node_flush_required;

  /// @brief bookkeeping of added and removed elements
  std::vector< std::set<boost::uint64_t> > added_elements;

  /// @brief bookkeeping of added and removed nodes
  std::vector< std::set<boost::uint64_t> > added_nodes;

#if 0
  void fix_node_ranks(const std::vector< std::vector<boost::uint64_t> >& nodes);
#endif

};

////////////////////////////////////////////////////////////////////////////////

/// @brief Class that contains all stand-alone global information of an element.
///
/// This information can be retrieved from a common::PE::buffer that has been
/// communicated from other ranks, or from local information.
class PackedElement : public common::PE::PackedObject
{
public:

  /// @brief Constructor
  PackedElement(const mesh::Mesh& mesh);

  /// @brief Constructor, packing from local information
  PackedElement(const mesh::Mesh& mesh, const Uint entities_idx , const Uint elem_idx);

  // Unpack from buffer
  virtual void unpack(common::PE::Buffer& buf);

  // Pack in a buffer
  virtual void pack(common::PE::Buffer& buf);

public:

  Uint entities_idx() const { return m_entities_idx; }
  Uint loc_idx() const       { return m_loc_idx; }
  const boost::uint64_t& glb_idx() const       { return m_glb_idx; }
  Uint rank() const { return m_rank; }
  Uint& rank() { return m_rank; }
  const std::vector< std::vector<boost::uint64_t> >& connectivity() const { return m_connectivity; }
  Uint nb_spaces() const { return m_connectivity.size(); }

private:
  const Mesh& m_mesh;

  Uint m_entities_idx ;        ///< Component index as it exists in a vector owned by the mesh
  Uint m_loc_idx;              ///< Element index inside the Entities component
  boost::uint64_t m_glb_idx;   ///< Global index of the element
  Uint m_rank;                 ///< Rank of the element
  /// Per available space, the node connectivity, in global indices
  std::vector< std::vector<boost::uint64_t> > m_connectivity;
};



////////////////////////////////////////////////////////////////////////////////


/// @brief Class that contains all stand-alone global information of point in a dictionary.
///
/// This information can be retrieved from a common::PE::buffer that has been
/// communicated from other ranks, or from local information.
class PackedNode : public common::PE::PackedObject
{
public:

  /// @brief Constructor
  PackedNode(const mesh::Mesh& mesh) : m_mesh(mesh) {}

  /// @brief Constructor, packing from local information
  PackedNode(const mesh::Mesh& mesh, const Uint dict_idx , const Uint node_idx);

  // Unpack from buffer
  virtual void unpack(common::PE::Buffer& buf);

  // Pack in a buffer
  virtual void pack(common::PE::Buffer& buf);

public:

  Uint dict_idx() const { return m_dict_idx; }
  Uint loc_idx() const       { return m_loc_idx; }
  const boost::uint64_t& glb_idx() const       { return m_glb_idx; }
  Uint rank() const { return m_rank; }
  Uint& rank() { return m_rank; }
  const std::vector< std::vector<Real> >& field_values() const { return m_field_values; }

private:

  Uint m_dict_idx;  ///< Component index as it exists in a vector owned by the mesh
  Uint m_loc_idx;   ///< Local index of the node
  boost::uint64_t m_glb_idx;   ///< Global index of the node
  Uint m_rank;                 ///< Rank of the node
  /// Per available field, the node values
  std::vector< std::vector<Real> > m_field_values;
  const Mesh& m_mesh;
};

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_Manipulations_hpp
