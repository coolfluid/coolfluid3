// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_SF_Quad3DLagrangeP1_hpp
#define CF_Mesh_SF_Quad3DLagrangeP1_hpp

#include "Math/RealMatrix.hpp"
#include "Mesh/Quad3D.hpp"

#include "Mesh/SF/Quad2DLagrangeP1.hpp"

namespace CF {
namespace Mesh {
namespace SF {

/// This class provides the lagrangian shape function describing the
/// representation of the solution and/or the geometry in a P1 (bilinear)
/// quadrilateral element that lives in 3D space
/// @author Andrea Lani
/// @author Geoffrey Deliege
/// @author Tiago Quintino
/// @author Bart Janssens
struct Quad3DLagrangeP1  : public Quad3D
{
  Quad3DLagrangeP1();

  /// Compute the shape functions corresponding to the given
  /// mapped coordinates
  /// @param mappedCoord The mapped coordinates
  /// @param shapeFunc Vector storing the result
  static void shape_function(const RealVector& mapped_coord, RealVector& shape_func)
  {
    Quad2DLagrangeP1::shape_function(mapped_coord, shape_func);
  }

  /// Compute the gradient with respect to mapped coordinates, i.e. parial derivatives are in terms of the
  /// mapped coordinates.
  /// @param mappedCoord The mapped coordinates where the gradient should be calculated
  /// @param result Storage for the resulting gradient matrix
  static void mapped_gradient(const RealVector& mapped_coord, RealMatrix& result)
  {
    Quad2DLagrangeP1::mapped_gradient(mapped_coord, result);
  }

  /// Compute the Jacobian matrix
  /// In the case of the Quad3D element, this is the vector corresponding to the line segment
  /// @param mappedCoord The mapped coordinates where the Jacobian should be calculated
  /// @param result Storage for the resulting Jacobian matrix
  template<typename NodesT>
  static void jacobian(const RealVector& mappedCoord, const NodesT& nodes, RealMatrix& result)
  {
    cf_assert(result.nbRows() == dimensionality);
    cf_assert(result.nbCols() == dimension);

    JacobianCoefficients jc(nodes);

    const Real xi = mappedCoord[KSI];
    const Real eta = mappedCoord[ETA];

    result(KSI,XX) = jc.bx + jc.dx*eta;
    result(KSI,YY) = jc.by + jc.dy*eta;
    result(KSI,ZZ) = jc.bz + jc.dz*eta;

    result(ETA,XX) = jc.cx + jc.dx*xi;
    result(ETA,YY) = jc.cy + jc.dy*xi;
    result(ETA,ZZ) = jc.cz + jc.dz*xi;
  }

  /// Normal vector to the surface.
  /// @param mappedCoord The mapped coordinates where the Jacobian should be calculated
  /// @param result Storage for the resulting Jacobian matrix
  template<typename NodesT>
  static void normal(const RealVector& mappedCoord, const NodesT& nodes, RealVector& result)
  {
    cf_assert(result.size() == dimension);
    RealMatrix jac(dimensionality, dimension);
    jacobian(mappedCoord, nodes, jac);

    result[XX] = jac(KSI,YY)*jac(ETA,ZZ) - jac(KSI,ZZ)*jac(ETA,YY);
    result[YY] = jac(KSI,ZZ)*jac(ETA,XX) - jac(KSI,XX)*jac(ETA,ZZ);
    result[ZZ] = jac(KSI,XX)*jac(ETA,YY) - jac(KSI,YY)*jac(ETA,XX);
  }

  /// Volume of the cell. 0 in case of elements with a dimensionality that is less than
  /// the dimension of the problem
  template<typename NodesT>
  static Real volume(const NodesT& nodes)
  {
    return 0.;
  }

  /// The area of an element that represents a surface in the solution space, i.e.
  /// 1D elements in 2D space or 2D elements in 3D space
  template<typename NodesT>
  static Real area(const NodesT& nodes)
  {
    RealVector n(dimension);
    RealVector center(0., 2);
    normal(center, nodes, n);
    return 4.*n.norm2();
  }

  /// Number of nodes
  static const Uint nb_nodes = 4;

  /// Order of the shape function
  static const Uint order = 1;

  /// Given nodal values, write the interpolation
  template<typename NodalValuesT, typename ValueT>
  void operator()(const RealVector& mapped_coord, const NodalValuesT& nodal_values, ValueT& interpolation) const
  {
    cf_assert(mapped_coord.size() == dimensionality);
    cf_assert(nodal_values.size() == nb_nodes);

    RealVector sf(nb_nodes);
    shape_function(mapped_coord, sf);

    interpolation = sf[0]*nodal_values[0] + sf[1]*nodal_values[1] + sf[2]*nodal_values[2] + sf[3]*nodal_values[3];
  }

  virtual std::string getElementTypeName() const;

  /// The volume of an element with a dimensionality that is less than
  /// the dimension of the problem is 0.
  virtual Real computeVolume(const NodesT& coord) const;
  virtual const FaceConnectivity& face_connectivity() const;
  virtual const CF::Mesh::ElementType& face_type(const CF::Uint face) const;

private:
  /// Convenience struct to easily access the elements that make up the jacobian
  struct JacobianCoefficients
  {
    Real ax, bx, cx, dx;
    Real ay, by, cy, dy;
    Real az, bz, cz, dz;
    template<typename NodesT>
    JacobianCoefficients(const NodesT& nodes)
    {
      const Real x0 = nodes[0][XX];
      const Real y0 = nodes[0][YY];
      const Real z0 = nodes[0][ZZ];

      const Real x1 = nodes[1][XX];
      const Real y1 = nodes[1][YY];
      const Real z1 = nodes[1][ZZ];

      const Real x2 = nodes[2][XX];
      const Real y2 = nodes[2][YY];
      const Real z2 = nodes[2][ZZ];

      const Real x3 = nodes[3][XX];
      const Real y3 = nodes[3][YY];
      const Real z3 = nodes[3][ZZ];

      ax = 0.25*( x0 + x1 + x2 + x3);
      bx = 0.25*(-x0 + x1 + x2 - x3);
      cx = 0.25*(-x0 - x1 + x2 + x3);
      dx = 0.25*( x0 - x1 + x2 - x3);

      ay = 0.25*( y0 + y1 + y2 + y3);
      by = 0.25*(-y0 + y1 + y2 - y3);
      cy = 0.25*(-y0 - y1 + y2 + y3);
      dy = 0.25*( y0 - y1 + y2 - y3);

      az = 0.25*( z0 + z1 + z2 + z3);
      bz = 0.25*(-z0 + z1 + z2 - z3);
      cz = 0.25*(-z0 - z1 + z2 + z3);
      dz = 0.25*( z0 - z1 + z2 - z3);
    }
  };

};

} // namespace SF
} // namespace Mesh
} // namespace CF

#endif /* CF_Mesh_SF_Quad3DLagrangeP1 */
