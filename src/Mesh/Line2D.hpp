// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_Line2D_hpp
#define CF_Mesh_Line2D_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/assign/list_of.hpp>
#include "Mesh/ElementType.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

////////////////////////////////////////////////////////////////////////////////
  
/// This class defines a 2D line mesh element
/// @author Willem Deconinck
struct Mesh_API Line2D : public ElementType
{

  /// constructor
  Line2D(const std::string& name);
  
  /// Gets the Class name
  static std::string type_name() { return "Line2D"; }

  /// @return m_geoShape
  static const GeoShape::Type shape = GeoShape::LINE;
  
  /// @return number of faces
  static const Uint nb_faces = 1;
  
  /// @return number of edges
  static const Uint nb_edges = 2;
  
  /// @return m_dimensionality
  static const Uint dimensionality = DIM_1D;
  
  /// @return m_dimension
  static const Uint dimension = DIM_2D;

}; // end Line2D
  
////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_Line2D_hpp
