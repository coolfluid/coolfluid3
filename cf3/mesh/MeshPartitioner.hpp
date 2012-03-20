// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_MeshPartitioner_hpp
#define cf3_mesh_MeshPartitioner_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/tuple/tuple.hpp>

#include "common/FindComponents.hpp"
#include "common/Map.hpp"
#include "common/Foreach.hpp"
#include "common/DynTable.hpp"
#include "common/Table.hpp"
#include "common/List.hpp"

#include "common/PE/Comm.hpp"

#include "mesh/LibMesh.hpp"
#include "mesh/ParallelDistribution.hpp"
#include "mesh/MergedParallelDistribution.hpp"
#include "mesh/Elements.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/MeshTransformer.hpp"
#include "mesh/Connectivity.hpp"
#include "mesh/Space.hpp"

namespace cf3 {
namespace mesh {

  class Mesh;

////////////////////////////////////////////////////////////////////////////////

/// MeshPartitioner component class
/// This class serves as a component that that will partition the mesh
/// @author Willem Deconinck
class Mesh_API MeshPartitioner : public MeshTransformer {

public: // typedefs

  /// type of pointer to Component

  /// type of pointer to constant Component


public: // functions

  /// Contructor
  /// @param name of the component
  MeshPartitioner ( const std::string& name );

  /// Virtual destructor
  virtual ~MeshPartitioner() {}

  /// Get the class name
  static std::string type_name () { return "MeshPartitioner"; }

  virtual void execute();

  void initialize(Mesh& mesh);

  /// Partitioning functions

  virtual void build_graph() = 0;

  virtual void partition_graph() = 0;

  void show_changes();

  /// Migrate the elements and nodes to corresponding processors
  void migrate();

  void load_balance ( common::SignalArgs& args );

  void load_balance_signature ( common::SignalArgs& node );

  /// location finding functions

  void build_global_to_local_index(Mesh& mesh);

  /// Graph building functions

  Uint nb_objects_owned_by_part(const Uint part) const;

  template <typename VectorT>
  void list_of_objects_owned_by_part(const Uint part, VectorT& obj_list) const;

  template <typename VectorT>
  Uint nb_connected_objects_in_part(const Uint part, VectorT& nb_connections_per_obj) const;

  template <typename VectorT>
  void list_of_connected_objects_in_part(const Uint part, VectorT& connections_per_obj) const;

  template <typename VectorT>
  void list_of_connected_procs_in_part(const Uint part, VectorT& proc_per_neighbor) const;


public: // functions

  const std::vector<std::vector<Uint> >& exported_nodes() { return m_nodes_to_export; }

  const std::vector<std::vector<std::vector<Uint> > >& exported_elements() { return m_elements_to_export; }

protected: // functions

  bool is_node(const Uint glb_obj) const
  {
    Uint p = part_of_obj(glb_obj);
    return m_start_node_per_part[p] <= glb_obj && glb_obj < m_end_node_per_part[p];
  }

  bool is_elem(const Uint glb_obj) const
  {
    Uint p = part_of_obj(glb_obj);
    return m_start_node_per_part[p] <= glb_obj && glb_obj < m_end_node_per_part[p];
  }

  boost::tuple<Uint,Uint> location_idx(const Uint glb_obj) const;

  boost::tuple<Handle< common::Component >,Uint> location(const Uint glb_obj) const;

  Uint part_of_obj(const Uint obj) const
  {
    for (Uint p=0; p<m_end_id_per_part.size(); ++p)
    {
      if ( obj < m_end_id_per_part[p])
        return p;
    }
    cf3_assert_desc("[obj " + common::to_str(obj)+ ">"+common::to_str(m_end_id_per_part.back())+" Should not be here", false);
    return 0;
  }

protected: // data

  /// nodes_to_export[part][loc_node_idx]
  std::vector< std::vector<Uint> >                m_nodes_to_export;

  /// elements_to_export[part][elements_comp_idx][loc_elem_idx]
  std::vector< std::vector< std::vector<Uint> > > m_elements_to_export;

private: // data

  Uint m_base;

  Uint m_nb_parts;

  Uint m_nb_owned_obj;


  Handle< common::Map<Uint,Uint> > m_global_to_local;

  std::vector<Uint> m_start_id_per_part;
  std::vector<Uint> m_end_id_per_part;
  std::vector<Uint> m_start_node_per_part;
  std::vector<Uint> m_end_node_per_part;
  std::vector<Uint> m_start_elem_per_part;
  std::vector<Uint> m_end_elem_per_part;

  Handle< UnifiedData > m_lookup;

};

//////////////////////////////////////////////////////////////////////////////

inline Uint MeshPartitioner::nb_objects_owned_by_part(const Uint part) const
{
  return m_nb_owned_obj;
}

//////////////////////////////////////////////////////////////////////////////

template <typename VectorT>
void MeshPartitioner::list_of_objects_owned_by_part(const Uint part, VectorT& obj_list) const
{
  Uint idx=0;
  foreach_container((const Uint glb_obj),*m_global_to_local)
  {
    if (part_of_obj(glb_obj) == part)
      obj_list[idx++] = glb_obj;
  }
}

//////////////////////////////////////////////////////////////////////////////

template <typename VectorT>
Uint MeshPartitioner::nb_connected_objects_in_part(const Uint part, VectorT& nb_connections_per_obj) const
{
  // declaration for boost::tie
  Handle< common::Component > comp;
  Uint loc_idx;
  Uint size = 0;
  Uint idx = 0;
  foreach_container((const Uint glb_obj)(const Uint loc_obj),*m_global_to_local)
  {
    if (part_of_obj(glb_obj) == part)
    {
      boost::tie(comp,loc_idx) = m_lookup->location(loc_obj);

      if (Handle< Dictionary > nodes = Handle<Dictionary>(comp))
      {
        const common::DynTable<Uint>& node_to_glb_elm = nodes->glb_elem_connectivity();
        nb_connections_per_obj[idx] = node_to_glb_elm.row_size(loc_idx);
      }
      else if (Handle< Elements > elements = Handle<Elements>(comp))
      {
        const Connectivity& connectivity_table = elements->geometry_space().connectivity();
        nb_connections_per_obj[idx] = connectivity_table.row_size(loc_idx);
      }
      size += nb_connections_per_obj[idx];
      ++idx;
    }
  }
  cf3_assert_desc(common::to_str(idx)+"!="+common::to_str(nb_objects_owned_by_part(part)), idx == nb_objects_owned_by_part(part));
  return size;
}

//////////////////////////////////////////////////////////////////////////////

template <typename VectorT>
void MeshPartitioner::list_of_connected_objects_in_part(const Uint part, VectorT& connected_objects) const
{
  // declaration for boost::tie
  Handle< common::Component > comp;
  Uint loc_idx;

  Uint idx = 0;
  foreach_container((const Uint glb_obj)(const Uint loc_obj),*m_global_to_local)
  {
    if (part_of_obj(glb_obj) == part)
    {
      boost::tie(comp,loc_idx) = m_lookup->location(loc_obj);
      if (Handle< Dictionary > nodes = Handle<Dictionary>(comp))
      {
        const common::DynTable<Uint>& node_to_glb_elm = nodes->glb_elem_connectivity();
        boost_foreach (const Uint glb_elm , node_to_glb_elm[loc_idx])
          connected_objects[idx++] = glb_elm;
      }
      else if (Handle< Elements > elements = Handle<Elements>(comp))
      {
        const Connectivity& connectivity_table = elements->geometry_space().connectivity();
        const common::List<Uint>& glb_node_indices    = elements->geometry_fields().glb_idx();

        boost_foreach (const Uint loc_node , connectivity_table[loc_idx])
          connected_objects[idx++] = glb_node_indices[ loc_node ];
      }
    }
  }

  std::vector<Uint> edges(m_nb_owned_obj);
  cf3_assert( idx == nb_connected_objects_in_part(part,edges) );
}

//////////////////////////////////////////////////////////////////////////////

template <typename VectorT>
void MeshPartitioner::list_of_connected_procs_in_part(const Uint part, VectorT& connected_procs) const
{
  // declaration for boost::tie
  Handle< common::Component > comp;
  Uint loc_idx;

  Uint idx = 0;
  foreach_container((const Uint glb_obj)(const Uint loc_obj),*m_global_to_local)
  {
    if (part_of_obj(glb_obj) == part)
    {
      boost::tie(comp,loc_idx) = m_lookup->location(loc_obj);
      if (Handle< Dictionary > nodes = Handle<Dictionary>(comp))
      {
        const common::DynTable<Uint>& node_to_glb_elm = nodes->glb_elem_connectivity();
        boost_foreach (const Uint glb_elm , node_to_glb_elm[loc_idx])
          connected_procs[idx++] = part_of_obj(glb_elm); /// @todo should be proc of obj, not part!!!
      }
      else if (Handle< Elements > elements = Handle<Elements>(comp))
      {
        const Connectivity& connectivity_table = elements->geometry_space().connectivity();
        const common::List<Uint>& glb_node_indices    = elements->geometry_fields().glb_idx();
        boost_foreach (const Uint loc_node , connectivity_table[loc_idx])
          connected_procs[idx++] = part_of_obj( glb_node_indices[loc_node] ); /// @todo should be proc of obj, not part!!!
      }
    }
  }
  std::vector<Uint> edges(m_nb_owned_obj);
  cf3_assert( idx == nb_connected_objects_in_part(part,edges) );
}

//////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_MeshPartitioner_hpp
