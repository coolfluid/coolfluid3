// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_BlockMesh_WriteDict_hpp
#define CF_Mesh_BlockMesh_WriteDict_hpp

#include "Common/CF.hpp"
#include "Mesh/BlockMesh/BlockData.hpp"
#include "Mesh/BlockMesh/LibBlockMesh.hpp"

namespace CF {
namespace Mesh {  
namespace BlockMesh {

BlockMesh_API std::ostream& operator<<(std::ostream& os, const BlockData::IndicesT& data);

BlockMesh_API std::ostream& operator<<(std::ostream& os, const BlockData::PointT& data);

BlockMesh_API std::ostream& operator<<(std::ostream& os, const BlockData& block_data);

////////////////////////////////////////////////////////////////////////////////

} // BlockMesh
} // Mesh
} // CF

#endif /* CF_Mesh_BlockMesh_WriteDict_hpp */
