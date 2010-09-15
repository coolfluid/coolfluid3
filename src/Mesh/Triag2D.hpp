// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_P1_Triag2D_hpp
#define CF_Mesh_P1_Triag2D_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/assign/list_of.hpp>

#include "Math/RealMatrix.hpp"

#include "Mesh/ElementType.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

////////////////////////////////////////////////////////////////////////////////
  
/// This class defines a 2D Triangle mesh element
/// @author Willem Deconinck
struct Mesh_API Triag2D : public ElementType
{

  /// constructor
  Triag2D();
  
  /// Gets the Class name
  static std::string type_name() { return "Triag2D"; }

  /// @return m_geoShape
  static const GeoShape::Type shape = GeoShape::TRIAG;
  
  /// @return number of faces
  static const Uint nb_faces = 3;
  
  /// @return number of edges
  static const Uint nb_edges = 3;
  
  /// @return m_dimensionality
  static const Uint dimensionality = 2;
  
  /// @return m_dimension
  static const Uint dimension = 2;

}; // end Triag2D
  
////////////////////////////////////////////////////////////////////////////////

} // namespace Mesh
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_P1_Triag2D_hpp
