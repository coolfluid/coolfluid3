// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_Gmsh_CWriter_hpp
#define cf3_mesh_Gmsh_CWriter_hpp

////////////////////////////////////////////////////////////////////////////////

#include "mesh/MeshWriter.hpp"
#include "mesh/GeoShape.hpp"

#include "mesh/Gmsh/LibGmsh.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common { template <typename KEY, typename DATA> class Map; }
namespace mesh {
namespace Gmsh {

//////////////////////////////////////////////////////////////////////////////

/// This class defines Gmsh mesh format writer
/// @author Willem Deconinck
class Gmsh_API CWriter : public MeshWriter
{
public: // typedefs

    typedef boost::shared_ptr<CWriter> Ptr;
    typedef boost::shared_ptr<CWriter const> ConstPtr;

public: // functions

  /// constructor
  CWriter( const std::string& name );

  /// Gets the Class name
  static std::string type_name() { return "CWriter"; }

  virtual void write_from_to(const Mesh& mesh, const common::URI& file);

  virtual std::string get_format() { return "Gmsh"; }

  virtual std::vector<std::string> get_extensions();

private: // functions

  void write_header(std::fstream& file);

  void write_coordinates(std::fstream& file);

  void write_connectivity(std::fstream& file);

//  void write_nodal_data(std::fstream& file);

  void write_elem_nodal_data(std::fstream& file);

//  void write_element_data(std::fstream& file);

private: // data

  std::map<std::string,Uint> m_groupnumber;

  std::map<std::string,Uint> m_elementTypes;

  std::map<CEntities const*,Uint> m_element_start_idx;

  boost::shared_ptr< common::Map<Uint,Uint> > m_cf_2_gmsh_node;
}; // end CWriter


////////////////////////////////////////////////////////////////////////////////

} // Gmsh
} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_Gmsh_CWriter_hpp
