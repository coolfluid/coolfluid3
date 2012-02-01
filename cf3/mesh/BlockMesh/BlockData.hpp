// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_BlockMesh_BlockData_hpp
#define cf3_mesh_BlockMesh_BlockData_hpp

#include <boost/scoped_ptr.hpp>

#include "common/CF.hpp"
#include "common/Component.hpp"

#include "mesh/LibMesh.hpp"
#include "mesh/BlockMesh/LibBlockMesh.hpp"

namespace cf3 {
  namespace common
  {
    template<typename T> class Table;
    class Group;
}
namespace mesh {

class Mesh;
class Connectivity;

namespace BlockMesh {


////////////////////////////////////////////////////////////////////////////////

class BlockArrays : public common::Component
{
public:

  BlockArrays(const std::string& name);

  static std::string type_name () { return "BlockArrays"; }

  /// Create the table that holds the points for the blocks
  Handle< common::Table<Real> > create_points(const Uint dimensions, const Uint nb_points);

  /// Create the table that holds the blocks
  Handle< common::Table<Uint> > create_blocks(const Uint nb_blocks);

  /// Create the per-direction number of segments table.
  /// @pre create_points and create_blocks have been called
  Handle< common::Table<Uint> > create_block_subdivisions();

  /// Create the gradings
  /// @pre create_points and create_blocks have been called
  Handle< common::Table<Real> > create_block_gradings();

  /// Add a zero-filled patch, taking the number of faces in the patch as argument
  /// @param nb_faces The number of faces (i.e. block sides) in the patch
  Handle< common::Table<Uint> > create_patch(const std::string& name, const Uint nb_faces);

  /// Add a patch, initialized to contain the faces referred to by face_indices
  /// @param face_indices The indices of the faces, as they appear in default boundary region when no patches have been defined
  Handle< common::Table<Uint> > create_patch(const std::string& name, const std::vector<Uint>& face_indices);

  /// Create the volume block mesh
  Handle<Mesh> create_block_mesh();

  /// Create the refined mesh
  /// @param mesh The mesh in which the output will be stored
  void create_mesh(Mesh& mesh);

  /// @name SIGNALS
  //@{

  void signature_create_points(common::SignalArgs& args);
  void signal_create_points(common::SignalArgs& args);
  void signature_create_blocks(common::SignalArgs& args);
  void signal_create_blocks(common::SignalArgs& args);
  void signal_create_block_subdivisions(common::SignalArgs& args);
  void signal_create_block_gradings(common::SignalArgs& args);
  void signature_create_patch_nb_faces(common::SignalArgs& args);
  void signal_create_patch_nb_faces(common::SignalArgs& args);
  void signature_create_patch_face_list(common::SignalArgs& args);
  void signal_create_patch_face_list(common::SignalArgs& args);
  void signal_create_block_mesh(common::SignalArgs& args);
  void signature_create_mesh(common::SignalArgs& args);
  void signal_create_mesh(common::SignalArgs& args);

  //@} END SIGNALS

private:
  class Implementation;
  boost::scoped_ptr<Implementation> m_implementation;
};

/// Storage for the information about blocks for structured grid generation
struct BlockMesh_API BlockData : common::Component
{
  BlockData(const std::string& name);

  static std::string type_name () { return "BlockData"; }

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

  /// Copy data to another BlockData component
  void copy_to(BlockData& other) const;
};

/// Using the given block data, construct the mesh. Global node indices are generated as well, so there is no need for a separate global ID generation
/// @param block_data Description of the structured blocks that make up the grid. A mesh containing only the blocks will be created here.
/// @param mesh Stores the generated mesh
/// @param overlap Amount of cell overlap to generate
void BlockMesh_API build_mesh(cf3::mesh::BlockMesh::BlockData& block_data, cf3::mesh::Mesh& mesh, const Uint overlap = 0);

/// Partition a mesh along the X, Y or Z axis into the given number of partitions
/// Partitioning ensures that processor boundaries lie on a boundary between blocks
/// @param blocks_in Unpartitioned block data
/// @param nb_partitions Number of partitions to create
/// @param direction Axis along which to partition
/// @param blocks_out Will store the paritioned block data, ready for parallel mesh generation
void BlockMesh_API partition_blocks(const BlockData& blocks_in, const Uint nb_partitions, const CoordXYZ direction, BlockData& blocks_out);

/// Creates a mesh containing only the blocks as elements
void BlockMesh_API create_block_mesh(const BlockData& block_data, Mesh& mesh);

////////////////////////////////////////////////////////////////////////////////

} // BlockMesh
} // mesh
} // cf3

#endif /* CF3_Mesh_BlockMesh_BlockData_hpp */
