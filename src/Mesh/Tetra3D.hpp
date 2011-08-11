// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_Tetra3D_hpp
#define CF_Mesh_Tetra3D_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/assign/list_of.hpp>

#include "Mesh/ElementType.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

////////////////////////////////////////////////////////////////////////////////
  
/// This class defines a 2D Triangle mesh element
/// @author Willem Deconinck
struct Mesh_API Tetra3D : public ElementType
{

  /// constructor
  Tetra3D(const std::string& name);
  
  /// Gets the Class name
  static std::string type_name() { return "Tetra3D"; }

  /// @return m_geoShape
  static const GeoShape::Type shape = GeoShape::TETRA;
  
  /// @return number of faces
  static const Uint nb_faces = 4;
  
  /// @return number of edges
  static const Uint nb_edges = 6;
  
  /// @return m_dimensionality
  static const Uint dimensionality = 3;
  
  /// @return m_dimension
  static const Uint dimension = 3;

  virtual Real compute_area(const NodesT& coord) const { return 0.; }

}; // end Tetra3D
  
////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_Tetra3D_hpp
