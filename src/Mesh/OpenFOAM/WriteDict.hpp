#ifndef CF_Mesh_OpenFOAM_WriteDict_hpp
#define CF_Mesh_OpenFOAM_WriteDict_hpp

#include "Common/CF.hpp"
#include "Mesh/OpenFOAM/BlockData.hpp"

namespace CF {
namespace Mesh {  
namespace OpenFOAM {

std::ostream& operator<<(std::ostream& os, const BlockData::IndicesT& data);

std::ostream& operator<<(std::ostream& os, const BlockData::PointT& data);

std::ostream& operator<<(std::ostream& os, const BlockData& block_data);

////////////////////////////////////////////////////////////////////////////////

} // namespace OpenFOAM
} // namespace Mesh
} // namespace CF

#endif /* CF_Mesh_OpenFOAM_WriteDict_hpp */
