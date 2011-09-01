// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#ifndef CF_Solver_Actions_Proto_Terminals_hpp
#define CF_Solver_Actions_Proto_Terminals_hpp

#include<iostream>

#include <boost/proto/core.hpp>

#include "Mesh/CElements.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/ElementType.hpp"

/// @file
/// Some commonly used, statically defined terminal types

namespace CF {
namespace Solver {
namespace Actions {
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
  FieldBase(const std::string& name, const std::string& field_tag) :
    m_name(name),
    m_field_tag(field_tag)
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

private:
  std::string m_name;
  std::string m_field_tag;
};

/// Field data for a scalar field
struct ScalarField : FieldBase
{
  ScalarField() : FieldBase() {}
  ScalarField(const std::string& varname, const std::string& field_tag) : FieldBase(varname, field_tag) {}
};

/// Field data for a vector having the dimension of the problem
struct VectorField : FieldBase
{
  VectorField() : FieldBase() {}
  VectorField(const std::string& varname, const std::string& field_tag) : FieldBase(varname, field_tag) {}
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
struct MeshTerm :
  boost::proto::extends< typename NumberedTermType<I, T>::type, MeshTerm<I, T> >
{
  typedef boost::proto::extends< typename NumberedTermType<I, T>::type, MeshTerm<I, T> > base_type;

  MeshTerm() : base_type(boost::proto::make_expr<boost::proto::tag::terminal>(Var<boost::mpl::int_<I>, T>())) {}

  template<typename T1>
  MeshTerm(const T1& par1) : base_type(boost::proto::make_expr<boost::proto::tag::terminal>(Var<boost::mpl::int_<I>, T>(par1))) {}

  template<typename T1>
  MeshTerm(T1& par1) : base_type(boost::proto::make_expr<boost::proto::tag::terminal>(Var<boost::mpl::int_<I>, T>(par1))) {}

  template<typename T1, typename T2>
  MeshTerm(const T1& par1, const T2& par2) : base_type(boost::proto::make_expr<boost::proto::tag::terminal>(Var<boost::mpl::int_<I>, T>(par1, par2))) {}

  template<typename T1, typename T2>
  MeshTerm(T1& par1, T2& par2) : base_type(boost::proto::make_expr<boost::proto::tag::terminal>(Var<boost::mpl::int_<I>, T>(par1, par2))) {}

  template<typename T1, typename T2, typename T3>
  MeshTerm(const T1& par1, const T2& par2, const T3& par3) : base_type(boost::proto::make_expr<boost::proto::tag::terminal>(Var<boost::mpl::int_<I>, T>(par1, par2, par3))) {}

  template<typename T1, typename T2, typename T3>
  MeshTerm(T1& par1, T2& par2, T3& par3) : base_type(boost::proto::make_expr<boost::proto::tag::terminal>(Var<boost::mpl::int_<I>, T>(par1, par2, par3))) {}

  BOOST_PROTO_EXTENDS_USING_ASSIGN(MeshTerm)
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

/// Wrap std::cout
static boost::proto::terminal< std::ostream & >::type _cout = {std::cout};

/// Accept a 2D realvector for atan2
inline Real atan_vec(const RealVector2& vec)
{
  return atan2(vec[1], vec[0]);
}

// Wrap some math functions
static boost::proto::terminal< double(*)(double) >::type const _sin = {&sin};
static boost::proto::terminal< double(*)(double, double) >::type const _atan2 = {&atan2};
static boost::proto::terminal< double(*)(const RealVector2&) >::type const _atan_vec = {&atan_vec};
static boost::proto::terminal< double(*)(double) >::type const _exp = {&exp};
static boost::proto::terminal< double(*)(double) >::type const _sqrt = {&sqrt};

} // namespace Proto
} // namespace Actions
} // namespace Solver
} // namespace CF

#endif // CF_Solver_Actions_Proto_Terminals_hpp
