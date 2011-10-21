// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_CEntities_hpp
#define cf3_mesh_CEntities_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/EnumT.hpp"

#include "math/MatrixTypes.hpp"
#include "mesh/LibMesh.hpp"
#include "mesh/CTable.hpp"

namespace cf3 {
namespace common { class CLink; class Group;}
namespace mesh {

  template <typename T> class CList;
  class Geometry;
  class ElementType;
  class CSpace;

////////////////////////////////////////////////////////////////////////////////

/// CEntities component class
/// This class stores information about a set of elements of the same type
/// @author Willem Deconinck, Tiago Quintino, Bart Janssens
class Mesh_API CEntities : public common::Component {

public: // typedefs

  typedef boost::shared_ptr<CEntities> Ptr;
  typedef boost::shared_ptr<CEntities const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CEntities ( const std::string& name );

  /// Initialize the CEntities using the given type
  virtual void initialize(const std::string& element_type_name);

  /// Initialize the CEntities using the given type, also setting the nodes in one go
  virtual void initialize(const std::string& element_type_name, Geometry& geometry);

  /// Set the nodes
  virtual void assign_geometry(Geometry& geometry);

  /// Virtual destructor
  virtual ~CEntities();

  /// Get the class name
  static std::string type_name () { return "CEntities"; }

  /// set the element type
  void configure_element_type();

  /// return the elementType
  ElementType& element_type() const;

  /// Const access to the coordinates
  Geometry& geometry() const { cf3_assert(!m_geometry.expired()); return *m_geometry.lock(); }

  /// Mutable access to the list of nodes
  CList<Uint>& glb_idx() { return *m_global_numbering; }

  /// Const access to the list of nodes
  const CList<Uint>& glb_idx() const { return *m_global_numbering; }

  CList<Uint>& rank() { return *m_rank; }
  const CList<Uint>& rank() const { return *m_rank; }

  bool is_ghost(const Uint idx) const;

  /// return the number of elements
  virtual Uint size() const;

  static CList<Uint>& used_nodes(Component& parent, const bool rebuild=false);

  virtual CTable<Uint>::ConstRow get_nodes(const Uint elem_idx) const;

  CSpace& space (const std::string& space_name) const;

  CSpace& create_space(const std::string& space_name, const std::string& shape_function_builder_name);

  CSpace& geometry_space() const { cf3_assert(!m_geometry_space.expired()); return *m_geometry_space.lock(); }

  bool exists_space(const Uint space_idx) const;

  bool exists_space(const std::string& space_name) const;

  Uint space_idx(const std::string& space_name) const;

  virtual RealMatrix get_coordinates(const Uint elem_idx) const;

  virtual void put_coordinates(RealMatrix& coordinates, const Uint elem_idx) const;

  void allocate_coordinates(RealMatrix& coords) const;

  void signal_create_space ( common::SignalArgs& node );

  void signature_create_space ( common::SignalArgs& node);

protected: // data

  boost::shared_ptr<ElementType> m_element_type;

  boost::weak_ptr<Geometry> m_geometry;

  boost::weak_ptr<CSpace> m_geometry_space;

  boost::shared_ptr<CList<Uint> > m_global_numbering;

  boost::shared_ptr<common::Group> m_spaces_group;

  boost::shared_ptr<CList<Uint> > m_rank;

};

////////////////////////////////////////////////////////////////////////////////

class IsElementsVolume
{
public:
  IsElementsVolume () {}

  bool operator()(const CEntities::Ptr& component);
  bool operator()(const CEntities& component);
};

class IsElementsSurface
{
public:
  IsElementsSurface () {}

  bool operator()(const CEntities::Ptr& component);
  bool operator()(const CEntities& component);
};

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_CEntities_hpp
