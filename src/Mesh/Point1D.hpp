// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_P1_Point1D_hpp
#define CF_Mesh_P1_Point1D_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/assign/list_of.hpp>
#include "Mesh/ElementType.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

////////////////////////////////////////////////////////////////////////////////
  
/// This class defines a 1D point element
/// @author Willem Deconinck
/// @author Bart Janssens
struct Mesh_API Point1D : public ElementType
{

  /// constructor
  Point1D(const std::string& name);
  
  /// Gets the Class name
  static std::string type_name() { return "Point1D"; }

  /// @return m_geoShape
  static const GeoShape::Type shape = GeoShape::POINT;
  
  /// @return number of faces
  static const Uint nb_faces = 0;
  
  /// @return number of edges
  static const Uint nb_edges = 0;
  
  /// @return m_dimensionality
  static const Uint dimensionality = 1;
  
  /// @return m_dimension
  static const Uint dimension = 1;

}; // end Point1D
  
////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_P1_Point1D_hpp
