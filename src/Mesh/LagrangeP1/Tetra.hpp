// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_LagrangeP1_Tetra_hpp
#define CF_Mesh_LagrangeP1_Tetra_hpp

#include "Math/MatrixTypes.hpp"
#include "Mesh/GeoShape.hpp"
#include "Mesh/LagrangeP1/LibLagrangeP1.hpp"

namespace CF {
namespace Mesh {
namespace LagrangeP1 {

////////////////////////////////////////////////////////////////////////////////

/// @class Tetra
/// @verbatim
/// Local connectivity:
///             2------3
///             | \   . \
///             |   \.   \
///             |   . \   \
///             |  .    \  \
///             | .       \ \
///             0------------1
/// Reference domain: <0,1> x <0,1>
/// @endverbatim
/// @see ShapeFunction for documentation on undocumented static functions
class Mesh_LagrangeP1_API Tetra
{
public: // typedefs

  /// @name Shape function definitions
  //  --------------------------------
  //@{
  enum { dimensionality = 3               };
  enum { nb_nodes       = 4               };
  enum { order          = 1               };
  static const GeoShape::Type shape = GeoShape::TETRA;
  //@}

  /// @name Matrix Types
  //  --------------------------------
  //@{
  typedef Eigen::Matrix<Real, dimensionality, 1> MappedCoordsT;
  typedef Eigen::Matrix<Real, 1, nb_nodes> ValueT;
  typedef Eigen::Matrix<Real, dimensionality, nb_nodes> GradientT;
  //@}

public: // functions

  /// @name Constructor / Destructor / Type name
  //  ------------------------------------------
  //@{

  Tetra() {}
  ~Tetra() {}
  static std::string type_name() { return "Tetra"; }

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

} // LagrangeP1
} // Mesh
} // CF

#endif // CF_Mesh_LagrangeP1_Tetra_hpp
