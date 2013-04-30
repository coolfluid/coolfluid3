// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Tools_MeshGeneration_Tools_hpp
#define cf3_Tools_MeshGeneration_Tools_hpp

#include "math/Consts.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/BlockMesh/BlockData.hpp"

#include "Tools/MeshGeneration/LibMeshGeneration.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace Tools {
namespace MeshGeneration {

////////////////////////////////////////////////////////////////////////////////

/// Create a 1D line mesh
void MeshGeneration_API create_line(mesh::Mesh& mesh, const Real x_len, const Uint x_segments);

/// Create a rectangular, 2D, quad-only mesh. No buffer for creation
void MeshGeneration_API create_rectangle(mesh::Mesh& mesh, const Real x_len, const Real y_len, const Uint x_segments, const Uint y_segments);

/// Create a triangulated version of a 2D rectangular grid.
void MeshGeneration_API create_rectangle_tris(mesh::Mesh& mesh, const Real x_len, const Real y_len, const Uint x_segments, const Uint y_segments);

/// Creates a 2D circular arc
void MeshGeneration_API create_circle_2d(mesh::Mesh& mesh, const Real radius, const Uint segments, const Real start_angle = 0., const Real end_angle = 2.*math::Consts::pi());

/// Create block data for a 3D periodic channel (flow between infinite flat plates)
/// @param length: Total distance between the streamwise periodic boundaries (X-direction)
/// @param half_height: Half of the distance between the plates
/// @param width: Total distance between periodic boundaries in the spanwise (Z) direction
/// @param x_segs: Number of segments in the X direction
/// @param y_segs_half: HALF the number of segements between the flat plates
/// @param z_segs: Number of segments in the Z-direction
/// @param ratio: Ratio (smallest cell / largest cell) in the Y-direction
void MeshGeneration_API create_channel_3d(mesh::BlockMesh::BlockArrays& blocks, const Real length, const Real half_height, const Real width, const Uint x_segs, const Uint y_segs_half, const Uint z_segs, const Real ratio);

////////////////////////////////////////////////////////////////////////////////

/// Global fixture that provides unique access to meshes with a certain size
/// Useful for creating large grids outside of profiled fixtures
template<Uint MeshSize>
struct MeshSourceGlobalFixture {

  /// Init all meshes at creation time
  MeshSourceGlobalFixture()
  {
    // CFinfo << "Created mesh " << grid2().name() << CFendl;
  }

  /// Returns a 2D square grid, with side MeshSize
  static const mesh::Mesh& grid2()
  {
    static boost::shared_ptr< mesh::Mesh > grid2D;
    if(!grid2D) {
      grid2D = common::allocate_component<mesh::Mesh>("grid2D");
      create_rectangle(*grid2D, 1., 1., MeshSize, MeshSize);
    }
    return *grid2D;
  }

};


////////////////////////////////////////////////////////////////////////////////

} // MeshGeneration
} // Tools
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_Tools_MeshGeneration_Tools_hpp

