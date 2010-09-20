// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_OpenFOAM_WriteDict_hpp
#define CF_Mesh_OpenFOAM_WriteDict_hpp

#include "Common/CF.hpp"
#include "Mesh/OpenFOAM/BlockData.hpp"
#include "Mesh/OpenFOAM/OpenFOAMAPI.hpp"

namespace CF {
namespace Mesh {  
namespace OpenFOAM {

OpenFOAM_API std::ostream& operator<<(std::ostream& os, const BlockData::IndicesT& data);

OpenFOAM_API std::ostream& operator<<(std::ostream& os, const BlockData::PointT& data);

OpenFOAM_API std::ostream& operator<<(std::ostream& os, const BlockData& block_data);

////////////////////////////////////////////////////////////////////////////////

} // namespace OpenFOAM
} // namespace Mesh
} // namespace CF

#endif /* CF_Mesh_OpenFOAM_WriteDict_hpp */
