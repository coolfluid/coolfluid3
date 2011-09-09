// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

/// @file
/// @brief Translation from dynamic to static API
///
/// This file deals with the translation from the dynamic API of
/// ShapeFunction to static implementations of shape functions.
/// Implementations of Shapefunctions only implement static functions, have
/// a default constructor, and inherit from ShapeFunctionBase.\n
/// The concrete dynamic implementation
/// is created as ShapeFunctionT<LagrangeP1::Triag>, wrapping the static
/// implementation in a component inheriting from ShapeFunction.
/// @author Willem Deconinck

#ifndef CF_Mesh_ShapeFunctionT_hpp
#define CF_Mesh_ShapeFunctionT_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Mesh/ShapeFunction.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

////////////////////////////////////////////////////////////////////////////////


/// @brief Translation class to link concrete static implementations to the dynamic API
template <typename SF>
class ShapeFunctionT : public ShapeFunction
{
public: // typedefs

  typedef boost::shared_ptr< ShapeFunctionT >        Ptr;
  typedef boost::shared_ptr< ShapeFunctionT const > ConstPtr;

public:
  /// @name Constructor / Destructor / Type name
  //  ------------------------------------------
  //@{

  /// Default constructor without arguments
  ShapeFunctionT( const std::string& name = type_name() ) : ShapeFunction(name)
  {
    m_dimensionality = SF::dimensionality;
    m_nb_nodes = SF::nb_nodes;
    m_order = SF::order;
    m_shape = SF::shape;
  }

  /// Default destructor
  virtual ~ShapeFunctionT() {}

  /// Type name: ShapeFunction
  static std::string type_name() { return SF::type_name(); }

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
    typename SF::ValueT const& val = const_cast<RealRowVector const&>(value);
    SF::compute_value(local_coordinate, const_cast<typename SF::ValueT&>(val));
    value = val;
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
    gradient = grad;
  }
};

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_ShapeFunctionT_hpp
