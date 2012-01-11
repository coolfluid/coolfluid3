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

  class SpaceFields;

  class ElementType;
  class Space;

////////////////////////////////////////////////////////////////////////////////

/// Entities component class
/// This class stores information about a set of elements of the same type
/// @author Willem Deconinck, Tiago Quintino, Bart Janssens
class Mesh_API Entities : public common::Component {

public: // typedefs




public: // functions

  /// Contructor
  /// @param name of the component
  Entities ( const std::string& name );

  /// Initialize the Entities using the given type
  virtual void initialize(const std::string& element_type_name);

  /// Initialize the Entities using the given type, also setting the nodes in one go
  virtual void initialize(const std::string& element_type_name, SpaceFields& geometry);

  /// Set the nodes
  virtual void assign_geometry(SpaceFields& geometry);

  /// Virtual destructor
  virtual ~Entities();

  /// Get the class name
  static std::string type_name () { return "Entities"; }

  /// set the element type
  void configure_element_type();

  /// return the elementType
  ElementType& element_type() const;

  /// Const access to the coordinates
  SpaceFields& geometry_fields() const { cf3_assert(is_not_null(m_geometry_fields)); return *m_geometry_fields; }

  /// Mutable access to the list of nodes
  common::List<Uint>& glb_idx() { return *m_global_numbering; }

  /// Const access to the list of nodes
  const common::List<Uint>& glb_idx() const { return *m_global_numbering; }

  common::List<Uint>& rank() { return *m_rank; }
  const common::List<Uint>& rank() const { return *m_rank; }

  bool is_ghost(const Uint idx) const;

  /// return the number of elements
  virtual Uint size() const;

  static boost::shared_ptr< common::List<Uint> > create_used_nodes(const Component& node_user, const std::string& space_name);
  
  /// return the number of elements across all processes;
  Uint glb_size() const;

  static common::List<Uint>& used_nodes(Component& parent, const bool rebuild=false);

  virtual common::TableConstRow<Uint>::type get_nodes(const Uint elem_idx) const;

  Space& space (const std::string& space_name) const;

  Space& create_space(const std::string& space_name, const std::string& shape_function_builder_name);

  Space& geometry_space() const { cf3_assert(is_not_null(m_geometry_space)); return *m_geometry_space; }

  bool exists_space(const std::string& space_name) const;

  virtual RealMatrix get_coordinates(const Uint elem_idx) const;

  virtual void put_coordinates(RealMatrix& coordinates, const Uint elem_idx) const;

  void allocate_coordinates(RealMatrix& coords) const;

  void signal_create_space ( common::SignalArgs& node );

  void signature_create_space ( common::SignalArgs& node);

  void resize(const Uint nb_elem);

protected: // data

  Handle<ElementType> m_element_type;

  Handle<SpaceFields> m_geometry_fields;

  Handle<Space> m_geometry_space;

  Handle<common::List<Uint> > m_global_numbering;

  Handle<common::Group> m_spaces_group;

  Handle<common::List<Uint> > m_rank;

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
