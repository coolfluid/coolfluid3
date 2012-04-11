// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_Gmsh_Writer_hpp
#define cf3_mesh_Gmsh_Writer_hpp

////////////////////////////////////////////////////////////////////////////////

#include "mesh/MeshWriter.hpp"
#include "mesh/GeoShape.hpp"

#include "mesh/gmsh/LibGmsh.hpp"
#include "mesh/gmsh/Shared.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common { template <typename KEY, typename DATA> class Map; }
namespace mesh {
  class Entities;
namespace gmsh {

//////////////////////////////////////////////////////////////////////////////

/// This class defines gmsh mesh format writer
/// @author Willem Deconinck
class gmsh_API Writer : public MeshWriter, public gmsh::Shared
{

public: // functions

  /// constructor
  Writer( const std::string& name );

  /// Gets the Class name
  static std::string type_name() { return "Writer"; }

  virtual std::string get_format() { return "Gmsh"; }

  virtual std::vector<std::string> get_extensions();

private: // functions

  virtual void write();

  void write_header(std::fstream& file);

  void write_coordinates(std::fstream& file);

  void write_connectivity(std::fstream& file);

//  void write_nodal_data(std::fstream& file);

  void write_elem_nodal_data(std::fstream& file);

//  void write_element_data(std::fstream& file);

private: // data

  std::map<std::string,Uint> m_groupnumber;

  std::map<std::string,Uint> m_elementTypes;

  std::vector< Handle<Entities const> > m_entities_vector;
}; // end Writer


////////////////////////////////////////////////////////////////////////////////

} // gmsh
} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_Gmsh_Writer_hpp
