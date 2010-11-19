// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_P1_Line1D_hpp
#define CF_Mesh_P1_Line1D_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/assign/list_of.hpp>
#include "Mesh/ElementType.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

////////////////////////////////////////////////////////////////////////////////
  
/// This class defines a 2D Triangle mesh element
/// @author Willem Deconinck
struct Mesh_API Line1D : public ElementType
{

  /// constructor
  Line1D();
  
  /// Gets the Class name
  static std::string type_name() { return "Line1D"; }

  /// @return m_geoShape
  static const GeoShape::Type shape = GeoShape::LINE;
  
  /// @return number of faces
  static const Uint nb_faces = 0;
  
  /// @return number of edges
  static const Uint nb_edges = 1;
  
  /// @return m_dimensionality
  static const Uint dimensionality = 1;
  
  /// @return m_dimension
  static const Uint dimension = 1;

}; // end Line1D
  
////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_P1_Line1D_hpp
