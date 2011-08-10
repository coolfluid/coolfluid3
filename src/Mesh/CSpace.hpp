// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CSpace_hpp
#define CF_Mesh_CSpace_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Mesh/LibMesh.hpp"
#include "Mesh/ShapeFunction.hpp"
#include "Mesh/CEntities.hpp"
#include "CConnectivity.hpp"

namespace CF {
namespace Common { class CLink; }
namespace Mesh {

  class ElementType;
  class CElements;
  class CConnectivity;
  class FieldGroup;

////////////////////////////////////////////////////////////////////////////////

/// CSpace component class
/// This class stores information about a set of elements of the same type
/// @author Willem Deconinck, Tiago Quintino, Bart Janssens
class Mesh_API CSpace : public Common::Component {

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
  ShapeFunction& shape_function() const { cf_assert(is_not_null(m_shape_function)); return *m_shape_function; }

  /// The geometric support of this space. This is equal to the element type defined in CEntities
  ElementType& element_type() const { return support().element_type(); }

  CEntities& support() const { return parent().as_type<CEntities>(); }

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

private: // functions

  /// Configuration option trigger for the shape function
  void configure_shape_function();

protected: // data

  /// Shape function of this space
  boost::shared_ptr<ShapeFunction> m_shape_function;

  /// node_connectivity or state_connectivity for this space
  boost::shared_ptr<CConnectivity> m_connectivity;

  boost::shared_ptr<Common::CLink> m_bound_fields;

  /// keyword "mutable" means that this variable can be changed using a
  /// const access function.
  /// This is because this is just a temporary storage to mimic the full
  /// connectivity table. This variable is being accessed by the function
  /// indexes_for_element()
  mutable CConnectivity::ArrayT m_connectivity_proxy;

private: // data

  bool m_is_proxy;
  Uint m_elem_start_idx;
};

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CSpace_hpp
