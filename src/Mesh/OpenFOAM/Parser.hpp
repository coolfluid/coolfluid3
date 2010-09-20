// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_OpenFOAM_Parser_hpp
#define CF_Mesh_OpenFOAM_Parser_hpp

#include <fstream>

#include "Mesh/OpenFOAM/OpenFOAMAPI.hpp"

namespace CF {
namespace Mesh {
namespace OpenFOAM {

class BlockData;
  
////////////////////////////////////////////////////////////////////////////////

/// Parse an OpenFoam blockMeshDict file to generate block data 
void OpenFOAM_API parse_blockmesh_dict(std::istream& file, BlockData& blockData);

////////////////////////////////////////////////////////////////////////////////

} // namespace OpenFOAM
} // namespace Mesh
} // namespace CF

#endif /* CF_Mesh_OpenFOAM_Parser_hpp */
