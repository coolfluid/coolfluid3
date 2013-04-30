// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text  .
#ifndef cf3_mesh_tecplot_Shared_hpp
#define cf3_mesh_tecplot_Shared_hpp

////////////////////////////////////////////////////////////////////////////////

#include "mesh/GeoShape.hpp"

#include "mesh/tecplot/LibTecplot.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
namespace tecplot {

//////////////////////////////////////////////////////////////////////////////

/// This class defines tecplot mesh format common functionality
/// @author Willem Deconinck
class tecplot_API Shared
{
public:
  
  /// constructor
  Shared();
  
  /// Gets the Class name
  static std::string type_name() { return "Shared"; }
  
  std::vector<std::string>& get_supported_element_types() { return m_supported_types; }

  static std::string tp_name_to_cf_name(const Uint dim, const Uint tp_type);

protected:


  static const Uint nb_tp_types = 37;

  static const Uint m_nodes_in_tp_elem[nb_tp_types]; //For each element type, remember how many nodes it has
  static const Uint m_tp_elem_dim[nb_tp_types];      //Store what is the geometrical dimension of each tp elem
  static const Uint m_tp_elem_order[nb_tp_types];    //Store the order of each element type
  static const std::string tp_elem_geo_name[nb_tp_types]; //Give names to the types
  static const std::string dim_name[4];
  static const std::string order_name[10];

  enum tecplotElement { P1LINE=1,   P1TRIAG=2,  P1QUAD=3,  P1TETRA=4,  P1HEXA=5,
                     P2LINE=8,   P2TRIAG=9,  P2QUAD=10, P2TETRA=11, P2HEXA=12,
                     P1POINT=15, P3TRIAG=21, P3LINE=26, P3QUAD = 36 };
  
  std::map<GeoShape::Type,Uint> m_element_cf_to_tp;
  std::vector<std::string> m_supported_types;

  /// Faces are not defined in tp format
  // std::vector<std::vector<Uint> > m_faces_cf_to_tp;
  // std::vector<std::vector<Uint> > m_faces_tp_to_cf;

  std::vector<std::vector<Uint> > m_nodes_cf_to_tp;
  std::vector<std::vector<Uint> > m_nodes_tp_to_cf;

}; // end Shared


////////////////////////////////////////////////////////////////////////////////

} // tecplot
} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_tecplot_Shared_hpp
