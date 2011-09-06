// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_LagrangeP0_Point2D_hpp
#define CF_Mesh_LagrangeP0_Point2D_hpp

#include "Mesh/ElementType.hpp"
#include "Mesh/LagrangeP0/Point.hpp"

namespace CF {
namespace Mesh {
  template <typename SF> class ShapeFunctionT;
namespace LagrangeP0 {

////////////////////////////////////////////////////////////////////////////////

/// @brief 2D Lagrange P0 Point Element type
/// This class provides the lagrangian shape function describing the
/// representation of the solution and/or the geometry in a P0 (constant)
/// point element.
/// @see ElementType for documentation of undocumented functions
/// @author Willem Deconinck
/// @author Tiago Quintino
/// @author Bart Janssens
struct Mesh_LagrangeP0_API Point2D
{
public: // typedefs

  typedef boost::shared_ptr<Point2D>       Ptr;
  typedef boost::shared_ptr<Point2D const> ConstPtr;

  /// The shape function of this element
  typedef Point SF;

  /// @name Element definitions
  //  -------------------------
  //@{
  static const GeoShape::Type shape = SF::shape;
  enum { dimensionality = SF::dimensionality };
  enum { nb_nodes       = SF::nb_nodes       };
  enum { order          = SF::order          };

  enum { dimension      = 2 };
  enum { nb_faces       = 0 };
  enum { nb_edges       = 0 };
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

  Point2D() {}
  ~Point2D() {}
  static std::string type_name() { return "Point2D"; }

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

  static Real volume(const NodesT& nodes);
  static Real area(const NodesT& nodes);
  static void compute_centroid(const NodesT& nodes , CoordsT& centroid);
  static bool is_coord_in_element(const CoordsT& coord, const NodesT& nodes);

  //@}

};

////////////////////////////////////////////////////////////////////////////////

} // LagrangeP0
} // Mesh
} // CF

#endif // CF_Mesh_LagrangeP0_Point2D_hpp
