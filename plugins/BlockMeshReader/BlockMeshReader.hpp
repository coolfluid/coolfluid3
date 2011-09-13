// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_BlockMeshReader_BlockMeshReader_hpp
#define CF_BlockMeshReader_BlockMeshReader_hpp

#include "Mesh/CMeshReader.hpp"

namespace CF {
namespace BlockMeshReader {

//////////////////////////////////////////////////////////////////////////////

/// This class defines BlockMeshReader BlockMesh mesh format reader
/// @author Bart Janssens
class BlockMeshReader : public CF::Mesh::CMeshReader
{
public:

  /// constructor
  BlockMeshReader(const std::string& name);

  /// Gets the Class name
  static std::string type_name() { return "BlockMeshReader"; }

  virtual std::string get_format() { return "blockMeshDict"; }

  virtual std::vector<std::string> get_extensions();

private:
  virtual void do_read_mesh_into(const Common::URI& path, Mesh::CMesh& mesh);

}; // end BlockMeshReader


////////////////////////////////////////////////////////////////////////////////

} // BlockMeshReader
} // CF

#endif /* CF_BlockMeshReader_BlockMeshReader_hpp */
