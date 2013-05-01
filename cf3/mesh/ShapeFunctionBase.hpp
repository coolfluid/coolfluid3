// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

/// @file
/// @brief Base class to inherit shape function implementations from
/// @author Willem Deconinck

#ifndef cf3_mesh_ShapeFunctionBase_hpp
#define cf3_mesh_ShapeFunctionBase_hpp

////////////////////////////////////////////////////////////////////////////////

#include "math/MatrixTypes.hpp"
#include "mesh/GeoShape.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {

////////////////////////////////////////////////////////////////////////////////

/// @brief Fallback class if a concrete ShapeFunction doesn't implement a static function
///
/// Implements all functions ShapeFunctionT expects.
/// When creating a new shape function, this list shows all the possible static functions
/// that can be implemented and are recognized by the dynamic interface from ShapeFunction
/// @tparam SF Concrete shapefunction implementation with only static functions
/// @tparam TR Traits of the implementation, containing
///  - shape
///  - nb_nodes
///  - dimensionality
///  - order
/// The TR template parameter is necessary to be able to implement default implementations
/// and define special matrix types.
/// @author Willem Deconinck
template <typename SF, typename TR>
class ShapeFunctionBase
{
public:

  // Information coming from Traits
  // ------------------------------
  static const GeoShape::Type shape          = (GeoShape::Type) TR::shape;
  static const Uint           dimensionality = (Uint) TR::dimensionality;
  static const Uint           order          = (Uint) TR::order;
  static const Uint           nb_nodes       = (Uint) TR::nb_nodes;
  static const Uint           nb_faces       = (Uint) TR::nb_faces;

  static std::string type_name() { return GeoShape::Convert::instance().to_str(shape); }

  // Typedefs for special matrices
  // -----------------------------
  typedef Eigen::Matrix<Real, dimensionality, 1       >  MappedCoordsT;
  typedef Eigen::Matrix<Real, 1             , nb_nodes>  ValueT;
  typedef Eigen::Matrix<Real, dimensionality, nb_nodes>  GradientT;

  // Not-implemented static functions
  // --------------------------------
  static const RealMatrix& local_coordinates();
  static const RealMatrix& face_normals();
  static void compute_value(const MappedCoordsT& mapped_coord, ValueT& result);
  static void compute_gradient(const MappedCoordsT& mapped_coord, GradientT& result);

  // Implemented static functions
  // ----------------------------
  static ValueT value(const MappedCoordsT& mapped_coord);
  static GradientT gradient(const MappedCoordsT& mapped_coord);

private:

  static void throw_not_implemented(const common::CodeLocation& where);

};

////////////////////////////////////////////////////////////////////////////////

template <typename SF,typename TR>
const GeoShape::Type ShapeFunctionBase<SF,TR>::shape;

template <typename SF,typename TR>
const Uint ShapeFunctionBase<SF,TR>::nb_nodes;

template <typename SF,typename TR>
const Uint ShapeFunctionBase<SF,TR>::nb_faces;

template <typename SF,typename TR>
const Uint ShapeFunctionBase<SF,TR>::dimensionality;

template <typename SF,typename TR>
const Uint ShapeFunctionBase<SF,TR>::order;

////////////////////////////////////////////////////////////////////////////////

template <typename SF,typename TR>
inline void ShapeFunctionBase<SF,TR>::throw_not_implemented(const common::CodeLocation& where)
{
  throw common::NotImplemented(where,"static function not implemented / not applicable for shape function ["+SF::type_name()+"]");
}

////////////////////////////////////////////////////////////////////////////////

template <typename SF,typename TR>
inline const RealMatrix& ShapeFunctionBase<SF,TR>::local_coordinates()
{
  throw_not_implemented(FromHere());
  const static RealMatrix local_coords_obj;
  return local_coords_obj;
}

////////////////////////////////////////////////////////////////////////////////

template <typename SF,typename TR>
inline const RealMatrix& ShapeFunctionBase<SF,TR>::face_normals()
{
  throw_not_implemented(FromHere());
  const static RealMatrix face_normal_obj;
  return face_normal_obj;
}

////////////////////////////////////////////////////////////////////////////////

template <typename SF,typename TR>
inline void ShapeFunctionBase<SF,TR>::compute_value(const MappedCoordsT& mapped_coord, ValueT& result)
{
  throw_not_implemented(FromHere());
}

////////////////////////////////////////////////////////////////////////////////

template <typename SF,typename TR>
inline void ShapeFunctionBase<SF,TR>::compute_gradient(const MappedCoordsT& mapped_coord, GradientT& result)
{
  throw_not_implemented(FromHere());
}

////////////////////////////////////////////////////////////////////////////////

template <typename SF,typename TR>
inline typename ShapeFunctionBase<SF,TR>::ValueT ShapeFunctionBase<SF,TR>::value(const MappedCoordsT& mapped_coord)
{
  ValueT result;
  SF::compute_value(mapped_coord,result);
  return result;
}

////////////////////////////////////////////////////////////////////////////////

template <typename SF,typename TR>
inline typename ShapeFunctionBase<SF,TR>::GradientT ShapeFunctionBase<SF,TR>::gradient(const MappedCoordsT& mapped_coord)
{
  GradientT result;
  SF::compute_gradient(mapped_coord,result);
  return result;
}

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_ShapeFunctionBase_hpp
