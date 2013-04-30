// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_LagrangeP1_Line1D_hpp
#define cf3_mesh_LagrangeP1_Line1D_hpp

#include "mesh/ElementTypeBase.hpp"
#include "mesh/LagrangeP1/Line.hpp"

namespace cf3 {
namespace mesh {
namespace LagrangeP1 {

////////////////////////////////////////////////////////////////////////////////

struct Mesh_LagrangeP1_API Line1D_traits
{
  typedef Line SF;

  enum { dimension      = 1 };
  enum { nb_faces       = 2 };
  enum { nb_edges       = 2 };
};

/// @brief Lagrange P1 Triangular Element type
/// This class provides the lagrangian shape function describing the
/// representation of the solution and/or the geometry in a P1 (linear)
/// triangular element.
/// @see ElementType for documentation of undocumented functions
/// @author Willem Deconinck
/// @author Tiago Quintino
/// @author Bart Janssens
struct Mesh_LagrangeP1_API Line1D : public ElementTypeBase<Line1D,Line1D_traits>
{
  /// @name Accessor functions
  //  ------------------------
  //@{

  static const ElementTypeFaceConnectivity& faces();
  static const ElementType& face_type(const Uint face);

  //@}

  /// @name Computation functions
  //  ---------------------------
  //@{

  static MappedCoordsT mapped_coordinate(const CoordsT& coord, const NodesT& nodes);
  static void compute_mapped_coordinate(const CoordsT& coord, const NodesT& nodes, MappedCoordsT& mapped_coord);
  static Real jacobian_determinant(const MappedCoordsT& mapped_coord, const NodesT& nodes);
  static JacobianT jacobian(const MappedCoordsT& mapped_coord, const NodesT& nodes);
  template < typename MatrixType >
  static void compute_jacobian(const MappedCoordsT& mapped_coord, const NodesT& nodes, MatrixType& jacobian);
  static void compute_jacobian_adjoint(const MappedCoordsT& mapped_coord, const NodesT& nodes, JacobianT& result);
  static Real volume(const NodesT& nodes);
  static Real area(const NodesT& nodes);
  static void compute_centroid(const NodesT& nodes , CoordsT& centroid);
  static bool is_coord_in_element(const CoordsT& coord, const NodesT& nodes);
  static CoordsT plane_jacobian_normal(const MappedCoordsT& mapped_coord, const NodesT& nodes, const CoordRef orientation);
  static void compute_plane_jacobian_normal(const MappedCoordsT& mapped_coord, const NodesT& nodes, const CoordRef orientation, CoordsT& result);

  //@}

};

////////////////////////////////////////////////////////////////////////////////

} // LagrangeP1
} // mesh
} // cf3

#endif // cf3_mesh_LagrangeP1_Line1D_hpp
