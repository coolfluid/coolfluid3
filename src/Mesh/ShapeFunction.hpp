// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_ShapeFunction_hpp
#define CF_Mesh_ShapeFunction_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"

#include "Math/MatrixTypes.hpp"

#include "Mesh/GeoShape.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

////////////////////////////////////////////////////////////////////////////////

/// This class represents the the data related to an ShapeFunction
/// @author Willem Deconinck
class Mesh_API ShapeFunction : public Common::Component {

public: // typedefs

  typedef boost::shared_ptr< ShapeFunction > Ptr;
  typedef boost::shared_ptr< ShapeFunction const> ConstPtr;

public: // functions

  /// Default constructor without arguments
  ShapeFunction( const std::string& name = type_name() );

  /// Default destructor
  virtual ~ShapeFunction();

  /// Type name: ShapeFunction
  static std::string type_name() { return "ShapeFunction"; }
  
  /// @return shape as string
  std::string shape_name() const { return Mesh::GeoShape::Convert::instance().to_str( m_shape ); }

  /// @return shape as enumerator
  GeoShape::Type shape() const  {  return m_shape; }

  /// @return number of nodes
  Uint nb_nodes() const  { return m_nb_nodes; }

  /// @return order
  Uint order() const { return m_order; }

  /// @return dimensionality (e.g. shell in 3D world: dimensionality = 2)
  Uint dimensionality() const { return m_dimensionality; }

  virtual void compute_value(const RealVector& local_coord, RealRowVector& result);

  virtual void compute_gradient(const RealVector& local_coord, RealMatrix& result);

protected: // data

  /// the GeoShape::Type corresponding to the shape
  GeoShape::Type m_shape;
  /// the number of nodes used in this shape function
  Uint m_nb_nodes;
  /// the order of this shape function
  Uint m_order;
  /// the dimensionality of the element
  Uint m_dimensionality;

}; // ShapeFunction

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_ShapeFunction_hpp
