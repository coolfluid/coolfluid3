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

namespace CF {
namespace Mesh {

  class ElementType;
  class CElements;

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
  const ShapeFunction& shape_function() const { cf_assert(is_not_null(m_shape_function)); return *m_shape_function; }

  /// The geometric support of this space. This is equal to the element type defined in CEntities
  const ElementType& element_type() const { return parent()->as_type<CEntities>().element_type(); }

  Uint nb_states() const { return shape_function().nb_nodes(); }
  
protected: // data

  void configure_shape_function();

  boost::shared_ptr<ShapeFunction> m_shape_function;

};

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CSpace_hpp
