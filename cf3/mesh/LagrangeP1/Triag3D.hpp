// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_LagrangeP1_Triag3D_hpp
#define cf3_mesh_LagrangeP1_Triag3D_hpp

#include "mesh/ElementTypeBase.hpp"
#include "mesh/LagrangeP1/Triag.hpp"

namespace cf3 {
namespace mesh {
namespace LagrangeP1 {

////////////////////////////////////////////////////////////////////////////////

struct Mesh_LagrangeP1_API Triag3D_traits
{
  typedef Triag SF;

  enum { dimension      = 3 };
  enum { nb_faces       = 1 };
  enum { nb_edges       = 3 };
};

/// @brief 3D Lagrange P1 Triangular Element type
/// This class provides the lagrangian shape function describing the
/// representation of the solution and/or the geometry in a P1 (linear)
/// triangular element.
/// @see ElementType for documentation of undocumented functions
/// @author Willem Deconinck
/// @author Tiago Quintino
/// @author Bart Janssens
struct Mesh_LagrangeP1_API Triag3D : public ElementTypeBase<Triag3D,Triag3D_traits>
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

  static JacobianT jacobian(const MappedCoordsT& mapped_coord, const NodesT& nodes);
  template < typename MatrixType >
  static void compute_jacobian(const MappedCoordsT& mapped_coord, const NodesT& nodes, MatrixType& jacobian);
  static Real volume(const NodesT& nodes);
  static Real area(const NodesT& nodes);
  static void compute_centroid(const NodesT& nodes , CoordsT& centroid);
  static void compute_normal(const NodesT& nodes , CoordsT& normal);

  //@}

  /// @name Non-API static functions
  //  ------------------------------
  //@{

  /// Normal vector to the surface.
  /// @param mappedCoord The mapped coordinates where the Jacobian should be calculated
  /// @param result Storage for the resulting Jacobian matrix
  /// @note this is not the unit normal, but scaled by 1/4*area
  static void normal(const MappedCoordsT& mapped_coord, const NodesT& nodes, CoordsT& result);

  //@}
};

////////////////////////////////////////////////////////////////////////////////

} // LagrangeP1
} // mesh
} // cf3

#endif // cf3_mesh_LagrangeP1_Triag3D_hpp
