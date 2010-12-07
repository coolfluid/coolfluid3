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

#include "Mesh/LibMesh.hpp"
#include "Mesh/CHash.hpp"
#include "Mesh/CMixedHash.hpp"
#include "Mesh/CDynTable.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/CList.hpp"
#include "Mesh/CElements.hpp"


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
  
  /// location finding functions
    
  void build_global_to_local_index(CMesh& mesh);
  
  boost::tuple<Component::Ptr,Uint> to_local(const Uint glb_obj) const;
  
  
  /// Graph building functions
  
  Uint nb_owned_objects() const;
  
  template <typename VectorT>
  void list_of_owned_objects(VectorT& obj_list) const;
  
  template <typename VectorT>
  void nb_connected_objects(VectorT& nb_connections_per_obj) const;

  template <typename VectorT>  
  void list_of_connected_objects(VectorT& connections_per_obj) const;

  template <typename VectorT>
  void list_of_connected_procs(VectorT& proc_per_neighbor) const;

  bool is_node(const Uint glb_obj) const { return m_hash->subhash_of_obj(glb_obj) == NODES; }
  
	
	Uint from_node_glb(Uint glb) const
	{
	  Uint part = m_hash->subhash(NODES)->part_of_obj(glb);
		if (part == 0)
			return glb;
		else
		{
      Uint offset = glb - m_hash->subhash(NODES)->start_idx_in_part(part);
      return m_hash->start_idx_in_part(part) + offset;
		}
	}
	
	Uint from_elem_glb(Uint glb) const
	{
	  Uint part = m_hash->subhash(ELEMS)->part_of_obj(glb);
		if (part == 0)
			return m_hash->subhash(NODES)->part_size()+glb;
		else
		{
      Uint offset = glb - m_hash->subhash(ELEMS)->start_idx_in_part(part);
      return m_hash->start_idx_in_part(part) + m_hash->subhash(NODES)->nb_objects_in_part(part) + offset;
		}
	}

	Uint to_node_glb(Uint glb) const
	{
		Uint part = m_hash->part_of_obj(glb);
		if (part == 0)
      return glb;
		else
		{
			Uint offset = glb - m_hash->start_idx_in_part(part);
			return m_hash->subhash(NODES)->start_idx_in_part(part) + offset;
		}
	}
	
	Uint to_elem_glb(Uint glb) const
	{
		Uint part = m_hash->part_of_obj(glb);
		if (part == 0)
      return glb - m_hash->subhash(NODES)->part_size();
		else
		{
      Uint offset = glb - m_hash->start_idx_in_part(part) - m_hash->subhash(NODES)->nb_objects_in_part(part);;
			return m_hash->subhash(ELEMS)->start_idx_in_part(part) + offset;
		}
	}
	
private: // functions

  boost::tuple<Uint,Uint> to_local_indices_from_loc_obj(const Uint loc_obj) const;

  boost::tuple<Uint,Uint,bool> to_local_indices_from_glb_obj(const Uint glb_obj) const;

  void config_nb_parts();

protected: // data

  Common::CMap<Uint,Uint>::Ptr m_changes;

private: // data

  Uint m_base;

  Uint m_nb_parts;

  Uint m_nb_owned_obj;

  bool m_map_built;

  std::vector<Common::Component::Ptr> m_local_components;

  std::vector<Common::Component::ConstPtr> m_glb_idx_components;

  std::vector<Common::Component::ConstPtr> m_connectivity_components;

  std::vector<Uint> m_local_start_index;

  std::vector<Uint> m_local_index;

  Common::CMap<Uint,Uint>::Ptr m_global_to_local;

  CMixedHash::Ptr m_hash;
  enum HashType {NODES=0, ELEMS=1};

};

////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////

Uint CMeshPartitioner::nb_owned_objects() const
{
  return m_nb_owned_obj;
}

//////////////////////////////////////////////////////////////////////////////

template <typename VectorT>
void CMeshPartitioner::list_of_owned_objects(VectorT& obj_list) const
{
  Uint idx=0;
  foreach_container((const Uint glb_obj)(const Uint loc_obj),*m_global_to_local)
  {
    if (m_hash->owns(glb_obj))
      obj_list[idx++] = glb_obj;
  }
}

//////////////////////////////////////////////////////////////////////////////

template <typename VectorT>
void CMeshPartitioner::nb_connected_objects(VectorT& nb_connections_per_obj) const
{
  // declaration for boost::tie
  Uint component_idx;
  Uint loc_idx;

  Uint idx = 0;
  foreach_container((const Uint glb_obj)(const Uint loc_obj),*m_global_to_local)
  {
    if (m_hash->owns(glb_obj))
    {
      boost::tie(component_idx,loc_idx) = to_local_indices_from_loc_obj(loc_obj);
      if (is_node(glb_obj))
      {
        const CDynTable<Uint>& node_to_glb_elm = *m_connectivity_components[component_idx]->as_type<CDynTable<Uint> >();
        nb_connections_per_obj[idx] = node_to_glb_elm.row_size(loc_idx);
      }
      else
      {
        const CTable<Uint>& connectivity_table = *m_connectivity_components[component_idx]->as_type<CTable<Uint> >();
        nb_connections_per_obj[idx] = connectivity_table.row_size(loc_idx);
      }
			++idx;
    }
  }
  cf_assert(idx == nb_owned_objects());
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
    if (m_hash->owns(glb_obj))
    {	
      boost::tie(component_idx,loc_idx) = to_local_indices_from_loc_obj(loc_obj);
      if (is_node(glb_obj))
      {
        const CDynTable<Uint>& node_to_glb_elm = *m_connectivity_components[component_idx]->as_type<CDynTable<Uint> >();
        boost_foreach (const Uint glb_elm , node_to_glb_elm[loc_idx])
          connected_objects[idx++] = from_elem_glb(glb_elm);
      }
      else
      {
        const CTable<Uint>& connectivity_table = *m_connectivity_components[component_idx]->as_type<CTable<Uint> >();
        const CList<Uint>& glb_node_indices = *m_glb_idx_components[component_idx]->as_type<CList<Uint> >();
        boost_foreach (const Uint loc_node , connectivity_table[loc_idx])
          connected_objects[idx++] = from_node_glb(glb_node_indices[ loc_node ]);
      }
    }
  }
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
    if (m_hash->owns(glb_obj))
    {
      boost::tie(component_idx,loc_idx) = to_local_indices_from_loc_obj(loc_obj);
      if (is_node(glb_obj))
      {
        const CDynTable<Uint>& node_to_glb_elm = *m_connectivity_components[component_idx]->as_type<CDynTable<Uint> >();
        boost_foreach (const Uint glb_elm , node_to_glb_elm[loc_idx])
        {
          connected_procs[idx++] = m_hash->subhash(ELEMS)->proc_of_obj(glb_elm);
        }
      }
      else
      {
        const CTable<Uint>& connectivity_table = *m_connectivity_components[component_idx]->as_type<CTable<Uint> >();
        const CList<Uint>& glb_node_indices = *m_glb_idx_components[component_idx]->as_type<CList<Uint> >();
        boost_foreach (const Uint loc_node , connectivity_table[loc_idx])
        {
          connected_procs[idx++] = m_hash->subhash(NODES)->proc_of_obj( glb_node_indices[loc_node] );
        }
      }
    }
  }
}

//////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CMeshPartitioner_hpp
