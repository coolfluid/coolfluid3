// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_Gmsh_CWriter_hpp
#define CF_Mesh_Gmsh_CWriter_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Mesh/CMeshWriter.hpp"
#include "Mesh/GeoShape.hpp"

#include "Mesh/Gmsh/LibGmsh.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace Gmsh {

//////////////////////////////////////////////////////////////////////////////

/// This class defines Gmsh mesh format writer
/// @author Willem Deconinck
class Gmsh_API CWriter : public CMeshWriter
{
public: // typedefs

    typedef boost::shared_ptr<CWriter> Ptr;
    typedef boost::shared_ptr<CWriter const> ConstPtr;

public: // functions

  /// constructor
  CWriter( const std::string& name );

  /// Gets the Class name
  static std::string type_name() { return "CWriter"; }

  virtual void write_from_to(const CMesh::Ptr& mesh, boost::filesystem::path& path);

  virtual std::string get_format() { return "Gmsh"; }

  virtual std::vector<std::string> get_extensions();

private: // functions

  void write_header(std::fstream& file);

  void write_coordinates(std::fstream& file);

  void write_connectivity(std::fstream& file);

	void write_nodal_data(std::fstream& file);
	void write_nodal_data2(std::fstream& file);

  void write_elem_nodal_data(std::fstream& file);
  void write_elem_nodal_data2(std::fstream& file);

  void write_element_data(std::fstream& file);
  void write_element_data2(std::fstream& file);

private: // data

  std::map<std::string,Uint> m_groupnumber;

  std::map<std::string,Uint> m_elementTypes;

  std::map<CEntities*,Uint> m_node_start_idx;
  std::map<CEntities*,Uint> m_element_start_idx;

}; // end CWriter


////////////////////////////////////////////////////////////////////////////////

} // Gmsh
} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_Gmsh_CWriter_hpp
