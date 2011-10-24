// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_actions_Proto_ExpressionGroup_hpp
#define cf3_solver_actions_Proto_ExpressionGroup_hpp

#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include <boost/fusion/include/for_each.hpp>

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

    /// Fusion functor to evaluate each child expression using the GrammarT supplied in the template argument
    struct evaluate_expr
    {
      evaluate_expr(typename impl::state_param state, typename impl::data_param data) :
        m_state(state),
        m_data(data)
      {
      }

      template<typename ChildExprT>
      void operator()(ChildExprT& expr) const
      {
        GrammarT()(expr, m_state, m_data);
      }

    private:
      typename impl::state_param  m_state;
      typename impl::data_param m_data;
    };

    void operator ()(
                typename impl::expr_param expr
              , typename impl::state_param state
              , typename impl::data_param data
    ) const
    {
      boost::fusion::for_each(boost::proto::flatten(boost::proto::right(expr)), evaluate_expr(state, data) );
    }
  };
};

/// Tags a terminal that triggers expression grouping
struct ExpressionGroupTag {};

/// Use group << (expr1, expr2, ..., exprN) to evaluate a group of expressions.
/// State can also be set for the expressions, using group(state) << (expr1, expr2, ..., exprN)
static boost::proto::terminal< ExpressionGroupTag >::type group = {};

/// Matches and evaluates groups of expressions matching GrammarT
template<typename GrammarT>
struct GroupGrammar :
  boost::proto::or_
  <
    boost::proto::when
    <
      boost::proto::shift_left< boost::proto::terminal<ExpressionGroupTag>, boost::proto::comma<boost::proto::_, boost::proto::_> >,
      boost::proto::call< ExpressionGroup<GrammarT> >
    >,
    boost::proto::when // This variant allows passing a state as parameter
    <
      boost::proto::shift_left< boost::proto::function< boost::proto::terminal<ExpressionGroupTag>, boost::proto::terminal<boost::proto::_> >, boost::proto::comma<boost::proto::_, boost::proto::_> >,
      boost::proto::call< ExpressionGroup<GrammarT> >(boost::proto::_expr, boost::proto::_value(boost::proto::_child1(boost::proto::_left)))
    >
  >
{
};


} // Proto
} // actions
} // solver
} // cf3

#endif // cf3_solver_actions_Proto_ExpressionGroup_hpp
