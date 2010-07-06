#ifndef CF_Mesh_OpenFOAM_BlockMeshReader_hpp
#define CF_Mesh_OpenFOAM_BlockMeshReader_hpp

#include "Math/RealVector.hpp"
#include "Mesh/CMeshReader.hpp"

namespace CF {
namespace Mesh {
namespace OpenFOAM {

//////////////////////////////////////////////////////////////////////////////

/// This class defines OpenFOAM BlockMesh mesh format reader
/// @author Bart Janssens
class BlockMeshReader : public CMeshReader
{
public:

  /// constructor
  BlockMeshReader();

  /// Gets the Class name
  static std::string type_name() { return "BlockMeshReader"; }

private:

  virtual void read_impl(std::fstream& file);
}; // end BlockMeshReader


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


  Real scalingFactor;

  /// The coordinates for all the nodes
  std::vector<PointT> points;

  /// Points for each block, in terms of node indices
  std::vector<IndicesT> blockPoints;
  /// Subdivisions for each block, along X, Y and Z
  std::vector<CountsT> blockSubdivisions;
  /// edgeGrading for each block
  std::vector<GradingT> blockGradings;

  /// Type of each patch
  std::vector<std::string> patchTypes;
  /// Name for each patch
  std::vector<std::string> patchNames;
  /// Point indices for each patch (grouped per 4)
  std::vector<IndicesT> patchPoints;
};

/// Populate structured grid info from a BlockMeshDict file
void readBlockMeshFile(std::fstream& file, BlockData& blockData);

/// Determine the number of dimensions (TODO, always returns 3 now)
/// Note: All OpenFOAM meshes are 3D, but 2D meshes are only 1 cell thick.
/// Based on this, the real mesh dimensionality can be determined
Uint computeDimensionality(const BlockData& blockData) { return 3; }

void buildMesh(const BlockData& blockData, CMesh& mesh);

////////////////////////////////////////////////////////////////////////////////

} // namespace OpenFOAM
} // namespace Mesh
} // namespace CF

#endif /* CF_Mesh_OpenFOAM_BlockMeshReader_hpp */
