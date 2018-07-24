// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#ifndef cf3_solver_actions_Proto_Terminals_hpp
#define cf3_solver_actions_Proto_Terminals_hpp

#include<iostream>

#include <boost/proto/core.hpp>

#include "mesh/Elements.hpp"
#include "mesh/Region.hpp"
#include "mesh/ElementType.hpp"

/// @file
/// Some commonly used, statically defined terminal types

namespace cf3 {
namespace solver {
namespace actions {
namespace Proto {

/// Using this on a type always gives a compile error showing the type of T
template<typename T>
void print_error()
{
  T::print_error();
}

template<typename T>
void print_error(const T&)
{
  T::print_error();
}

/// Dummy shape function type used for element-based fields
template<Uint Dim>
struct ElementBased
{
  static const Uint dimension = Dim;
  static const Uint nb_nodes = 1;
  /// Mimic some shape function functionality, to avoid compile errors. Not that this is only used during the recursion on the types, and never actually used
  struct SF
  {
    typedef RealMatrix GradientT;
    typedef RealMatrix ValueT;
  };
};

/// Creates a variable that has unique ID I
template<typename I, typename T>
struct Var : I
{
  /// Type that is wrapped
  typedef T type;
  Var() {}

  /// Index of the var
  typedef I index_type;

  template<typename T1>
  Var(const T1& par1) : variable_value(par1) {}

  template<typename T1>
  Var(T1& par1) : variable_value(par1) {}

  template<typename T1, typename T2>
  Var(const T1& par1, const T2& par2) : variable_value(par1, par2) {}

  template<typename T1, typename T2>
  Var(T1& par1, T2& par2) : variable_value(par1, par2) {}

  template<typename T1, typename T2, typename T3>
  Var(const T1& par1, const T2& par2, const T3& par3) : variable_value(par1, par2, par3) {}

  template<typename T1, typename T2, typename T3>
  Var(T1& par1, T2& par2, T3& par3) : variable_value(par1, par2, par3) {}

  type variable_value;
};

/// Extract an index-type from a var
template<typename T>
struct IndexType
{
  typedef T type;
};

template<typename I, typename T>
struct IndexType< Var<I, T> >
{
  typedef I type;
};

/// Base class for field data
struct FieldBase
{
  FieldBase()
  {
  }

  /// Construct a new field placeholder
  /// @param name Variable name for the represented quantity (i.e. Temperature)
  /// @param field_tag Tag to identify the field
  /// @param space_lib_name The name of the space library used for the field
  FieldBase(const std::string& name, const std::string& field_tag, const std::string& space_lib_name = "geometry") :
    m_name(name),
    m_field_tag(field_tag),
    m_space_lib_name(space_lib_name)
  {
  }

  inline const std::string& name() const
  {
    return m_name;
  }

  inline const std::string& field_tag() const
  {
    return m_field_tag;
  }

  inline const std::string& space_lib_name() const
  {
    return m_space_lib_name;
  }

private:
  std::string m_name;
  std::string m_field_tag;
  std::string m_space_lib_name;
};

/// Field data for a scalar field
struct ScalarField : FieldBase
{
  ScalarField() : FieldBase() {}
  ScalarField(const std::string& varname, const std::string& field_tag) : FieldBase(varname, field_tag) {}
  ScalarField(const std::string& varname, const std::string& field_tag, const std::string& space_lib_name) : FieldBase(varname, field_tag, space_lib_name) {}
};

/// Field data for a vector having the dimension of the problem
struct VectorField : FieldBase
{
  VectorField() : FieldBase() {}
  VectorField(const std::string& varname, const std::string& field_tag) : FieldBase(varname, field_tag) {}
  VectorField(const std::string& varname, const std::string& field_tag, const std::string& space_lib_name) : FieldBase(varname, field_tag, space_lib_name) {}
};

/// Shorthand for terminals containing a numbered variable
template<Uint I, typename T>
struct NumberedTermType
{
  typedef typename boost::proto::result_of::make_expr
  <
    boost::proto::tag::terminal
  , Var<boost::mpl::int_<I>, T>
  >::type type;
};

template<Uint I, typename T>
struct FieldVariable :
  boost::proto::extends< typename NumberedTermType<I, T>::type, FieldVariable<I, T> >
{
  typedef boost::proto::extends< typename NumberedTermType<I, T>::type, FieldVariable<I, T> > base_type;

  FieldVariable() : base_type(boost::proto::make_expr<boost::proto::tag::terminal>(Var<boost::mpl::int_<I>, T>())) {}

  template<typename T1>
  FieldVariable(const T1& par1) : base_type(boost::proto::make_expr<boost::proto::tag::terminal>(Var<boost::mpl::int_<I>, T>(par1))) {}

  template<typename T1>
  FieldVariable(T1& par1) : base_type(boost::proto::make_expr<boost::proto::tag::terminal>(Var<boost::mpl::int_<I>, T>(par1))) {}

  template<typename T1, typename T2>
  FieldVariable(const T1& par1, const T2& par2) : base_type(boost::proto::make_expr<boost::proto::tag::terminal>(Var<boost::mpl::int_<I>, T>(par1, par2))) {}

  template<typename T1, typename T2>
  FieldVariable(T1& par1, T2& par2) : base_type(boost::proto::make_expr<boost::proto::tag::terminal>(Var<boost::mpl::int_<I>, T>(par1, par2))) {}

  template<typename T1, typename T2, typename T3>
  FieldVariable(const T1& par1, const T2& par2, const T3& par3) : base_type(boost::proto::make_expr<boost::proto::tag::terminal>(Var<boost::mpl::int_<I>, T>(par1, par2, par3))) {}

  template<typename T1, typename T2, typename T3>
  FieldVariable(T1& par1, T2& par2, T3& par3) : base_type(boost::proto::make_expr<boost::proto::tag::terminal>(Var<boost::mpl::int_<I>, T>(par1, par2, par3))) {}

  BOOST_PROTO_EXTENDS_USING_ASSIGN(FieldVariable)
};

/// Match field types
struct FieldTypes :
  boost::proto::or_
  <
    boost::proto::terminal< Var< boost::proto::_, ScalarField > >,
    boost::proto::terminal< Var<boost::proto::_, VectorField> >
  >
{
};

struct ZeroTag
{
};

/// Placeholder for the zero matrix
static boost::proto::terminal<ZeroTag>::type _0 = {};

struct IdentityTag
{
};

/// Placeholder for the identity matrix
static boost::proto::terminal<IdentityTag>::type _I = {};

/// Wrap std::cout
static boost::proto::terminal< std::ostream & >::type _cout = {std::cout};

/// Accept a 2D realvector for atan2
inline Real atan_vec(const RealVector2& vec)
{
  return atan2(vec[1], vec[0]);
}

/// Maximum between two scalars
inline Real max(const Real a, const Real b)
{
  return a > b ? a : b;
}

/// Minimum between two scalars
inline Real min(const Real a, const Real b)
{
  return a < b ? a : b;
}

// Wrap some math functions
static boost::proto::terminal< double(*)(double) >::type const _sin = {&sin};
static boost::proto::terminal< double(*)(double, double) >::type const _atan2 = {&atan2};
static boost::proto::terminal< double(*)(const RealVector2&) >::type const _atan_vec = {&atan_vec};
static boost::proto::terminal< double(*)(double) >::type const _exp = {&exp};
static boost::proto::terminal< double(*)(double) >::type const _sqrt = {&sqrt};
static boost::proto::terminal< double(*)(double) >::type const _abs = {&fabs};
static boost::proto::terminal< double(*)(double, double) >::type const _max = {&max};
static boost::proto::terminal< double(*)(double, double) >::type const _min = {&min};
static boost::proto::terminal< double(*)(double, double) >::type const _std_pow = {&std::pow};

// Function for partial derivatives in index notation
struct PartialTag
{
};

static boost::proto::terminal<PartialTag>::type partial = {};

/// Base class of all functions that can be evaluated using "default" C++ semantics
struct FunctionBase
{
};

// Helpers for putting lambdas in terminals
template<typename T>
struct LambdaTraits
{
};

template<typename R, typename LambdaT, typename... ArgsT>
struct LambdaTraits<R (LambdaT::*)(ArgsT...) const>
{
  typedef R return_type;
};

template<typename T>
struct LambdaWrapper : FunctionBase
{
  typedef typename LambdaTraits<decltype(&T::operator())>::return_type result_type;

  LambdaWrapper(const T& lambda) : m_wrapped(lambda)
  {
  }

  template<typename... ArgsT>
  inline result_type operator()(ArgsT... args) const
  {
    return m_wrapped(args...);
  }

  T m_wrapped;
};

/// Make a proto expression out of a C++11 lambda function
template<typename T>
auto make_lambda(const T& lambda) -> decltype(boost::proto::as_expr(LambdaWrapper<T>(lambda)))
{
  return boost::proto::as_expr(LambdaWrapper<T>(lambda));
}


} // namespace Proto
} // namespace actions
} // namespace solver
} // namespace cf3

#endif // cf3_solver_actions_Proto_Terminals_hpp
