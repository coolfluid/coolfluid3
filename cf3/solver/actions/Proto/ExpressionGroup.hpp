// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_actions_Proto_ExpressionGroup_hpp
#define cf3_solver_actions_Proto_ExpressionGroup_hpp

#include <boost/mpl/for_each.hpp>
#include <boost/mpl/range_c.hpp>

#include <boost/proto/core.hpp>

/// @file
/// Grammar and transform to make grouping expressions possible
/// @author Bart Janssens

namespace cf3 {
namespace solver {
namespace actions {
namespace Proto {

/// Primitive transform to evaluate a group of expressions
template<typename GrammarT>
struct ExpressionGroup :
  boost::proto::transform< ExpressionGroup<GrammarT> >
{
  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  {
    typedef void result_type;

    struct evaluate_expr
    {
      evaluate_expr(typename impl::expr_param expr, typename impl::state_param state, typename impl::data_param data) :
        m_expr(expr),
        m_state(state),
        m_data(data)
      {
      }

      template<typename I>
      void operator()(const I&) const
      {
        GrammarT()(boost::proto::child_c<I::value>(m_expr), m_state, m_data);
      }

      typename impl::expr_param m_expr;
      typename impl::state_param m_state;
      typename impl::data_param m_data;
    };

    void operator()(typename impl::expr_param expr, typename impl::state_param state, typename impl::data_param data) const
    {
      boost::mpl::for_each< boost::mpl::range_c<int, 1, boost::proto::arity_of<ExprT>::value> >
      (
        evaluate_expr(expr, state, data)
      );
    }
  };
};

/// Tags a terminal that triggers expression grouping
struct ExpressionGroupTag {};

/// Use group(expr1, expr2, ..., exprN) to evaluate a group of expressions.
static boost::proto::terminal< ExpressionGroupTag >::type group = {};

/// Matches and evaluates groups of expressions matching GrammarT
template<typename GrammarT>
struct GroupGrammar :
  boost::proto::or_
  <
    boost::proto::when
    <
      boost::proto::function< boost::proto::terminal<ExpressionGroupTag>, boost::proto::vararg< boost::proto::_ > >,
      ExpressionGroup< GroupGrammar<GrammarT> >
    >,
    GrammarT
  >
{
};


} // Proto
} // actions
} // solver
} // cf3

#endif // cf3_solver_actions_Proto_ExpressionGroup_hpp
