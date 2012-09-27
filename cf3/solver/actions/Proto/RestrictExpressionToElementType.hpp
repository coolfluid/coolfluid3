// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_actions_Proto_RestrictToElementType_hpp
#define cf3_solver_actions_Proto_RestrictToElementType_hpp

#include <boost/mpl/contains.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/range_c.hpp>

#include <boost/proto/core.hpp>

#include "ExpressionGroup.hpp"

/// @file
/// Grammar and transform to restrict certain expressions to run only over a given set of element types
/// @author Bart Janssens

namespace cf3 {
namespace solver {
namespace actions {
namespace Proto {

/// Primitive transform to evaluate a group of expressions for specific element types only
template<typename GrammarT>
struct RestrictToElementType :
  boost::proto::transform< RestrictToElementType<GrammarT> >
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

    /// Executed only for the listed element types
    void apply(boost::mpl::true_, typename impl::expr_param expr, typename impl::state_param state, typename impl::data_param data) const
    {
      boost::mpl::for_each< boost::mpl::range_c<int, 1, boost::proto::arity_of<ExprT>::value> >
      (
        evaluate_expr(expr, state, data)
      );
    }

    /// Do nothing if the element supprot type is not in the required list
    void apply(boost::mpl::false_, typename impl::expr_param expr, typename impl::state_param state, typename impl::data_param data) const
    {
    }

    void operator()(typename impl::expr_param expr, typename impl::state_param state, typename impl::data_param data) const
    {
      typedef typename boost::remove_reference<typename boost::proto::result_of::value<typename boost::proto::result_of::child_c<ExprT, 0>::type>::type>::type::AllowedSupportTypesT AllowedSupportTypesT;
      apply(typename boost::mpl::contains<AllowedSupportTypesT, typename boost::remove_reference<DataT>::type::SupportShapeFunction>::type(), expr, state, data);
    }
  };
};

/// Tags a terminal that triggers restriction to element types. T is assuled to be an MPL sequence of types for which
/// the sub-expressions must be run
template<typename T>
struct RestrictToElementTypeTag
{
  typedef T AllowedSupportTypesT;
};

/// Matches and evaluates groups of expressions matching GrammarT, optionally restricting to certain element types
template<typename GrammarT>
struct RestrictToElementTypeGrammarSingle :
  boost::proto::or_
  <
    boost::proto::when
    <
      boost::proto::function< boost::proto::terminal< RestrictToElementTypeTag<boost::proto::_> >, boost::proto::vararg< boost::proto::_ > >,
      RestrictToElementType< GrammarT >
    >,
    GrammarT
  >
{
};

template<typename GrammarT>
struct RestrictToElementTypeGrammar :
  boost::proto::or_
  <
    RestrictToElementTypeGrammarSingle<GrammarT>,
    GroupGrammar< RestrictToElementTypeGrammarSingle<GrammarT> >
  >
{
};


} // Proto
} // actions
} // solver
} // cf3

#endif // cf3_solver_actions_Proto_RestrictToElementType_hpp
