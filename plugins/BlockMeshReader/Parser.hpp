// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_BlockMeshReader_Parser_hpp
#define cf3_BlockMeshReader_Parser_hpp

#include <fstream>

#include "LibBlockMeshReader.hpp"

namespace cf3 {
  
  namespace Mesh { namespace BlockMesh { class BlockData; } }
  
namespace BlockMeshReader {

////////////////////////////////////////////////////////////////////////////////

/// Parse an OpenFoam blockMeshDict file to generate block data
void BlockMeshReader_API parse_blockmesh_dict(std::istream& file, Mesh::BlockMesh::BlockData& blockData);

////////////////////////////////////////////////////////////////////////////////

} // BlockMeshReader
} // cf3

#endif /* CF3_BlockMeshReader_Parser_hpp */
