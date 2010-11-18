// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_Gmsh_Shared_hpp
#define CF_Mesh_Gmsh_Shared_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Mesh/GeoShape.hpp"

#include "Mesh/Gmsh/LibGmsh.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace Gmsh {

//////////////////////////////////////////////////////////////////////////////

/// This class defines Gmsh mesh format common functionality
/// @author Willem Deconinck
/// @author Martin Vymazal
class Gmsh_API Shared
{
public:
  
  /// constructor
  Shared();
  
  /// Gets the Class name
  static std::string type_name() { return "Shared"; }
  
  std::vector<std::string>& get_supported_element_types() { return m_supported_types; }

protected:

  /// We use the same convention regarding the numbering of element types and local
  /// numbering of vertices of elements as described in gmsh reference manual:
  /// http://www.geuz.org/gmsh/doc/texinfo/gmsh.html#MSH-ASCII-file-format

  enum GmshElement {P1LINE=1,P1TRIAG=2,P1QUAD=3, P1TETRA=4, P1HEXA=5,
                    P2LINE=8,P2TRIAG=6,P2QUAD=10,P2TETRA=11,P2HEXA= 12};
  
  std::map<GeoShape::Type,Uint> m_CFelement_to_GmshElement;
  std::vector<std::string> m_supported_types;

  /// Faces are not defined in gmsh format
  // std::vector<std::vector<Uint> > m_faces_cf_to_gmsh;
  // std::vector<std::vector<Uint> > m_faces_gmsh_to_cf;

  std::vector<std::vector<Uint> > m_nodes_cf_to_gmsh;
  std::vector<std::vector<Uint> > m_nodes_gmsh_to_cf;

}; // end Shared


////////////////////////////////////////////////////////////////////////////////

} // namespace Gmsh
} // namespace Mesh
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_Gmsh_Shared_hpp
