// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_BlockMesh_BlockData_hpp
#define CF_Mesh_BlockMesh_BlockData_hpp

#include "Common/CF.hpp"

#include "Mesh/LibMesh.hpp"
#include "Mesh/BlockMesh/LibBlockMesh.hpp"

namespace CF {
namespace Mesh {

class CMesh;

namespace BlockMesh {

  
////////////////////////////////////////////////////////////////////////////////

/// Storage for the information about blocks for structured grid generation
struct BlockMesh_API BlockData
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
  Uint dimension;

  /// The coordinates for all the nodes
  std::vector<PointT> points;

  /// Points for each block, in terms of node indices
  std::vector<IndicesT> block_points;
  /// Subdivisions for each block, along X, Y and Z
  std::vector<CountsT> block_subdivisions;
  /// edgeGrading for each block
  std::vector<GradingT> block_gradings;
  /// Distribution of blocks among processors
  IndicesT block_distribution;

  /// Type of each patch
  std::vector<std::string> patch_types;
  /// Name for each patch
  std::vector<std::string> patch_names;
  /// Point indices for each patch (grouped per 4)
  std::vector<IndicesT> patch_points;
  
  /// Checks for equality
  bool operator== (const BlockData& other) const;
};

/// Using the given block data, construct the mesh. Global node indices are generated as well, so there is no need for a separate global ID generation
/// @param block_data Description of the structured blocks that make up the grid
/// @param mesh Stores the generated mesh
void BlockMesh_API build_mesh(const CF::Mesh::BlockMesh::BlockData& block_data, CF::Mesh::CMesh& mesh);

/// Partition a mesh along the X, Y or Z axis into the given number of partitions
/// Partitioning ensures that processor boundaries lie on a boundary between blocks
/// @param blocks_in Unpartitioned block data
/// @param block_mesh Storage for a mesh containing only the (serial) blocks
/// @param nb_partitions Number of partitions to create
/// @param direction Axis along which to partition
/// @param blocks_out Will store the paritioned block data, ready for parallel mesh generation
void BlockMesh_API partition_blocks(const BlockData& blocks_in, CMesh& block_mesh, const Uint nb_partitions, const CoordXYZ direction, BlockData& blocks_out);

/// Creates a mesh containing only the blocks as elements
void BlockMesh_API create_block_mesh(const BlockData& block_data, CMesh& mesh);

////////////////////////////////////////////////////////////////////////////////

} // BlockMesh
} // Mesh
} // CF

#endif /* CF_Mesh_BlockMesh_BlockData_hpp */
