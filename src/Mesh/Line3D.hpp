// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_Line3D_hpp
#define CF_Mesh_Line3D_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/assign/list_of.hpp>

#include "Mesh/ElementType.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

////////////////////////////////////////////////////////////////////////////////
  
/// This class defines a 3D line mesh element
/// @author Willem Deconinck
struct Mesh_API Line3D : public ElementType
{

  /// constructor
  Line3D(const std::string& name);
  
  /// Gets the Class name
  static std::string type_name() { return "Line3D"; }

  /// @return m_geoShape
  static const GeoShape::Type shape = GeoShape::LINE;
  
  /// @return number of faces
  static const Uint nb_faces = 0;
  
  /// @return number of edges
  static const Uint nb_edges = 1;
  
  /// @return m_dimensionality
  static const Uint dimensionality = DIM_1D;
  
  /// @return m_dimension
  static const Uint dimension = DIM_3D;

}; // end Line3D
  
////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_Line3D_hpp
