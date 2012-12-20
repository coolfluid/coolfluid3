// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_actions_Proto_IndexLooping_hpp
#define cf3_solver_actions_Proto_IndexLooping_hpp

#include <boost/proto/core.hpp>

/// @file
/// Loop over indices that range over the dimension of the problem

namespace cf3 {
namespace solver {
namespace actions {
namespace Proto {

/// Tags a gauss-quadrature loop for the following expressions
struct ElementQuadratureTag {};

/// Match quadrature expressions. THese call the grouping separately, so don't trigger on indices there.
struct ElementQuadratureMatch :
  boost::proto::or_
  <
    boost::proto::function< boost::proto::terminal<ElementQuadratureTag>, boost::proto::_ >,
    boost::proto::shift_left< boost::proto::terminal<ElementQuadratureTag>, boost::proto::comma<boost::proto::_, boost::proto::_> >
  >
{
};

/// Tag terminals used as index
template<typename T>
struct IndexTag
{
};

/// Index looping over the dimensions of a variable
static boost::proto::terminal< IndexTag<boost::mpl::int_<0> > >::type const _i = {};
/// Index looping over the dimensions of a variable
static boost::proto::terminal< IndexTag<boost::mpl::int_<1> > >::type const _j = {};

/// Check if index I is used in the expression
template<Uint I>
struct HasIdx :
  boost::proto::or_
  <
    boost::proto::when
    <
      boost::proto::terminal< IndexTag< boost::mpl::int_<I> > >,
      boost::mpl::true_()
    >,
    boost::proto::when
    <
      boost::proto::terminal< boost::proto::_ >,
      boost::mpl::false_()
    >,
    boost::proto::when
    <
      ElementQuadratureMatch,
      boost::mpl::false_()
    >,
    boost::proto::when
    <
      boost::proto::nary_expr<boost::proto::_, boost::proto::vararg<boost::proto::_> >,
      boost::proto::fold< boost::proto::_, boost::mpl::false_(), boost::mpl::max< boost::proto::_state, boost::proto::call< HasIdx<I> > >() >
    >
  >
{
};

/// Evaluate the value of both indices to the integral constants given in the template arguments
template<typename I, typename J>
struct IndexValues :
  boost::proto::or_
  <
    boost::proto::when
    <
      boost::proto::terminal< IndexTag< boost::mpl::int_<0> > >,
      I()
    >,
    boost::proto::when
    <
      boost::proto::terminal< IndexTag< boost::mpl::int_<1> > >,
      J()
    >
  >
{
};

template<template<typename, typename> class GrammarT>
struct IndexLooper : boost::proto::transform< IndexLooper<GrammarT> >
{
  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  {
    /// True if index _i is used
    typedef typename boost::result_of<HasIdx<0>(ExprT)>::type HasIT;
    /// True if index _j is used
    typedef typename boost::result_of<HasIdx<1>(ExprT)>::type HasJT;

    /// Dimension of the problem
    typedef boost::mpl::int_<boost::remove_reference<DataT>::type::dimension> DimensionT;
    /// Number iterations over _i
    typedef typename boost::mpl::if_< HasIT, DimensionT, boost::mpl::int_<1> >::type IterationsIT;
    /// Number iterations over _j
    typedef typename boost::mpl::if_< HasJT, DimensionT, boost::mpl::int_<1> >::type IterationsJT;

    template<Uint NI, Uint NJ, Uint Dummy = 0>
    struct OuterLoop
    {
      typedef void result_type;

      void operator ()(typename impl::expr_param expr, typename impl::state_param state, typename impl::data_param data) const
      {
        boost::mpl::for_each< boost::mpl::range_c<int, 0, NI> >(InnerLoop<NJ>(expr, state, data));
      }
    };

    template<Uint Dummy>
    struct OuterLoop<1, 1, Dummy>
    {
      typedef GrammarT< boost::mpl::int_<0>, boost::mpl::int_<0> > ConcreteGrammarT;
      typedef typename boost::result_of<ConcreteGrammarT
      (
        typename impl::expr_param, typename impl::state_param, typename impl::data_param
      )>::type result_type;

      result_type operator ()(typename impl::expr_param expr, typename impl::state_param state, typename impl::data_param data) const
      {
        return ConcreteGrammarT()(expr, state, data);
      }
    };

    typedef typename OuterLoop<IterationsIT::value, IterationsJT::value>::result_type result_type;

    result_type operator ()(typename impl::expr_param expr, typename impl::state_param state, typename impl::data_param data) const
    {
      return OuterLoop<IterationsIT::value, IterationsJT::value>()(expr, state, data);
    }

    template<Uint NJ>
    struct InnerLoop
    {
      InnerLoop(typename impl::expr_param expr, typename impl::state_param state, typename impl::data_param data) :
        m_expr(expr),
        m_state(state),
        m_data(data)
      {
      }

      template<typename I>
      void operator()(const I&) const
      {
        boost::mpl::for_each< boost::mpl::range_c<int, 0, NJ> >(EvalExpr<I>(m_expr, m_state, m_data));
      }

      typename impl::expr_param m_expr;
      typename impl::state_param m_state;
      typename impl::data_param m_data;
    };

    template<typename I>
    struct EvalExpr
    {
      EvalExpr(typename impl::expr_param expr, typename impl::state_param state, typename impl::data_param data) :
        m_expr(expr),
        m_state(state),
        m_data(data)
      {
      }

      template<typename J>
      void operator()(const J&) const
      {
        GrammarT<I, J>()(m_expr, m_state, m_data);
      }

      typename impl::expr_param m_expr;
      typename impl::state_param m_state;
      typename impl::data_param m_data;
    };
  };
};

} // namespace Proto
} // namespace actions
} // namespace solver
} // namespace cf3

#endif // cf3_solver_actions_Proto_IndexLooping_hpp
