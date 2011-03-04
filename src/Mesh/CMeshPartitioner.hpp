// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CMeshPartitioner_hpp
#define CF_Mesh_CMeshPartitioner_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/tuple/tuple.hpp>

#include "Common/Component.hpp"
#include "Common/ComponentPredicates.hpp"
#include "Common/CMap.hpp"
#include "Common/Foreach.hpp"
#include "Common/MPI/PE.hpp"

#include "Mesh/LibMesh.hpp"
#include "Mesh/CHash.hpp"
#include "Mesh/CMixedHash.hpp"
#include "Mesh/CDynTable.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/CList.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CNodes.hpp"
#include "Mesh/CMesh.hpp"

namespace CF {
namespace Mesh {

  class CMesh;

////////////////////////////////////////////////////////////////////////////////

/// CMeshPartitioner component class
/// This class serves as a component that that will partition the mesh
/// @author Willem Deconinck
class Mesh_API CMeshPartitioner : public Common::Component {

public: // typedefs

  /// type of pointer to Component
  typedef boost::shared_ptr<CMeshPartitioner> Ptr;
  /// type of pointer to constant Component
  typedef boost::shared_ptr<CMeshPartitioner const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CMeshPartitioner ( const std::string& name );

  /// Virtual destructor
  virtual ~CMeshPartitioner() {}

  /// Get the class name
  static std::string type_name () { return "CMeshPartitioner"; }

  void initialize(CMesh& mesh);

  /// Partitioning functions

  virtual void build_graph() = 0;

  virtual void partition_graph() = 0;

  void show_changes();

	/// Migrate the elements and nodes to corresponding processors
	/// @todo this is now virtual because Zoltan is used.
	virtual void migrate();

	void load_balance ( Common::Signal::arg_t& xml );

	void load_balance_signature ( Common::Signal::arg_t& node );

	/// location finding functions

	boost::tuple<Component::Ptr,Uint> to_local(const Uint glb_obj) const;

	void build_global_to_local_index(CMesh& mesh);

	/// Graph building functions

	Uint nb_owned_objects() const;

  template <typename VectorT>
  void list_of_owned_objects(VectorT& obj_list) const;

  template <typename VectorT>
  Uint nb_connected_objects(VectorT& nb_connections_per_obj) const;

  template <typename VectorT>
  void list_of_connected_objects(VectorT& connections_per_obj) const;

  template <typename VectorT>
  void list_of_connected_procs(VectorT& proc_per_neighbor) const;

  bool is_node(const Uint glb_obj) const 
  {
    Uint p = proc_of_obj(glb_obj);
    return m_start_node_per_proc[p] <= glb_obj && glb_obj < m_end_node_per_proc[p];
  }

	Common::CMap<Uint,Uint>& changes() { return (*m_changes);	}

/// @todo must be protected when migration is moved to this class
public: // functions

  boost::tuple<Uint,Uint> to_local_indices_from_loc_obj(const Uint loc_obj) const;

  boost::tuple<Uint,Uint,bool> to_local_indices_from_glb_obj(const Uint glb_obj) const;

  void config_nb_parts();

	Uint proc_of_obj(const Uint obj) const
	{
    for (Uint p=0; p<m_end_id_per_proc.size(); ++p)
    {
      if ( obj < m_end_id_per_proc[p])
        return p;
    }
	}
	
	Uint owns_obj(const Uint obj) const
	{
    return proc_of_obj(obj) == mpi::PE::instance().rank();
	}

	std::vector<Common::Component::Ptr>& components_vector() { return m_local_components; }

protected: // data

	Common::CMap<Uint,Uint>::Ptr m_changes;

private: // data

  Uint m_base;

  Uint m_nb_parts;

  Uint m_nb_owned_obj;

  bool m_map_built;

  std::vector<Common::Component::Ptr> m_local_components;

  std::vector<Uint> m_local_start_index;

  std::vector<Uint> m_local_index;

  Common::CMap<Uint,Uint>::Ptr m_global_to_local;
  
  CMesh::Ptr m_mesh;
  
  std::vector<Uint> m_start_id_per_proc;
  std::vector<Uint> m_end_id_per_proc;
  std::vector<Uint> m_start_node_per_proc;
  std::vector<Uint> m_end_node_per_proc;
  std::vector<Uint> m_start_elem_per_proc;
  std::vector<Uint> m_end_elem_per_proc;
  

};

////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////

inline Uint CMeshPartitioner::nb_owned_objects() const
{
  return m_nb_owned_obj;
}

//////////////////////////////////////////////////////////////////////////////

template <typename VectorT>
void CMeshPartitioner::list_of_owned_objects(VectorT& obj_list) const
{  
  Uint idx=0;
  foreach_container((const Uint glb_obj),*m_global_to_local)
  {
    if (owns_obj(glb_obj))
      obj_list[idx++] = glb_obj;
  }
}

//////////////////////////////////////////////////////////////////////////////

template <typename VectorT>
Uint CMeshPartitioner::nb_connected_objects(VectorT& nb_connections_per_obj) const
{
  // declaration for boost::tie
  Uint component_idx;
  Uint loc_idx;
  Uint size = 0;
  Uint idx = 0;
  foreach_container((const Uint glb_obj)(const Uint loc_obj),*m_global_to_local)
  {
    if (owns_obj(glb_obj))
    {
      boost::tie(component_idx,loc_idx) = to_local_indices_from_loc_obj(loc_obj);
      
      if (is_node(glb_obj))
      {
        const CDynTable<Uint>& node_to_glb_elm = m_local_components[component_idx]->as_type<CNodes>().glb_elem_connectivity();
        nb_connections_per_obj[idx] = node_to_glb_elm.row_size(loc_idx);
      }
      else
      {
        const CTable<Uint>& connectivity_table = m_local_components[component_idx]->as_type<CElements>().connectivity_table();
        nb_connections_per_obj[idx] = connectivity_table.row_size(loc_idx);
      }
      size += nb_connections_per_obj[idx];
      ++idx;
    }
  }
  cf_assert(idx == nb_owned_objects());
  return size;
}

//////////////////////////////////////////////////////////////////////////////

template <typename VectorT>
void CMeshPartitioner::list_of_connected_objects(VectorT& connected_objects) const
{  
  // declaration for boost::tie
  Uint component_idx;
  Uint loc_idx;

  Uint idx = 0;
  foreach_container((const Uint glb_obj)(const Uint loc_obj),*m_global_to_local)
  {
    if (owns_obj(glb_obj))
    {
      boost::tie(component_idx,loc_idx) = to_local_indices_from_loc_obj(loc_obj);
      if (is_node(glb_obj))
      {
        const CDynTable<Uint>& node_to_glb_elm = m_local_components[component_idx]->as_type<CNodes>().glb_elem_connectivity();
        boost_foreach (const Uint glb_elm , node_to_glb_elm[loc_idx])
          connected_objects[idx++] = glb_elm;
      }
      else
      {
        const CTable<Uint>& connectivity_table = m_local_components[component_idx]->as_type<CElements>().connectivity_table();
        const CList<Uint>& glb_node_indices    = m_local_components[component_idx]->as_type<CElements>().nodes().glb_idx();

        boost_foreach (const Uint loc_node , connectivity_table[loc_idx])
          connected_objects[idx++] = glb_node_indices[ loc_node ];
      }
    }
  }

	std::vector<Uint> edges(m_nb_owned_obj);
	cf_assert( idx == nb_connected_objects(edges) );
}

//////////////////////////////////////////////////////////////////////////////

template <typename VectorT>
void CMeshPartitioner::list_of_connected_procs(VectorT& connected_procs) const
{  
  // declaration for boost::tie
  Uint component_idx;
  Uint loc_idx;

  Uint idx = 0;
  foreach_container((const Uint glb_obj)(const Uint loc_obj),*m_global_to_local)
  {
    if (owns_obj(glb_obj))
    {
      boost::tie(component_idx,loc_idx) = to_local_indices_from_loc_obj(loc_obj);
      if (is_node(glb_obj))
      {
        const CDynTable<Uint>& node_to_glb_elm = m_local_components[component_idx]->as_ptr<CNodes>()->glb_elem_connectivity();
        boost_foreach (const Uint glb_elm , node_to_glb_elm[loc_idx])
          connected_procs[idx++] = proc_of_obj(glb_elm);
      }
      else
      {
        const CTable<Uint>& connectivity_table = m_local_components[component_idx]->as_ptr<CElements>()->connectivity_table();
        const CList<Uint>& glb_node_indices    = m_local_components[component_idx]->as_ptr<CElements>()->nodes().glb_idx();
        boost_foreach (const Uint loc_node , connectivity_table[loc_idx])
          connected_procs[idx++] = proc_of_obj( glb_node_indices[loc_node] );
      }
    }
  }
  std::vector<Uint> edges(m_nb_owned_obj);
  cf_assert( idx == nb_connected_objects(edges) );
}

//////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CMeshPartitioner_hpp
