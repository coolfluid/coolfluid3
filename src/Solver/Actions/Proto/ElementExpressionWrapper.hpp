// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_Actions_Proto_ElementExpressionWrapper_hpp
#define CF_Solver_Actions_Proto_ElementExpressionWrapper_hpp

#include "ElementGrammar.hpp"

namespace CF {
namespace Solver {
namespace Actions {
namespace Proto {

/// Matches expressions that can be wrapped
struct WrappableElementExpressions :
  boost::proto::or_
  <
    boost::proto::multiplies<boost::proto::_, boost::proto::_>,
    boost::proto::function< boost::proto::terminal< IntegralTag<boost::proto::_> >, boost::proto::_ >,
    boost::proto::function<boost::proto::terminal<LinearizeOp>, boost::proto::_, FieldTypes>,
    boost::proto::function< boost::proto::terminal< SFOp< CustomSFOp<boost::proto::_> > >, boost::proto::_ >
  >
{
};

/// Less restricitve grammar to get the result of expressions that are in an integral as well
struct LazyElementGrammar :
  boost::proto::or_
  <
    ElementGrammar,
    ElementMathImplicit // Normally this is only legal inside an integral
  >
{
};

/// Wraps a given expression, so the value that it represents can be stored inside the expression itself
template<typename ExprT, typename MatrixT>
struct StoredMatrixExpression :
  boost::proto::extends< ExprT, StoredMatrixExpression<ExprT, MatrixT> >
{
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  
  typedef boost::proto::extends< ExprT, StoredMatrixExpression<ExprT, MatrixT> > base_type;
  
  typedef MatrixT ValueT;
  
  explicit StoredMatrixExpression(ExprT const &expr = ExprT())
    : base_type(expr)
  {
  }
  
  /// Temporary storage for the result of the expression
  mutable ValueT value;
};

struct WrapMatrixExpression : boost::proto::transform< WrapMatrixExpression >
{
  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  {
    /// Helper to select if the expression is to be wrapped
    template<bool, int Dummy = 0>
    struct WrapperSelector;
    
    /// Expression must not be wrapped
    template<int Dummy>
    struct WrapperSelector<false, Dummy>
    {
      typedef typename impl::expr_param result_type;
      
      result_type operator()(typename impl::expr_param expr, typename impl::state_param , typename impl::data_param)
      {
        return expr;
      }
    };
    
    /// Expression is to be wrapped
    template<int Dummy>
    struct WrapperSelector<true, Dummy>
    {
      /// Calculate the type to store
      typedef typename boost::remove_const<typename boost::remove_reference
      <
        typename boost::result_of<LazyElementGrammar(typename impl::expr_param, typename impl::state_param, typename impl::data_param)>::type
      >::type>::type ValueT;
      
      typedef StoredMatrixExpression<typename boost::remove_const<typename boost::remove_reference<ExprT>::type>::type, ValueT> result_type;
      
      result_type operator()(typename impl::expr_param expr, typename impl::state_param, typename impl::data_param)
      {
        return result_type(expr);
      }
    };
    
    typedef WrapperSelector< boost::proto::matches<ExprT, WrappableElementExpressions>::value > ResultSelectorT;
    typedef typename ResultSelectorT::result_type result_type;
    
    result_type operator()(typename impl::expr_param expr, typename impl::state_param state, typename impl::data_param data)
    {
      return ResultSelectorT()(expr, state, data);
    }
  };
};

/// Grammar to do the expression wrapping
struct WrapExpression :
  boost::proto::or_
  <
    boost::proto::when
    <
      boost::proto::multiplies<boost::proto::_, boost::proto::_>,
      WrapMatrixExpression(boost::proto::functional::make_multiplies
      (
        WrapExpression(boost::proto::_left), WrapExpression(boost::proto::_right)
      ))
    >,
    boost::proto::when
    <
      boost::proto::function< boost::proto::terminal< IntegralTag<boost::proto::_> >, boost::proto::_ >,
      WrapMatrixExpression(boost::proto::functional::make_function
      (
        WrapExpression(boost::proto::_child0), WrapExpression(boost::proto::_child1)
      ))
    >,
    boost::proto::when
    <
      boost::proto::function<boost::proto::terminal<LinearizeOp>, boost::proto::_, FieldTypes>,
      WrapMatrixExpression(boost::proto::functional::make_function
      (
        WrapExpression(boost::proto::_child0), WrapExpression(boost::proto::_child1), WrapExpression(boost::proto::_child2)
      ))
    >,
    boost::proto::when
    <
      boost::proto::function< boost::proto::terminal< SFOp< CustomSFOp<boost::proto::_> > >, boost::proto::_ >,
      WrapMatrixExpression
    >,
    boost::proto::nary_expr< boost::proto::_, boost::proto::vararg<WrapExpression> >
  >
{};

} // namespace Proto
} // namespace Actions
} // namespace Solver
} // namespace CF

#endif // CF_Solver_Actions_Proto_ElementExpressionWrapper_hpp
