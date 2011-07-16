// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_Actions_Proto_EigenTransforms_hpp
#define CF_Solver_Actions_Proto_EigenTransforms_hpp

#include <boost/proto/core.hpp>

#include "Terminals.hpp"

#include "Math/MatrixTypes.hpp"

/// @file EigenTransforms.hpp 
/// @brief Transforms related to Eigen matrix library functionality

namespace CF {
namespace Solver {
namespace Actions {
namespace Proto {

/// Generalization of Eigen::ProductReturnType that also works for scalars (see specializations below)
template<typename LeftT, typename RightT>
struct EigenProductType
{
  typedef typename Eigen::ProductReturnType<LeftT, RightT>::Type type;
};

/// Scalar on the left
template<typename RightT>
struct EigenProductType<Real, RightT>
{
  typedef Eigen::CwiseUnaryOp<Eigen::internal::scalar_multiple_op<Real>, const RightT> type;
};

/// Scalar on the right
template<typename LeftT>
struct EigenProductType<LeftT, Real>
{
  typedef Eigen::CwiseUnaryOp<Eigen::internal::scalar_multiple_op<Real>, const LeftT> type;
};

/// Scalar - scalar
template<>
struct EigenProductType<Real, Real>
{
  typedef Real type;
};

/// Extract the real value type of a given type, which might be an Eigen expression
template<typename T>
struct ValueType
{
  typedef typename Eigen::MatrixBase<T>::PlainObject type;
};

/// Specialize for Eigen matrices
template<int I, int J>
struct ValueType< Eigen::Matrix<Real, I, J> >
{
  typedef Eigen::Matrix<Real, I, J> type;
};

/// Specialise for multiplication with a scalar
template<typename MatrixT>
struct ValueType< Eigen::CwiseUnaryOp<Eigen::internal::scalar_multiple_op<Real>, const MatrixT> >
{
  typedef typename ValueType<MatrixT>::type type;
};

/// Specialise for reals
template<>
struct ValueType<Real>
{
  typedef Real type;
};

/// Evaluate the product LeftT * RightT
template<typename GrammarT>
struct EigenProductEval :
  boost::proto::transform< EigenProductEval<GrammarT> >
{
  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  {
    typedef typename boost::proto::result_of::left<ExprT>::type LeftExprT;
    typedef typename boost::proto::result_of::right<ExprT>::type RightExprT;
    
    typedef typename boost::result_of<GrammarT(LeftExprT, StateT, DataT)>::type LeftT;
    typedef typename boost::result_of<GrammarT(RightExprT, StateT, DataT)>::type RightT;
    
    typedef typename boost::remove_const<typename boost::remove_reference<LeftT>::type>::type UnRefLeftT;
    typedef typename boost::remove_const<typename boost::remove_reference<RightT>::type>::type UnRefRightT;
    
    typedef const typename ValueType< typename EigenProductType<UnRefLeftT, UnRefRightT>::type >::type& result_type;
    
    result_type operator ()(typename impl::expr_param expr, typename impl::state_param state, typename impl::data_param data) const
    {
      expr.value = GrammarT()(boost::proto::left(expr), state, data) * GrammarT()(boost::proto::right(expr), state, data);
      return expr.value;
    }
  };
};

/// Evaluate A += B*C, which is a special case needed in Gauss integrator. If B or C is a scalar, it's also faster
template<typename GrammarT>
struct EigenPlusAssignProductEval :
  boost::proto::transform< EigenPlusAssignProductEval<GrammarT> >
{
  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  {
    
    
    typedef void result_type;
    
    template<typename T>
    struct LHSHelper
    {
      inline T operator()(T arg) const
      {
        return arg;
      }
    };
    
    template<typename T>
    struct LHSHelper<const T&>
    {
      inline T& operator()(const T& arg) const
      {
        return const_cast<T&>(arg);
      }
    };
    
    result_type operator ()(typename impl::expr_param expr, typename impl::state_param state, typename impl::data_param data) const
    {
      // Result type for the LHS
      typedef typename boost::result_of
      <
        GrammarT(typename boost::proto::result_of::left
        <
          typename impl::expr_param
        >::type, typename impl::state_param, typename impl::data_param)
      >::type LeftT;
      
      LHSHelper<LeftT>()(GrammarT()(boost::proto::left(expr), state, data)) += GrammarT()(boost::proto::left(boost::proto::right(expr)), state, data) * GrammarT()(boost::proto::right(boost::proto::right(expr)), state, data);
    }
  };
};

/// Match matrix terms (only matrix products need special treatment)
template<typename GrammarT>
struct MatrixTerm :
  boost::proto::and_
  <
    boost::proto::not_< boost::proto::terminal<Real> >,
    GrammarT
  >
{
};

/// Handle an expression that is filtered by Eigen
template<typename GrammarT>
struct EigenMultiplication :
  boost::proto::or_
  <
    boost::proto::when
    <
      boost::proto::multiplies<GrammarT, GrammarT>,
      EigenProductEval<GrammarT>
    >,
    boost::proto::when
    <
      boost::proto::plus_assign< GrammarT, boost::proto::multiplies<GrammarT, GrammarT> >,
      EigenPlusAssignProductEval<GrammarT>
    >
  >
{
};

/// Terminal to indicate we want a transpose
struct TransposeFunction
{
};

boost::proto::terminal<TransposeFunction>::type const transpose = {{}};

/// Primitive transform to perform the transpose
struct TransposeTransform :
  boost::proto::transform< TransposeTransform >
{
  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  { 
    typedef Eigen::Transpose<typename boost::remove_reference<typename impl::state_param>::type> result_type;
    
    result_type operator ()(typename impl::expr_param expr, typename impl::state_param state, typename impl::data_param data) const
    {
      return state.transpose();
    }
  };
};

/// Primitive transform to access matrix elements using operator()
struct MatrixElementAccess :
  boost::proto::transform< MatrixElementAccess >
{
  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  { 
    /// We assume our matrices contain Reals.
    typedef Real result_type;
    
    result_type operator ()(typename impl::expr_param expr, typename impl::state_param state, typename impl::data_param data) const
    {
      return state
      (
        boost::proto::value( boost::proto::child_c<1>(expr) ),
        boost::proto::value( boost::proto::child_c<2>(expr) )
      );
    }
  };
};

/// Primitive transform to access matrix elements using operator[]
struct MatrixSubscript :
  boost::proto::transform< MatrixSubscript >
{
  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  {
    typedef typename boost::remove_const<typename boost::remove_reference<ExprT>::type>::type ExprValT;
    
    typedef typename boost::mpl::if_c<ExprValT::ColsAtCompileTime == 1, Real, typename ExprValT::ConstRowXpr>::type result_type;
    
    /// Static dispatch through 2 versions of do_eval, in order to avoid compile errors
    inline Real do_eval(boost::mpl::true_, ExprT expr, StateT state) const
    {
      return expr[state];
    }
    
    inline typename ExprValT::ConstRowXpr do_eval(boost::mpl::false_, ExprT expr, StateT state) const
    {
      return expr.row(state);
    }
    
    result_type operator ()(typename impl::expr_param expr, typename impl::state_param state, typename impl::data_param data) const
    {
      // go through overloaded do_eval to get the correct expression, depending on if we are subscripting a vector or a matrix
      return do_eval(boost::mpl::bool_<ExprValT::ColsAtCompileTime == 1>(), expr, state);
    }
  };
};

struct MatrixRowAccess :
  boost::proto::transform< MatrixRowAccess >
{
  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  {
    typedef typename boost::remove_reference<ExprT>::type ExprValT;
    
    typedef typename ExprValT::RowXpr result_type;
    
    result_type operator ()(typename impl::expr_param expr, typename impl::state_param state, typename impl::data_param data) const
    {
      return expr.row(state);
    }
  };
};

struct MatrixColAccess :
  boost::proto::transform< MatrixColAccess >
{
  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  {
    typedef typename boost::remove_reference<ExprT>::type ExprValT;
    
    typedef typename ExprValT::ConstColXpr result_type;
    
    result_type operator ()(typename impl::expr_param expr, typename impl::state_param state, typename impl::data_param data) const
    {
      return expr.col(state);
    }
  };
};

struct SetZero :
  boost::proto::transform< SetZero >
{
  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  {
    typedef void result_type;
    
    template<typename MatrixT>
    result_type operator ()(MatrixT& expr, typename impl::state_param, typename impl::data_param) const
    {
      expr.setZero();
    }
  };
  
  template<typename MatT, int R, int C, typename StateT, typename DataT>
  struct impl<Eigen::Block<MatT, R, C>, StateT, DataT> : boost::proto::transform_impl<Eigen::Block<MatT, R, C>, StateT, DataT>
  {
    typedef void result_type;
    
    result_type operator ()(Eigen::Block<MatT, R, C> expr, typename impl::state_param, typename impl::data_param) const
    {
      expr.setZero();
    }
  };
};

/// Placeholders to indicate if we should get a row or a column
struct RowTag
{
};

struct ColTag
{
};

static boost::proto::terminal<RowTag>::type _row = {};
static boost::proto::terminal<ColTag>::type _col = {};

/// Indexing into Eigen expressions
template<typename GrammarT, typename IntegersT>
struct EigenIndexing :
  boost::proto::or_
  <
    boost::proto::when
    <
      boost::proto::function< GrammarT, IntegersT, IntegersT >,
      MatrixElementAccess( boost::proto::_expr, GrammarT(boost::proto::_child0) )
    >,
    boost::proto::when
    <
      boost::proto::function< GrammarT, boost::proto::terminal<RowTag>, IntegersT >,
      MatrixRowAccess( GrammarT(boost::proto::_child0), IntegersT(boost::proto::_child2) )
    >,
    boost::proto::when
    <
      boost::proto::function< GrammarT, boost::proto::terminal<ColTag>, IntegersT >,
      MatrixColAccess( GrammarT(boost::proto::_child0), IntegersT(boost::proto::_child2) )
    >,
    // Subscripting
    boost::proto::when
    <
      boost::proto::subscript< GrammarT, IntegersT >,
      MatrixSubscript( GrammarT(boost::proto::_left), IntegersT(boost::proto::_right) )
    >
  >
{
};

/// Grammar for valid Eigen expressions, composed of primitives matching GrammarT
template<typename GrammarT, typename IntegersT>
struct EigenMath :
  boost::proto::or_
  <
    EigenMultiplication<GrammarT>,
    // Indexing
    EigenIndexing<GrammarT, IntegersT>,
    // Transpose
    boost::proto::when
    <
      boost::proto::function< boost::proto::terminal<TransposeFunction>, GrammarT >,
      TransposeTransform(boost::proto::_expr, GrammarT(boost::proto::_child1), boost::proto::_data)
    >,
    // Setting to zero
    boost::proto::when
    <
      boost::proto::assign<GrammarT, boost::proto::terminal<ZeroTag> >,
      SetZero(GrammarT(boost::proto::_left))
    >,
    MathOpDefault<GrammarT>
  >
{
};

} // namespace Proto
} // namespace Actions
} // namespace Solver
} // namespace CF

#endif // CF_Solver_Actions_Proto_EigenTransforms_hpp
