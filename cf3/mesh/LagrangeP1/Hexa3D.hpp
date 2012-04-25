// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_LagrangeP1_Hexa3D_hpp
#define cf3_mesh_LagrangeP1_Hexa3D_hpp

#include "mesh/ElementTypeBase.hpp"
#include "mesh/LagrangeP1/Hexa.hpp"

namespace cf3 {
namespace mesh {
namespace LagrangeP1 {

struct Mesh_LagrangeP1_API Hexa3D_traits
{
  typedef Hexa SF;

  enum { dimension      = 3  };
  enum { nb_faces       = 6  };
  enum { nb_edges       = 12 };
};

////////////////////////////////////////////////////////////////////////////////

/// @brief 2D Lagrange P1 Triangular Element type
/// This class provides the lagrangian shape function describing the
/// representation of the solution and/or the geometry in a P1 (linear)
/// triangular element.
/// @see ElementType for documentation of undocumented functions
/// @author Willem Deconinck
/// @author Tiago Quintino
/// @author Bart Janssens
struct Mesh_LagrangeP1_API Hexa3D : public ElementTypeBase<Hexa3D,Hexa3D_traits>
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
  static void compute_jacobian(const MappedCoordsT& mapped_coord, const NodesT& nodes, JacobianT& jacobian);
  static void compute_jacobian_adjoint(const MappedCoordsT& mapped_coord, const NodesT& nodes, JacobianT& result);
  static Real volume(const NodesT& nodes);
  static Real area(const NodesT& nodes);
  static void compute_centroid(const NodesT& nodes , CoordsT& centroid);
  static bool is_coord_in_element(const CoordsT& coord, const NodesT& nodes);
  static CoordsT plane_jacobian_normal(const MappedCoordsT& mapped_coord, const NodesT& nodes, const CoordRef orientation);
  static void compute_plane_jacobian_normal(const MappedCoordsT& mapped_coord, const NodesT& nodes, const CoordRef orientation, CoordsT& result);

  //@}

private:

  static bool is_orientation_inside(const CoordsT& coord, const NodesT& nodes, const Uint face);

  static Eigen::Matrix<Real,nb_nodes,1> m_shapeFunc;
  static Eigen::Matrix<Real,nb_nodes,dimensionality> m_shapeFuncDerivs;
  static Eigen::Matrix<Real,dimension,1> m_vec1;
  static Eigen::Matrix<Real,dimension,1> m_vec2;

};

////////////////////////////////////////////////////////////////////////////////

} // LagrangeP1
} // mesh
} // cf3

#endif // cf3_mesh_LagrangeP1_Hexa3D_hpp
