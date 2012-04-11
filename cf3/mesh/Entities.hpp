// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_Entities_hpp
#define cf3_mesh_Entities_hpp

////////////////////////////////////////////////////////////////////////////////

#include <iosfwd>

#include "common/Table_fwd.hpp"

#include "math/MatrixTypes.hpp"
#include "mesh/LibMesh.hpp"

namespace cf3 {
namespace common { class Link; class Group;   template <typename T> class List;}
namespace mesh {

  class Dictionary;
  class Entity;
  typedef common::Table<Entity> ElementConnectivity;
  class FaceCellConnectivity;
  class ElementType;
  class Space;

////////////////////////////////////////////////////////////////////////////////

/// Entities component class
/// This class stores information about a set of elements of the same type
/// @author Willem Deconinck, Tiago Quintino, Bart Janssens
class Mesh_API Entities : public common::Component
{
  friend class Mesh;
public: // functions

  /// Contructor
  /// @param name of the component
  Entities ( const std::string& name );

  /// Initialize the Entities using the given type, also setting the nodes in one go
  virtual void initialize(const std::string& element_type_name, Dictionary& geometry);

  /// Set the nodes
  virtual void create_geometry_space(Dictionary& geometry);

  /// Virtual destructor
  virtual ~Entities();

  /// Get the class name
  static std::string type_name () { return "Entities"; }

  /// set the element type
  void configure_element_type();

  /// return the elementType
  ElementType& element_type() const;

  /// Const access to the coordinates
  Dictionary& geometry_fields() const { cf3_assert(is_not_null(m_geometry_dict)); return *m_geometry_dict; }

  /// Mutable access to the list of nodes
  common::List<Uint>& glb_idx() { return *m_global_numbering; }

  /// Const access to the list of nodes
  const common::List<Uint>& glb_idx() const { return *m_global_numbering; }

  common::List<Uint>& rank() { return *m_rank; }
  const common::List<Uint>& rank() const { return *m_rank; }

  bool is_ghost(const Uint idx) const;

  /// return the number of elements
  Uint size() const;
  
  /// return the number of elements across all processes;
  Uint glb_size() const;

  /// @deprecated Will be removed soon, use mesh/Functions.hpp --> create_used_nodes_list
  static common::List<Uint>& used_nodes(Component& parent, const bool rebuild=false);

  Space& space (const Dictionary& dict);
  const Space& space (const Dictionary& dict) const;

  std::vector< Handle<Space> > spaces() { return m_spaces_vector; }

  const std::vector< Handle<Space> > spaces() const { return m_spaces_vector; }

  Space& create_space(const std::string& shape_function_builder_name, Dictionary& space_fields);

  Space& geometry_space() const { cf3_assert(is_not_null(m_geometry_space)); return *m_geometry_space; }

  void resize(const Uint nb_elem);

  Uint entities_idx() const { return m_entities_idx; }

protected: // data

  Handle<ElementType> m_element_type;

  Handle<Dictionary> m_geometry_dict;

  Handle<Space> m_geometry_space;

  Handle<common::List<Uint> > m_global_numbering;

  Handle<common::Group> m_spaces_group;
  std::vector< Handle<Space> > m_spaces_vector;

  Handle<common::List<Uint> > m_rank;

// shortcuts to connectivity tables, otherwise expensive to access in loops

public:

  Handle<ElementConnectivity>&  connectivity_cell2face() { return m_connectivity_cell2face; }
  Handle<FaceCellConnectivity>& connectivity_face2cell() { return m_connectivity_face2cell; }
  Handle<ElementConnectivity>&  connectivity_cell2cell() { return m_connectivity_cell2cell; }
  const Handle<ElementConnectivity>&  connectivity_cell2face() const { return m_connectivity_cell2face; }
  const Handle<FaceCellConnectivity>& connectivity_face2cell() const { return m_connectivity_face2cell; }
  const Handle<ElementConnectivity>&  connectivity_cell2cell() const { return m_connectivity_cell2cell; }

private:

  Handle<ElementConnectivity>  m_connectivity_cell2face;
  Handle<FaceCellConnectivity> m_connectivity_face2cell;
  Handle<ElementConnectivity>  m_connectivity_cell2cell;

  /// @brief index as it appears in mesh.elements()
  Uint m_entities_idx;
};

////////////////////////////////////////////////////////////////////////////////

/// @brief Low storage struct to uniquely identify one element
///
/// Several access-functions are provided for convenience
class Mesh_API Entity
{
public:
  Entity() : comp(NULL), idx(0) {}
  Entity(const Entity& other) : comp(other.comp), idx(other.idx) {}

  Entity(Entities& entities, const Uint index=0) :
    comp( &entities ),
    idx(index)
  {
    cf3_assert(idx<comp->size());
  }

  Entities* comp;
  Uint idx;


  static std::string type_name() { return "Entity"; }

  /// return the elementType
  ElementType& element_type() const;
  Uint glb_idx() const;
  Uint rank() const;
  bool is_ghost() const;
  RealMatrix get_coordinates() const;
  void put_coordinates(RealMatrix& coordinates) const;
  void allocate_coordinates(RealMatrix& coordinates) const;
  common::TableConstRow<Uint>::type get_nodes() const;


  Entity& operator++()
    {  cf3_assert(idx<comp->size()); idx++; return *this; }
  Entity& operator--()
    {  cf3_assert(idx>=0u); idx--; return *this; }
  bool operator==(const Entity& other) const { return comp==other.comp && idx==other.idx; }
  bool operator!=(const Entity& other) const { return !(*this==other);  }

  friend std::ostream& operator<<(std::ostream& os, const Entity& entity);

};

std::ostream& operator<<(std::ostream& os, const Entity& entity);

////////////////////////////////////////////////////////////////////////////////

class IsElementsVolume
{
public:
  IsElementsVolume () {}

  bool operator()(const Handle< Entities >& component);
  bool operator()(const Entities& component);
};

class IsElementsSurface
{
public:
  IsElementsSurface () {}

  bool operator()(const Handle< Entities >& component);
  bool operator()(const Entities& component);
};

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_Entities_hpp
