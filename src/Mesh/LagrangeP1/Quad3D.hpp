// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_LagrangeP1_Quad3D_hpp
#define CF_Mesh_LagrangeP1_Quad3D_hpp

#include "Mesh/ElementType.hpp"
#include "Mesh/LagrangeP1/Quad.hpp"

namespace CF {
namespace Mesh {
  template <typename SF> class ShapeFunctionT;
namespace LagrangeP1 {

////////////////////////////////////////////////////////////////////////////////

/// @brief 2D Lagrange P1 Quadrilateral Element type
/// This class provides the lagrangian shape function describing the
/// representation of the solution and/or the geometry in a P1 (linear)
/// quadrilateral element.
/// @see ElementType for documentation of undocumented functions
/// @author Willem Deconinck
/// @author Tiago Quintino
/// @author Bart Janssens
struct Mesh_LagrangeP1_API Quad3D
{
public: // typedefs

  typedef boost::shared_ptr<Quad3D>       Ptr;
  typedef boost::shared_ptr<Quad3D const> ConstPtr;

  /// The shape function of this element
  typedef Quad SF;

  /// @name Element definitions
  //  -------------------------
  //@{
  enum { shape          = SF::shape          };
  enum { dimensionality = SF::dimensionality };
  enum { nb_nodes       = SF::nb_nodes       };
  enum { order          = SF::order          };

  enum { dimension      = 3 };
  enum { nb_faces       = 1 };
  enum { nb_edges       = 4 };
  //@}

  /// @name Matrix Types
  //  --------------------------------
  //@{
  typedef SF::MappedCoordsT                              MappedCoordsT;
  typedef Eigen::Matrix<Real, dimension, 1>              CoordsT;
  typedef Eigen::Matrix<Real, nb_nodes, dimension>       NodesT;
  typedef Eigen::Matrix<Real, dimensionality, dimension> JacobianT;
  //@}

public: // functions

  /// @name Constructor / Destructor / Type name
  //  ------------------------------------------
  //@{

  Quad3D() {}
  ~Quad3D() {}
  static std::string type_name() { return "Quad3D"; }

  //@}

  /// @name Accessor functions
  //  ------------------------
  //@{

  static const ShapeFunctionT<SF>& shape_function();
  static const ElementType::FaceConnectivity& faces();
  static const ElementType& face_type(const Uint face);

  //@}

  /// @name Computation functions
  //  ---------------------------
  //@{

  static JacobianT jacobian(const MappedCoordsT& mapped_coord, const NodesT& nodes);
  static void compute_jacobian(const MappedCoordsT& mapped_coord, const NodesT& nodes, JacobianT& jacobian);
  static Real volume(const NodesT& nodes);
  static Real area(const NodesT& nodes);
  static void compute_centroid(const NodesT& nodes , CoordsT& centroid);
  static void compute_normal(const NodesT& nodes , CoordsT& normal);

  //@}

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
      const Real x0 = nodes(0, XX);
      const Real y0 = nodes(0, YY);
      const Real z0 = nodes(0, ZZ);

      const Real x1 = nodes(1, XX);
      const Real y1 = nodes(1, YY);
      const Real z1 = nodes(1, ZZ);

      const Real x2 = nodes(2, XX);
      const Real y2 = nodes(2, YY);
      const Real z2 = nodes(2, ZZ);

      const Real x3 = nodes(3, XX);
      const Real y3 = nodes(3, YY);
      const Real z3 = nodes(3, ZZ);

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

////////////////////////////////////////////////////////////////////////////////

} // LagrangeP1
} // Mesh
} // CF

#endif // CF_Mesh_LagrangeP1_Quad3D_hpp
