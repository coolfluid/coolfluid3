// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_Actions_Proto_ForEachDimension_hpp
#define CF_Solver_Actions_Proto_ForEachDimension_hpp

#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include <boost/fusion/include/for_each.hpp>

#include <boost/proto/proto.hpp>

#include "Common/CF.hpp"

/// @file
/// Grammar and transform to make grouping expressions possible

namespace CF {
namespace Solver {
namespace Actions {
namespace Proto {

/// Tags terminals that will be replaced with the current dimension
struct DimensionIdxTag {};
static boost::proto::terminal< DimensionIdxTag >::type _dim = {};

/// Replaces dimension placeholder with state
struct ExpandDimension :
  boost::proto::or_<
    boost::proto::when
    <
      boost::proto::terminal<DimensionIdxTag>,
      boost::proto::_make_terminal(boost::proto::_state)
    >,
    boost::proto::nary_expr< boost::proto::_, boost::proto::vararg<ExpandDimension> >
   >
{};

template<typename GrammarT, typename ExprT, typename DataT>
inline void eval_expr(const ExprT& expr, DataT& data)
{
  GrammarT()(expr, 0, data);
}

/// Primitive transform to evaluate a group of expressions
template<typename GrammarT>
struct ForEachDimension :
  boost::proto::transform< ForEachDimension<GrammarT> >
{
  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  {
    /// Dimension of the problem
    static const Uint dims = boost::remove_reference<DataT>::type::SupportT::ShapeFunctionT::dimension;
    
    typedef void result_type;
    
    void operator ()(
                typename impl::expr_param expr
              , typename impl::state_param state
              , typename impl::data_param data
    ) const
    {
      for(Uint i = 0; i != dims; ++i)
      {
        eval_expr<GrammarT>(ExpandDimension()(expr, i), data); // Replace occurences of dim, and call the grammar given in the template argument
      }
    }
  };
};
/// Tags a terminal that indicates that indicates the argument should be evaluated for each dimension index
struct ForEachDimensionTag {};

/// Use for_each_dimension(expression) to execute expression for each dimension separately
static boost::proto::terminal< ForEachDimensionTag >::type for_each_dimension = {};

/// Matches and evaluates groups of expressions matching GrammarT
template<typename GrammarT>
struct ForEachDimensionGrammar :
  boost::proto::when
  <
    boost::proto::function< boost::proto::terminal<ForEachDimensionTag>, GrammarT >,
    boost::proto::call< ForEachDimension<GrammarT> >(boost::proto::_child1)
  >
{
};
  
} // Proto
} // Actions
} // Solver
} // CF

#endif // CF_Solver_Actions_Proto_ForEachDimension_hpp
