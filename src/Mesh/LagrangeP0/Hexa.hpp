// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_LagrangeP0_Hexa_hpp
#define CF_Mesh_LagrangeP0_Hexa_hpp

#include "Math/MatrixTypes.hpp"
#include "Mesh/LagrangeP0/LibLagrangeP0.hpp"

namespace CF {
namespace Mesh {
namespace LagrangeP0 {

////////////////////////////////////////////////////////////////////////////////

/// @class Hexa
/// @verbatim
/// Local connectivity:
///                -------------
///               /|          /|
///              / |         / |
///             |-----------|  |
///             |  |        |  |
///             |  |   0    |  |
///             |  |--------|--|
///             | /         | /
///     ETA     |/          |/
///      |      -------------
///      |
///      o--- KSI
///     /
///    /
///  ZTA
/// Reference domain: <-1,1> x <-1,1> x <-1,1>
/// @endverbatim
/// @see ShapeFunction for documentation on undocumented static functions
struct Mesh_LagrangeP0_API Hexa
{
public: // typedefs

  /// @name Shape function definitions
  //  --------------------------------
  //@{
  enum { dimensionality = 3               };
  enum { nb_nodes       = 1               };
  enum { order          = 0               };
  static const GeoShape::Type shape = GeoShape::HEXA;
  //@}

  /// @name Matrix Types
  //  ------------------
  //@{
  typedef Eigen::Matrix<Real, dimensionality, 1> MappedCoordsT;
  typedef Eigen::Matrix<Real, 1, nb_nodes> ValueT;
  typedef Eigen::Matrix<Real, dimensionality, nb_nodes> GradientT;
  //@}

public: // functions

  /// @name Constructor / Destructor / Type name
  //  ------------------------------------------
  //@{

  Hexa() {}
  ~Hexa() {}
  static std::string type_name() { return "Hexa"; }

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

#endif // CF_Mesh_LagrangeP0_Hexa_hpp
