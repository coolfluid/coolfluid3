// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text  .
#ifndef cf3_mesh_Gmsh_Shared_hpp
#define cf3_mesh_Gmsh_Shared_hpp

////////////////////////////////////////////////////////////////////////////////

#include "mesh/GeoShape.hpp"

#include "mesh/gmsh/LibGmsh.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
namespace gmsh {

//////////////////////////////////////////////////////////////////////////////

/// This class defines gmsh mesh format common functionality
/// @author Willem Deconinck
/// @author Martin Vymazal
class gmsh_API Shared
{
public:
  
  /// constructor
  Shared();
  
  /// Gets the Class name
  static std::string type_name() { return "Shared"; }
  
  std::vector<std::string>& get_supported_element_types() { return m_supported_types; }

  static std::string gmsh_name_to_cf_name(const Uint dim, const Uint gmsh_type);

protected:

  /// We use the same convention regarding the numbering of element types and local
  /// numbering of vertices of elements as described in gmsh reference manual:
  /// http://www.geuz.org/gmsh/doc/texinfo/gmsh.html#MSH-ASCII-file-format
  /// NOTE THAT THE NUMBER OF GMSH TYPES LISTED HERE IS BY 1 BIGGER THAT THE ACTUAL NUMBER OF
  /// GMSH TYPES. THIS IS BECAUSE WE START COUNTING FROM 1 AND END WITH nb_gmsh_types
  /// (THE FIRST ELEMENT TYPE ON THE OFFICIAL GMSH LIST - P1 line - IS TAGGED AS '1')


  static const Uint nb_gmsh_types = 37;

  static const Uint m_nodes_in_gmsh_elem[nb_gmsh_types]; //For each element type, remember how many nodes it has
  static const Uint m_gmsh_elem_dim[nb_gmsh_types];      //Store what is the geometrical dimension of each gmsh elem
  static const Uint m_gmsh_elem_order[nb_gmsh_types];    //Store the order of each element type
  static const std::string gmsh_elem_geo_name[nb_gmsh_types]; //Give names to the types
  static const std::string dim_name[4];
  static const std::string order_name[10];

  enum gmshElement { P1LINE=1,   P1TRIAG=2,  P1QUAD=3,  P1TETRA=4,  P1HEXA=5, P1PRISM=6,
                     P2LINE=8,   P2TRIAG=9,  P2QUAD=10, P2TETRA=11, P2HEXA=12,
                     P0POINT=15, P3TRIAG=21, P3LINE=26, P3QUAD = 36 };
  
  std::map<GeoShape::Type,Uint> m_CFelement_to_GmshElement;
  std::vector<std::string> m_supported_types;

  /// Faces are not defined in gmsh format
  // std::vector<std::vector<Uint> > m_faces_cf_to_gmsh;
  // std::vector<std::vector<Uint> > m_faces_gmsh_to_cf;

  std::vector<std::vector<Uint> > m_nodes_cf_to_gmsh;
  std::vector<std::vector<Uint> > m_nodes_gmsh_to_cf;

}; // end Shared


////////////////////////////////////////////////////////////////////////////////

} // gmsh
} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_Gmsh_Shared_hpp
