// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_CSpace_hpp
#define cf3_mesh_CSpace_hpp

////////////////////////////////////////////////////////////////////////////////

#include "mesh/LibMesh.hpp"
#include "mesh/ShapeFunction.hpp"
#include "mesh/CEntities.hpp"
#include "CConnectivity.hpp"

namespace cf3 {
namespace common { class CLink; }
namespace mesh {

  class ElementType;
  class CElements;
  class CConnectivity;
  class FieldGroup;

////////////////////////////////////////////////////////////////////////////////

/// CSpace component class
/// This class stores information about a set of elements of the same type
/// @author Willem Deconinck, Tiago Quintino, Bart Janssens
class Mesh_API CSpace : public common::Component {

public: // typedefs

  typedef boost::shared_ptr<CSpace> Ptr;
  typedef boost::shared_ptr<CSpace const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CSpace ( const std::string& name );

  /// Virtual destructor
  virtual ~CSpace();

  /// Get the class name
  static std::string type_name () { return "CSpace"; }

  /// return the elementType
  /// @pre the shape function must be configured first
  const ShapeFunction& shape_function() const;

  /// The geometric support of this space. This is equal to the element type defined in CEntities
  ElementType& element_type() { return support().element_type(); }
  const ElementType& element_type() const { return support().element_type(); }

  /// Set the geometric support that is associated with this space
  void set_support(CEntities& support);

  /// Access the geometric support
  /// @return a reference to the entities
  /// @throws SetupError if not set.
  CEntities& support();
  const CEntities& support() const;

  /// The number of nodes or states this element shape function provides
  Uint nb_states() const { return shape_function().nb_nodes(); }

  /// Return the node_connectivity table
  /// @pre node connectivity must have been created beforehand
  CConnectivity& connectivity() { return *m_connectivity; }

  /// Return the node_connectivity table
  /// @pre node connectivity must have been created beforehand
  const CConnectivity& connectivity() const { return *m_connectivity; }

  CConnectivity::ConstRow indexes_for_element(const Uint elem_idx) const;

  bool is_bound_to_fields() const;

  FieldGroup& bound_fields() const;

  void make_proxy(const Uint elem_start_idx);

  RealMatrix compute_coordinates(const Uint elem_idx) const;

  RealMatrix get_coordinates(const Uint elem_idx) const;

  void put_coordinates(RealMatrix& coordinates, const Uint elem_idx) const;

  void allocate_coordinates(RealMatrix& coordinates) const;

  /// Gives the index relative to global field arrays
  /// @returns the index in global element-based arrays for the first element
  Uint elements_begin() const { return m_elem_start_idx; }

private: // functions

  /// Configuration option trigger for the shape function
  void configure_shape_function();

protected: // data

  /// Shape function of this space
  boost::shared_ptr<ShapeFunction> m_shape_function;

  /// node_connectivity or state_connectivity for this space
  boost::shared_ptr<CConnectivity> m_connectivity;

  boost::shared_ptr<common::CLink> m_bound_fields;

  /// keyword "mutable" means that this variable can be changed using a
  /// const access function.
  /// This is because this is just a temporary storage to mimic the full
  /// connectivity table. This variable is being accessed by the function
  /// indexes_for_element()
  mutable CConnectivity::ArrayT m_connectivity_proxy;

private: // data

  bool m_is_proxy;
  Uint m_elem_start_idx;
  boost::weak_ptr<CEntities> m_support;
};

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_CSpace_hpp
