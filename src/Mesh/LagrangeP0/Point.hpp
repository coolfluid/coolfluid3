// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_LagrangeP0_Point_hpp
#define CF_Mesh_LagrangeP0_Point_hpp

#include "Math/MatrixTypes.hpp"
#include "Mesh/LagrangeP0/LibLagrangeP0.hpp"

namespace CF {
namespace Mesh {
namespace LagrangeP0 {

////////////////////////////////////////////////////////////////////////////////

/// @class Point
/// @verbatim
/// Local connectivity:
///
///      0
/// Reference domain: <0,0>
/// @endverbatim
/// @see ShapeFunction for documentation on undocumented static functions
struct Mesh_LagrangeP0_API Point
{
public: // typedefs

  /// @name Shape function definitions
  //  --------------------------------
  //@{
  enum { dimensionality = 0               };
  enum { nb_nodes       = 1               };
  enum { order          = 0               };
  enum { shape          = GeoShape::POINT };
  //@}

  /// @name Matrix Types
  //  ------------------
  //@{
  typedef RealVector                                    MappedCoordsT;
  typedef Eigen::Matrix<Real, 1, nb_nodes>              ValueT;
  typedef Eigen::Matrix<Real, 3, nb_nodes>              GradientT;
  //@}

public: // functions

  /// @name Constructor / Destructor / Type name
  //  ------------------------------------------
  //@{

  Point() {}
  ~Point() {}
  static std::string type_name() { return "Point"; }

  //@}

  /// @name Accessor functions
  //  ------------------------
  //@{

  static const RealMatrix& local_coordinates();

  //@}

  /// @name Computation functions
  //  ---------------------------
  //@{

  static ValueT value(const MappedCoordsT& mapped_coord);
  static void compute_value(const MappedCoordsT& mapped_coord, ValueT& result);
  static GradientT gradient(const MappedCoordsT& mapped_coord);
  static void compute_gradient(const MappedCoordsT& mapped_coord, GradientT& result);

  //@}
};

////////////////////////////////////////////////////////////////////////////////

} // LagrangeP0
} // Mesh
} // CF

#endif // CF_Mesh_LagrangeP0_Point_hpp
