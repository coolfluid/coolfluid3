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
  ~BlockArrays();

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

  /// Partition the blocks along one direction. If a previous partitioning exists, each partition is repartitioned into
  /// nb_partitions blocks
  /// @param nb_partitions Number of partitions to create
  /// @param direction Direction to partition in (X = 0, Y = 1, Z = 2)
  void partition_blocks(const Uint nb_partitions, const Uint direction);

  /// Extrude a 2D mesh in a number of spanwise (Z-direction) blocks. The number of spanwise blocks is determined by
  /// the size of the passed arguments
  /// @param positions Spanwise coordinate for each new spanwise layer of points. Values must ne greater than 0
  /// @param nb_segments Number of spanwise segments for each block
  /// @param gradings Uniform grading definition in the spanwise direction for each block
  void extrude_blocks(const std::vector<Real>& positions, const std::vector<Uint>& nb_segments, const std::vector<Real>& gradings);

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
  void signature_partition_blocks(common::SignalArgs& args);
  void signal_partition_blocks(common::SignalArgs& args);
  void signature_create_mesh(common::SignalArgs& args);
  void signature_extrude_blocks(common::SignalArgs& args);
  void signal_extrude_blocks(common::SignalArgs& args);
  void signal_create_mesh(common::SignalArgs& args);

  //@} END SIGNALS

private:
  class Implementation;
  boost::scoped_ptr<Implementation> m_implementation;
};

////////////////////////////////////////////////////////////////////////////////

} // BlockMesh
} // mesh
} // cf3

#endif /* CF3_Mesh_BlockMesh_BlockData_hpp */
