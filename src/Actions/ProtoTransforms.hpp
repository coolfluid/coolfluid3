// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Actions_ProtoTransforms_hpp
#define CF_Actions_ProtoTransforms_hpp

#include <boost/mpl/max.hpp>
#include <boost/mpl/void.hpp>

#include "Actions/ProtoVariables.hpp"

#include "Math/RealVector.hpp"

namespace CF {
namespace Actions {


template<typename T>
struct VarArity
{
  typedef typename T::index_type type;
};

/// Gets the arity (max index) for the numbered variables
struct ExprVarArity
  : boost::proto::or_<
        boost::proto::when< boost::proto::terminal< Var<boost::proto::_, boost::proto::_> >,
          boost::mpl::next< VarArity<boost::proto::_value> >()
        >
      , boost::proto::when< boost::proto::terminal<boost::proto::_>,
          boost::mpl::int_<0>()
        >
      , boost::proto::when<
            boost::proto::nary_expr<boost::proto::_, boost::proto::vararg<boost::proto::_> >
          , boost::proto::fold<boost::proto::_, boost::mpl::int_<0>(), boost::mpl::max<ExprVarArity, boost::proto::_state>()>
        >
    >
{};

/// Transform that extracts the type of variable I
template<Uint I>
struct DefineType
  : boost::proto::or_<
      boost::proto::when< boost::proto::terminal< Var<boost::mpl::int_<I>, boost::proto::_> >,
          boost::proto::_value
      >
    , boost::proto::when< boost::proto::terminal< boost::proto::_ >,
          boost::mpl::void_()
      >
    , boost::proto::when< boost::proto::nary_expr<boost::proto::_, boost::proto::vararg<boost::proto::_> >
            , boost::proto::fold<boost::proto::_, boost::mpl::void_(), boost::mpl::if_<boost::mpl::is_void_<boost::proto::_state>, DefineType<I>, boost::proto::_state>() >
      >
  >
{};

/// Ease application of DefineType as an MPL lambda expression, and strip the Var encapsulation
template<typename I, typename Expr>
struct DefineTypeOp
{
  typedef typename boost::result_of<DefineType<I::value>(Expr)>::type var_type;
  typedef typename boost::mpl::if_<boost::mpl::is_void_<var_type>, boost::mpl::void_, typename var_type::type>::type type;
};

/// Copy the terminal values to a fusion list
template<typename VarsT>
struct CopyNumberedVars
  : boost::proto::callable_context< CopyNumberedVars<VarsT>, boost::proto::null_context >
{
  typedef void result_type;
  
  CopyNumberedVars(VarsT& vars) : m_vars(vars) {}

  template<typename I, typename T>
  void operator()(boost::proto::tag::terminal, Var<I, T> val)
  {
    boost::fusion::at<I>(m_vars) = val;
  }
  
private:
  VarsT& m_vars;  
};

/// Strip the CF expression templates
template<typename T1, typename T2=boost::mpl::void_, typename T3=boost::mpl::void_>
struct StripExpr
{
  typedef T1 type;
};

template<typename T1, typename T2>
struct StripExpr< Math::Mult<RealVector, T1, T2> >
{
  typedef RealVector type;
};

template<typename T1, typename T2>
struct StripExpr< Math::Mult<T1, RealVector, T2> >
{
  typedef RealVector type;
};

} // namespace Actions
} // namespace CF

#endif // CF_Actions_ProtoTransforms_hpp
