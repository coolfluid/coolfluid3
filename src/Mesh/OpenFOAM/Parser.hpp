#ifndef CF_Mesh_OpenFOAM_Parser_hpp
#define CF_Mesh_OpenFOAM_Parser_hpp

#include <fstream>

namespace CF {
namespace Mesh {
namespace OpenFOAM {

class BlockData;
  
////////////////////////////////////////////////////////////////////////////////

/// Parse an OpenFoam blockMeshDict file to generate block data 
void parse_blockmesh_dict(std::fstream& file, BlockData& blockData);

////////////////////////////////////////////////////////////////////////////////

} // namespace OpenFOAM
} // namespace Mesh
} // namespace CF

#endif /* CF_Mesh_OpenFOAM_Parser_hpp */
