#ifndef CF_Mesh_OpenFOAM_BlockData_hpp
#define CF_Mesh_OpenFOAM_BlockData_hpp

#include "Common/CF.hpp"

namespace CF {
namespace Mesh {

class CMesh;
  
namespace OpenFOAM {

////////////////////////////////////////////////////////////////////////////////

/// Storage for the information about blocks for structured grid generation
struct BlockData
{
  /// Type to store indices into another vector
  typedef std::vector<Uint> IndicesT;
  /// Data type for counts of data, i.e. number of points
  typedef std::vector<Uint> CountsT;
  /// Storage for a single point coordinate (STL vector for ease of use with boost::spirit)
  typedef std::vector<Real> PointT;
  /// Storage for a grading corresponding to a single block
  typedef std::vector<Real> GradingT;
  /// Storage for true/false flags
  typedef std::vector<bool> BooleansT;


  Real scaling_factor;

  /// The coordinates for all the nodes
  std::vector<PointT> points;

  /// Points for each block, in terms of node indices
  std::vector<IndicesT> block_points;
  /// Subdivisions for each block, along X, Y and Z
  std::vector<CountsT> block_subdivisions;
  /// edgeGrading for each block
  std::vector<GradingT> block_gradings;

  /// Type of each patch
  std::vector<std::string> patch_types;
  /// Name for each patch
  std::vector<std::string> patch_names;
  /// Point indices for each patch (grouped per 4)
  std::vector<IndicesT> patch_points;
};

/// Using the given block data, construct the mesh
void build_mesh(const BlockData& block_data, CF::Mesh::CMesh& mesh);

////////////////////////////////////////////////////////////////////////////////

} // namespace OpenFOAM
} // namespace Mesh
} // namespace CF

#endif /* CF_Mesh_OpenFOAM_BlockData_hpp */
