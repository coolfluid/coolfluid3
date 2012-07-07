// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_SimpleMeshGenerator_hpp
#define cf3_mesh_SimpleMeshGenerator_hpp

////////////////////////////////////////////////////////////////////////////////

#include "MeshGenerator.hpp"

namespace cf3 {
namespace mesh {

////////////////////////////////////////////////////////////////////////////////

/// @brief Generate a simple carthesian P1 mesh without grading
///
/// Possible meshes to generate are 1D lines and 2D rectangles.
/// Required is the number of cells in every direction, the length and an offset.
/// The topology created for lines:
///    topology/interior
///    topology/xneg  (bdry)
///    topology/xpos  (bdry)
/// The topology created for rectangles:
///    topology/interior
///    topology/left    (bdry)
///    topology/right   (bdry)
///    topology/bottom  (bdry)
///    topology/top     (bdry)
/// The topology created for boxes:
///    topology/interior
///    topology/left    (bdry)
///    topology/right   (bdry)
///    topology/bottom  (bdry)
///    topology/top     (bdry)
///    topology/back     (bdry)
///    topology/front     (bdry)
/// @author Willem Deconinck
class Mesh_API SimpleMeshGenerator : public MeshGenerator {

public: // functions

  /// Contructor
  /// @param name of the component
  SimpleMeshGenerator ( const std::string& name );

  /// Virtual destructor
  virtual ~SimpleMeshGenerator();

  /// Get the class name
  static std::string type_name () { return "SimpleMeshGenerator"; }

  virtual void execute();

private: // functions

  /// Create a line
  void create_line();

  /// Create a rectangle
  void create_rectangle();

  /// Create a box
  void create_box();

  /// Helper function to get a linear index given indices i,j,k
  Uint node_idx(const Uint i, const Uint j, const Uint k, const Uint Nx, const Uint Ny, const Uint Nz);

  /// Helper function to get a linear index given indices i,j,k
  Uint elem_idx(const Uint i, const Uint j, const Uint k, const Uint Nx, const Uint Ny, const Uint Nz);

protected: // data

  std::vector<Uint> m_nb_cells;  ///< Number of cells in every direction
  std::vector<Real> m_lengths;   ///< Lengths in every direction
  std::vector<Real> m_offsets;   ///< Offset in every direction (coordinate of bottom-left point)

  Uint m_coord_dim;
};

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_SimpleMeshGenerator_hpp
