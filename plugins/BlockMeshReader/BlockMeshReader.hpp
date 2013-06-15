// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_BlockMeshReader_BlockMeshReader_hpp
#define cf3_BlockMeshReader_BlockMeshReader_hpp

#include "mesh/MeshReader.hpp"

namespace cf3 {
namespace BlockMeshReader {

//////////////////////////////////////////////////////////////////////////////

/// This class defines BlockMeshReader BlockMesh mesh format reader
/// @author Bart Janssens
class BlockMeshReader : public cf3::mesh::MeshReader
{
public:

  /// constructor
  BlockMeshReader(const std::string& name);

  /// Gets the Class name
  static std::string type_name() { return "BlockMeshReader"; }

  virtual std::string get_format() { return "blockMeshDict"; }

  virtual std::vector<std::string> get_extensions();

private:
  virtual void do_read_mesh_into(const common::URI& path, mesh::Mesh& mesh);

}; // end BlockMeshReader


////////////////////////////////////////////////////////////////////////////////

} // BlockMeshReader
} // cf3

#endif /* CF3_BlockMeshReader_BlockMeshReader_hpp */
