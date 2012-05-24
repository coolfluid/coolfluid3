// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_CGNS_Reader_hpp
#define cf3_mesh_CGNS_Reader_hpp

////////////////////////////////////////////////////////////////////////////////

#include "mesh/MeshReader.hpp"
#include "mesh/CGNS/LibCGNS.hpp"
#include "mesh/CGNS/Shared.hpp"

////////////////////////////////////////////////////////////////////////////////
#include "mesh/Elements.hpp"

namespace cf3 {
namespace mesh {
  class Region;
namespace CGNS {

//////////////////////////////////////////////////////////////////////////////

/// This class defines CGNS mesh format reader
/// @author Willem Deconinck
  class Mesh_CGNS_API Reader : public MeshReader, public CGNS::Shared
{
private: // typedefs

  typedef std::pair<Handle<Elements>,Uint> Region_TableIndex_pair;

public: // functions

  /// Contructor
  /// @param name of the component
  Reader ( const std::string& name );

  /// Gets the Class name
  static std::string type_name() { return "Reader"; }

  virtual std::string get_format() { return "CGNS"; }

  virtual std::vector<std::string> get_extensions();

private: // functions

  virtual void do_read_mesh_into(const common::URI& fp, Mesh& mesh);

  void read_base(Mesh& parent_region);
  void read_zone(Mesh& parent_region);
  void read_coordinates_unstructured(Region& parent_region);
  void read_coordinates_structured(Region& parent_region);
  void read_section(Region& parent_region);
  void create_structured_elements(Region& parent_region);
  void read_boco_unstructured(Region& parent_region);
  void read_boco_structured(Region& parent_region);
  void read_flowsolution();
  Uint get_total_nbElements();

  Uint structured_node_idx(Uint i, Uint j, Uint k)
  {
    return i + j*m_zone.nbVertices[XX] + k*m_zone.nbVertices[XX]*m_zone.nbVertices[YY];
  }
  Uint structured_elm_idx(Uint i, Uint j, Uint k)
  {
    return i + j*(m_zone.nbVertices[XX]-1) + k*(m_zone.nbVertices[XX]-1)*(m_zone.nbVertices[YY]-1);
  }

private: // data

  std::vector<Region_TableIndex_pair> m_global_to_region;
  Handle<Mesh> m_mesh;
  Uint m_coord_start_idx;

}; // end Reader


////////////////////////////////////////////////////////////////////////////////

} // CGNS
} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_CGNS_Reader_hpp
