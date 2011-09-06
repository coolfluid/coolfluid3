// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_LagrangeP3_Line_hpp
#define CF_Mesh_LagrangeP3_Line_hpp

#include "Math/MatrixTypes.hpp"
#include "Mesh/GeoShape.hpp"
#include "Mesh/LagrangeP3/LibLagrangeP3.hpp"

namespace CF {
namespace Mesh {
namespace LagrangeP3 {

////////////////////////////////////////////////////////////////////////////////

/// @class Line
/// @verbatim
/// Local connectivity:
///             0---3---4---1
/// Reference domain: <-1,1>
/// @endverbatim
/// @see ShapeFunction for documentation on undocumented static functions
class Mesh_LagrangeP3_API Line
{
public: // typedefs

  /// @name Shape function definitions
  //  --------------------------------
  //@{
  enum { dimensionality = 1               };
  enum { nb_nodes       = 4               };
  enum { order          = 3               };
  static const GeoShape::Type shape = GeoShape::LINE;
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

  Line() {}
  ~Line() {}
  static std::string type_name() { return "Line"; }

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

} // LagrangeP3
} // Mesh
} // CF

#endif // CF_Mesh_LagrangeP3_Line_hpp
