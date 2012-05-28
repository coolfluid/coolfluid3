// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_LagrangeP1_Quad2D_hpp
#define cf3_mesh_LagrangeP1_Quad2D_hpp

#include "mesh/ElementTypeBase.hpp"
#include "mesh/LagrangeP1/Quad.hpp"

namespace cf3 {
namespace mesh {
namespace LagrangeP1 {

////////////////////////////////////////////////////////////////////////////////

struct Mesh_LagrangeP1_API Quad2D_traits
{
  typedef Quad SF;

  enum { dimension      = 2 };
  enum { nb_faces       = 4 };
  enum { nb_edges       = 4 };
};


/// @brief 2D Lagrange P1 Quadrilateral Element type
/// This class provides the lagrangian shape function describing the
/// representation of the solution and/or the geometry in a P1 (linear)
/// quadrilateral element.
/// @see ElementType for documentation of undocumented functions
/// @author Willem Deconinck
/// @author Tiago Quintino
/// @author Bart Janssens
struct Mesh_LagrangeP1_API Quad2D : public ElementTypeBase<Quad2D,Quad2D_traits>
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
  /// Convenience struct to easily access the elements that make up the jacobian
  struct JacobianCoefficients
  {
    Real ax, bx, cx, dx;
    Real ay, by, cy, dy;
    template<typename NodesT>
    JacobianCoefficients(const NodesT& nodes)
    {
      const Real x0 = nodes(0, XX);
      const Real y0 = nodes(0, YY);
      const Real x1 = nodes(1, XX);
      const Real y1 = nodes(1, YY);
      const Real x2 = nodes(2, XX);
      const Real y2 = nodes(2, YY);
      const Real x3 = nodes(3, XX);
      const Real y3 = nodes(3, YY);

      ax = 0.25*( x0 + x1 + x2 + x3);
      bx = 0.25*(-x0 + x1 + x2 - x3);
      cx = 0.25*(-x0 - x1 + x2 + x3);
      dx = 0.25*( x0 - x1 + x2 - x3);
      ay = 0.25*( y0 + y1 + y2 + y3);
      by = 0.25*(-y0 + y1 + y2 - y3);
      cy = 0.25*(-y0 - y1 + y2 + y3);
      dy = 0.25*( y0 - y1 + y2 - y3);
    }
  };

  static RealVector2 m_D;
  static Real m_scale;

};

////////////////////////////////////////////////////////////////////////////////

} // LagrangeP1
} // mesh
} // cf3

#endif // cf3_mesh_LagrangeP1_Quad2D_hpp
