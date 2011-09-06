// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

/// @file
/// @brief This file deals with the translation from the dynamic API of
/// ShapeFunction to static implementations of shape functions.
///
/// Implementations of ShapeFunction don't inherit from Common::Component
/// e.g. LagrangeP1::Triag. \n
/// The actual concrete component is created as ShapeFunction<LagrangeP1::Triag>
///
/// @author Willem Deconinck

#ifndef CF_Mesh_ShapeFunctionT_hpp
#define CF_Mesh_ShapeFunctionT_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/mpl/if.hpp>

#include "Mesh/ShapeFunction.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

////////////////////////////////////////////////////////////////////////////////

/// @macro Check if a class has a static member function defined
/// based on SFINAE principle : substitution failure is not an error
/// example:
/// HAS_STATIC_MEMBER_XXX(foo)
/// HAS_STATIC_MEMBER_XXX(bar)
/// struct MyStruct { static void foo(); }
/// bool has_foo = has_static_member_foo<MyStruct>::value; // true
/// bool has_bar = has_static_member_bar<MyStruct>::value; // false
#define HAS_STATIC_MEMBER_XXX(XXX)                                       \
template <typename T> struct has_static_member_##XXX                     \
{                                                                        \
  typedef char Yes;                                                      \
  class No { char c[2]; };                                               \
                                                                         \
  /* murk<> helps to let us call test() with an integer argument */      \
  template <int> struct murk { murk(int); };                             \
                                                                         \
  template <typename S> static Yes test (murk< sizeof(&S::XXX) >);       \
  template <typename S> static No  test (...);                           \
                                                                         \
  enum { value = sizeof(Yes) == sizeof(test<T>(0)) };                    \
};

////////////////////////////////////////////////////////////////////////////////

/// @brief Fallback class if a concrete ShapeFunction doesn't implement a static function
///
/// Implements all functions ShapeFunctionT expects.
/// When creating a new shape function, this list shows all the possible static functions
/// that can be implemented and are recognized by the dynamic interface from ShapeFunction
/// @author Willem Deconinck
class Mesh_API ShapeFunctionFallBack
{
public:
  typedef RealVector MappedCoordsT;
  typedef RealRowVector ValueT;
  typedef RealMatrix GradientT;

  static const RealMatrix& local_coordinates();
  static ValueT value(const MappedCoordsT& mapped_coord);
  static void compute_value(const MappedCoordsT& mapped_coord, ValueT& result);
  static GradientT gradient(const MappedCoordsT& mapped_coord);
  static void compute_gradient(const MappedCoordsT& mapped_coord, GradientT& result);
};

////////////////////////////////////////////////////////////////////////////////

/// @brief Translation class to link concrete static implementations to the dynamic API
template <typename SF>
class ShapeFunctionT : public ShapeFunction
{
public: // typedefs

  typedef boost::shared_ptr< ShapeFunctionT >        Ptr;
  typedef boost::shared_ptr< ShapeFunctionT const > ConstPtr;

  // create const static bool has_static_member_xxx<T>::value
  //     where xxx is replaced by the given argument
  HAS_STATIC_MEMBER_XXX(local_coordinates)
  HAS_STATIC_MEMBER_XXX(value)
  HAS_STATIC_MEMBER_XXX(compute_value)
  HAS_STATIC_MEMBER_XXX(gradient)
  HAS_STATIC_MEMBER_XXX(compute_gradient)

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
    typedef typename boost::mpl::if_c<has_static_member_local_coordinates<SF>::value,SF,ShapeFunctionFallBack>::type SFT;
    return SFT::local_coordinates();
  }


  // Computation functions
  // ---------------------------

  virtual RealRowVector value(const RealVector& local_coordinate) const
  {
    typedef typename boost::mpl::if_c<has_static_member_value<SF>::value,SF,ShapeFunctionFallBack>::type SFT;
    return SFT::value(local_coordinate);
  }

  virtual void compute_value(const RealVector& local_coordinate, RealRowVector& value) const
  {
    typedef typename boost::mpl::if_c<has_static_member_compute_value<SF>::value,SF,ShapeFunctionFallBack>::type SFT;
    // This little hack is necessary and is documented on the Eigen website
    // http://eigen.tuxfamily.org/dox/TopicFunctionTakingEigenTypes.html
    typename SFT::ValueT const& val = const_cast<RealRowVector const&>(value);
    SFT::compute_value(local_coordinate, const_cast<typename SFT::ValueT&>(val));
    value = val;
  }

  virtual RealMatrix gradient(const RealVector& local_coordinate) const
  {
    typedef typename boost::mpl::if_c<has_static_member_gradient<SF>::value,SF,ShapeFunctionFallBack>::type SFT;
    return SFT::gradient(local_coordinate);
  }

  virtual void compute_gradient(const RealVector& local_coordinate, RealMatrix& gradient) const
  {
    typedef typename boost::mpl::if_c<has_static_member_compute_gradient<SF>::value,SF,ShapeFunctionFallBack>::type SFT;
    // This little hack is necessary and is documented on the Eigen website
    // http://eigen.tuxfamily.org/dox/TopicFunctionTakingEigenTypes.html
    const typename SFT::GradientT& grad( const_cast<RealMatrix const&>(gradient));
    SFT::compute_gradient(local_coordinate, const_cast<typename SFT::GradientT&>(grad));
    gradient = grad;
  }
};

////////////////////////////////////////////////////////////////////////////////

#undef HAS_STATIC_MEMBER_XXX

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_ShapeFunctionT_hpp
