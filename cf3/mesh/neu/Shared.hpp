// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_neu_Shared_hpp
#define cf3_mesh_neu_Shared_hpp

////////////////////////////////////////////////////////////////////////////////

#include <set>

#include "mesh/GeoShape.hpp"

#include "mesh/neu/LibNeu.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
namespace neu {

//////////////////////////////////////////////////////////////////////////////

/// This class defines neu mesh format common functionality
/// @author Willem Deconinck
class neu_API Shared
{
public:
  
  /// constructor
  Shared();
  
  /// Gets the Class name
  static std::string type_name() { return "Shared"; }
  
  std::vector<std::string>& get_supported_element_types() { return m_supported_types; }

protected:

  enum neuElement {LINE=1,QUAD=2,TRIAG=3,HEXA=4, PRISM=5, TETRA=6};
  
  std::map<GeoShape::Type,Uint> m_CFelement_to_neuElement;
  std::vector<std::string> m_supported_types;    
  std::vector<std::vector<Uint> > m_faces_cf_to_neu;
  std::vector<std::vector<Uint> > m_faces_neu_to_cf;
  std::vector<std::vector<Uint> > m_nodes_cf_to_neu;
  std::vector<std::vector<Uint> > m_nodes_neu_to_cf;

  std::set<Uint> m_supported_neu_types;

}; // end Shared


////////////////////////////////////////////////////////////////////////////////

} // neu
} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_neu_Shared_hpp
