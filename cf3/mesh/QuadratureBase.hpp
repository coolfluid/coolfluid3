// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

/// @file
/// @brief Base class to inherit shape function implementations from
/// @author Willem Deconinck

#ifndef cf3_mesh_QuadratureBase_hpp
#define cf3_mesh_QuadratureBase_hpp

////////////////////////////////////////////////////////////////////////////////

#include "math/MatrixTypes.hpp"
#include "mesh/GeoShape.hpp"
#include "common/StringConversion.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {

////////////////////////////////////////////////////////////////////////////////

/// @brief Fallback class if a concrete Quadrature doesn't implement a static function
///
/// Implements all functions QuadratureT expects.
/// When creating a new shape function, this list shows all the possible static functions
/// that can be implemented and are recognized by the dynamic interface from Quadrature
/// @tparam QDR Concrete shapefunction implementation with only static functions
/// @tparam TR Traits of the implementation, containing
///  - shape
///  - nb_nodes
///  - dimensionality
///  - order
/// The TR template parameter is necessary to be able to implement default implementations
/// and define special matrix types.
/// @author Willem Deconinck
template <typename TR>
class QuadratureBase
{
public:

  // Information coming from Traits
  // ------------------------------
  static const GeoShape::Type shape          = (GeoShape::Type) TR::shape;
  static const Uint           dimensionality = (Uint) TR::dimensionality;
  static const Uint           order          = (Uint) TR::order;
  static const Uint           nb_nodes       = (Uint) TR::nb_nodes;

  static std::string type_name() { return GeoShape::Convert::instance().to_str(shape)+"P"+common::to_str(order); }

  // Typedefs for special matrices
  // -----------------------------
  typedef Eigen::Matrix<Real, nb_nodes, dimensionality>  LocalCoordsT;
  typedef Eigen::Matrix<Real, 1,        nb_nodes      >  WeightsT;

  // Quadrature Coordinates in isoparametric element
  // -----------------------------------------------
  static const LocalCoordsT& local_coordinates();

  // Quadrature weights in isoparametric element
  // -------------------------------------------
  static const WeightsT& weights();

private:

  static void throw_not_implemented(const common::CodeLocation& where);

};

////////////////////////////////////////////////////////////////////////////////

template <typename TR>
const GeoShape::Type QuadratureBase<TR>::shape;

template <typename TR>
const Uint QuadratureBase<TR>::nb_nodes;

template <typename TR>
const Uint QuadratureBase<TR>::dimensionality;

template <typename TR>
const Uint QuadratureBase<TR>::order;

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_QuadratureBase_hpp
