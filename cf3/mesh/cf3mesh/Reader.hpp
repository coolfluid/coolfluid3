// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_CF3Mesh_Reader_hpp
#define cf3_mesh_CF3Mesh_Reader_hpp

////////////////////////////////////////////////////////////////////////////////

#include "mesh/MeshReader.hpp"
#include "mesh/GeoShape.hpp"

#include "mesh/cf3mesh/LibCF3Mesh.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {
  class BinaryDataReader;
}
}

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
namespace cf3mesh {

//////////////////////////////////////////////////////////////////////////////

/// This class defines CF3Mesh mesh format writer
/// @author Bart Janssens
class CF3Mesh_API Reader : public MeshReader
{
public: // functions

  /// constructor
  Reader( const std::string& name );

  /// Gets the Class name
  static std::string type_name() { return "Reader"; }

  virtual std::string get_format() { return "CF3Mesh"; }

  virtual std::vector<std::string> get_extensions();
private:
  virtual void do_read_mesh_into(const common::URI& path, Mesh& mesh);

  void read_mesh_part(const common::XML::XmlNode &topology_node, const common::XML::XmlNode &dictionaries_node, Mesh& mesh, const Uint rank);

  void read_topology(const common::XML::XmlNode& region_node, Region& region, Dictionary& geometry);

  void read_elements(const common::XML::XmlNode& region_node, Region& region, Dictionary& geometry, Mesh &mesh);

private:
  boost::shared_ptr<common::BinaryDataReader> data_reader;
  std::vector< Handle<Entities> > m_entities;
}; // end Reader


////////////////////////////////////////////////////////////////////////////////

} // cf3mesh
} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_CF3Mesh_Reader_hpp
