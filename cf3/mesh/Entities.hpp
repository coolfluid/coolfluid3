// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_Entities_hpp
#define cf3_mesh_Entities_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/EnumT.hpp"

#include "math/MatrixTypes.hpp"
#include "mesh/LibMesh.hpp"
#include "common/Table.hpp"

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

  typedef boost::shared_ptr<Entities> Ptr;
  typedef boost::shared_ptr<Entities const> ConstPtr;

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
  SpaceFields& geometry_fields() const { cf3_assert(!m_geometry_fields.expired()); return *m_geometry_fields.lock(); }

  /// Mutable access to the list of nodes
  common::List<Uint>& glb_idx() { return *m_global_numbering; }

  /// Const access to the list of nodes
  const common::List<Uint>& glb_idx() const { return *m_global_numbering; }

  common::List<Uint>& rank() { return *m_rank; }
  const common::List<Uint>& rank() const { return *m_rank; }

  bool is_ghost(const Uint idx) const;

  /// return the number of elements
  virtual Uint size() const;

  static common::List<Uint>& used_nodes(Component& parent, const bool rebuild=false);

  virtual common::Table<Uint>::ConstRow get_nodes(const Uint elem_idx) const;

  Space& space (const std::string& space_name) const;

  Space& create_space(const std::string& space_name, const std::string& shape_function_builder_name);

  Space& geometry_space() const { cf3_assert(!m_geometry_space.expired()); return *m_geometry_space.lock(); }

  bool exists_space(const std::string& space_name) const;

  virtual RealMatrix get_coordinates(const Uint elem_idx) const;

  virtual void put_coordinates(RealMatrix& coordinates, const Uint elem_idx) const;

  void allocate_coordinates(RealMatrix& coords) const;

  void signal_create_space ( common::SignalArgs& node );

  void signature_create_space ( common::SignalArgs& node);

protected: // data

  boost::shared_ptr<ElementType> m_element_type;

  boost::weak_ptr<SpaceFields> m_geometry_fields;

  boost::weak_ptr<Space> m_geometry_space;

  boost::shared_ptr<common::List<Uint> > m_global_numbering;

  boost::shared_ptr<common::Group> m_spaces_group;

  boost::shared_ptr<common::List<Uint> > m_rank;

};

////////////////////////////////////////////////////////////////////////////////

class IsElementsVolume
{
public:
  IsElementsVolume () {}

  bool operator()(const Entities::Ptr& component);
  bool operator()(const Entities& component);
};

class IsElementsSurface
{
public:
  IsElementsSurface () {}

  bool operator()(const Entities::Ptr& component);
  bool operator()(const Entities& component);
};

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_Entities_hpp
