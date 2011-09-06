// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_LagrangeP3_Triag2D_hpp
#define CF_Mesh_LagrangeP3_Triag2D_hpp

#include "Mesh/ElementType.hpp"
#include "Mesh/LagrangeP3/Triag.hpp"

namespace CF {
namespace Mesh {
  template <typename SF> class ShapeFunctionT;
namespace LagrangeP3 {

////////////////////////////////////////////////////////////////////////////////

/// @brief 2D Lagrange P3 Triangular Element type
/// This class provides the lagrangian shape function describing the
/// representation of the solution and/or the geometry in a P3 (linear)
/// triangular element.
/// @see ElementType for documentation of undocumented functions
/// @author Willem Deconinck
/// @author Tiago Quintino
/// @author Bart Janssens
struct Mesh_LagrangeP3_API Triag2D
{
public: // typedefs

  typedef boost::shared_ptr<Triag2D>       Ptr;
  typedef boost::shared_ptr<Triag2D const> ConstPtr;

  /// The shape function of this element
  typedef Triag SF;

  /// @name Element definitions
  //  -------------------------
  //@{
  static const GeoShape::Type shape = SF::shape;
  enum { dimensionality = SF::dimensionality };
  enum { nb_nodes       = SF::nb_nodes       };
  enum { order          = SF::order          };

  enum { dimension      = 2 };
  enum { nb_faces       = 3 };
  enum { nb_edges       = 3 };
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

  Triag2D() {}
  ~Triag2D() {}
  static std::string type_name() { return "Triag2D"; }

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

  static Real area(const NodesT& nodes);

  //@}

};

////////////////////////////////////////////////////////////////////////////////

} // LagrangeP3
} // Mesh
} // CF

#endif // CF_Mesh_LagrangeP3_Triag2D_hpp
