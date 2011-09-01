// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_ShapeFunction_hpp
#define CF_Mesh_ShapeFunction_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"

#include "Math/MatrixTypes.hpp"

#include "Mesh/GeoShape.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

////////////////////////////////////////////////////////////////////////////////

/// This class represents the API to a ShapeFunction.
/// An object of this class is typically owned by a CSpace object,
/// which in turn is owned by a CEntities object describing the element.
/// A shape function only exists in local coordinates. It has only a
/// dimensionality, and not a dimension.
/// @author Willem Deconinck
class Mesh_API ShapeFunction : public Common::Component {

public: // typedefs

  typedef boost::shared_ptr< ShapeFunction > Ptr;
  typedef boost::shared_ptr< ShapeFunction const> ConstPtr;

public: // functions

  /// @name Constructor / Destructor / Type name
  //  ------------------------------------------
  //@{

  /// Default constructor without arguments
  ShapeFunction( const std::string& name );

  /// Default destructor
  virtual ~ShapeFunction();

  /// Type name: ShapeFunction
  static std::string type_name() { return "ShapeFunction"; }

  //@}

  /// @name Accessor functions
  //  ------------------------
  //@{

  /// @return shape as string
  std::string shape_name() const { return Mesh::GeoShape::Convert::instance().to_str( m_shape ); }

  /// @return shape as enumerator
  GeoShape::Type shape() const  {  return m_shape; }

  /// @return number of nodes
  Uint nb_nodes() const  { return m_nb_nodes; }

  /// @return order
  Uint order() const { return m_order; }

  /// @return dimensionality (e.g. shell in 3D world: dimensionality = 2)
  Uint dimensionality() const { return m_dimensionality; }

  /// @return a matrix with all local coordinates where the shape function is defined
  virtual const RealMatrix& local_coordinates() const;

  //@}

  /// @name Computation functions
  //  ---------------------------
  //@{

  /// @brief Compute the shape function values in the given local coordinate
  /// @param [in] local_coordinate   local coordinate (size = dimensionality x 1)
  /// @return shape function values (size = 1 x nb_nodes)
  virtual RealRowVector value(const RealVector& local_coordinate) const;

  /// @brief Compute the shape function values in the given local coordinate
  /// @param [in]  local_coordinate   local coordinate (size = dimensionality x 1)
  /// @param [out] value              computed value (size = 1 x nb_nodes)
  virtual void compute_value(const RealVector& local_coordinate, RealRowVector& value) const;

  /// @brief Compute the shape function gradient in the given local coordinate
  /// @param [in] local_coordinate   local coordinate (size = dimensionality x 1)
  /// @return shape function gradient (size = nb_nodes x dimensionality)
  virtual RealMatrix gradient(const RealVector& local_coordinate) const;

  /// @brief Compute the shape function values in the given local coordinate
  /// @param [in]  local_coordinate   local coordinate (size = dimensionality x 1)
  /// @param [out] gradient           computed gradient (size = nb_nodes x dimensionality)
  virtual void compute_gradient(const RealVector& local_coordinate, RealMatrix& gradient) const;

  //@}

protected: // data

  /// the GeoShape::Type corresponding to the shape
  GeoShape::Type m_shape;
  /// the number of nodes used in this shape function
  Uint m_nb_nodes;
  /// the order of this shape function
  Uint m_order;
  /// the dimensionality of the element
  Uint m_dimensionality;

}; // ShapeFunction

////////////////////////////////////////////////////////////////////////////////

/// @brief Translation class to link concrete static implementations to the dynamic API
template <typename SF>
class ShapeFunctionT : public ShapeFunction
{
public:
  /// @name Constructor / Destructor / Type name
  //  ------------------------------------------
  //@{

  /// Default constructor without arguments
  ShapeFunctionT( const std::string& name ) : ShapeFunction(name) {}

  /// Default destructor
  virtual ~ShapeFunctionT() {}

  /// Type name: ShapeFunction
  static std::string type_name() { return "ShapeFunction"; }

  //@}

  //  Accessor functions
  //  ------------------------

  virtual const RealMatrix& local_coordinates() const
  {
    return SF::local_coordinates();
  }


  // Computation functions
  // ---------------------------

  virtual RealRowVector value(const RealVector& local_coordinate) const
  {
    return SF::value(local_coordinate);
  }

  virtual void compute_value(const RealVector& local_coordinate, RealRowVector& value) const
  {
    // This little hack is necessary and is documented on the Eigen website
    // http://eigen.tuxfamily.org/dox/TopicFunctionTakingEigenTypes.html
    const typename SF::ValueT& val( const_cast<RealRowVector const&>(value) );
    SF::compute_value(local_coordinate, const_cast<typename SF::ValueT&>(val));
  }

  virtual RealMatrix gradient(const RealVector& local_coordinate) const
  {
    return SF::gradient(local_coordinate);
  }

  virtual void compute_gradient(const RealVector& local_coordinate, RealMatrix& gradient) const
  {
    // This little hack is necessary and is documented on the Eigen website
    // http://eigen.tuxfamily.org/dox/TopicFunctionTakingEigenTypes.html
    const typename SF::GradientT& grad( const_cast<RealMatrix const&>(gradient));
    SF::compute_gradient(local_coordinate, const_cast<typename SF::GradientT&>(grad));
  }
};

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_ShapeFunction_hpp
