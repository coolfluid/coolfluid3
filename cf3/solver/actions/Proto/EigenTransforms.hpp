// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_actions_Proto_EigenTransforms_hpp
#define cf3_solver_actions_Proto_EigenTransforms_hpp

#include <boost/mpl/equal_to.hpp>
#include <boost/proto/core.hpp>

#include "IndexLooping.hpp"
#include "Terminals.hpp"
#include "Transforms.hpp"

#include "math/MatrixTypes.hpp"

/// @file EigenTransforms.hpp
/// @brief Transforms related to Eigen matrix library functionality

namespace cf3 {
namespace solver {
namespace actions {
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

/// Specialise for transpose of reals
template<>
struct ValueType< Eigen::Transpose<const Real> >
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
    typedef typename boost::remove_reference<typename impl::state_param>::type RealStateT;

    typedef typename boost::mpl::if_< boost::is_same<Real, typename boost::remove_const<RealStateT>::type>, Real, Eigen::Transpose<RealStateT> >::type result_type;

    result_type operator ()(typename impl::expr_param expr, typename impl::state_param state, typename impl::data_param data) const
    {
      return apply(state);
    }

    template<typename T>
    result_type apply(T& mat) const
    {
      return mat.transpose();
    }

    Real apply(const Real val) const
    {
      return val;
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
template<typename GrammarT, typename IntegersT>
struct MatrixSubscript :
  boost::proto::transform< MatrixSubscript<GrammarT, IntegersT> >
{
  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  {
    typedef typename boost::proto::result_of::left<ExprT>::type LeftExprT;
    typedef typename boost::remove_const<typename boost::remove_reference<typename boost::result_of<GrammarT(LeftExprT, StateT, DataT)>::type>::type>::type LeftT;
    typedef typename boost::proto::result_of::right<ExprT>::type IdxExprT;

    // True if the passed expression for the index is a looping index
    typedef boost::proto::matches< IdxExprT, boost::proto::terminal< IndexTag<boost::proto::_> > > IsLoopingIdxT;

    static const bool is_vector = LeftT::ColsAtCompileTime == 1 || LeftT::RowsAtCompileTime == 1;
    typedef typename boost::mpl::if_c<is_vector, Real, typename LeftT::ConstRowXpr>::type subscript_result_type;
    typedef typename boost::mpl::and_<IsLoopingIdxT, boost::mpl::bool_<boost::remove_reference<DataT>::type::dimension == 1> >::type IgnoreLoopingT;
    typedef typename boost::mpl::if_
    <
      IgnoreLoopingT,
      typename boost::result_of<GrammarT(LeftExprT, StateT, DataT)>::type,
      subscript_result_type
    >::type result_type;

    /// Static dispatch through 2 versions of do_eval, in order to avoid compile errors
    template<typename MatrixT, typename IndexT>
    inline Real do_eval(boost::mpl::true_, MatrixT& matrix, const IndexT idx) const // used for vectors
    {
      return matrix[idx];
    }

    template<typename MatrixT, typename IndexT>
    inline typename LeftT::ConstRowXpr do_eval(boost::mpl::false_, MatrixT& matrix, const IndexT idx) const // used for matrices
    {
      return matrix.row(idx);
    }

    template<typename MatrixT, typename IndexT>
    inline result_type apply(boost::mpl::false_, MatrixT& matrix, const IndexT idx) const
    {
      return do_eval(boost::mpl::bool_<is_vector>(), matrix, idx);
    }

    template<typename MatrixT, typename IndexT>
    inline result_type apply(boost::mpl::true_, MatrixT& matrix, const IndexT idx) const
    {
      return matrix;
    }

    result_type operator ()(typename impl::expr_param expr, typename impl::state_param state, typename impl::data_param data) const
    {
      // go through overloaded do_eval to get the correct expression, depending on if we are subscripting a vector or a matrix
      return apply(IgnoreLoopingT(), GrammarT()(boost::proto::left(expr), state, data), IntegersT()(boost::proto::right(expr), state, data));
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

struct NormTag
{
};

static boost::proto::terminal<NormTag>::type const _norm = {};

/// Compute the norm of a matrix or a vector
struct ComputeNorm :
  boost::proto::transform< ComputeNorm >
{
  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  {
    typedef Real result_type;

    template<typename MatrixT>
    result_type operator ()(const MatrixT& expr, typename impl::state_param, typename impl::data_param) const
    {
      return expr.norm();
    }
  };
};

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
      MatrixSubscript<GrammarT, IntegersT>
    >
  >
{
};

template<typename GrammarT>
struct TransposeGrammar :
  boost::proto::when
  <
    boost::proto::function< boost::proto::terminal<TransposeFunction>, GrammarT >,
    TransposeTransform(boost::proto::_expr, GrammarT(boost::proto::_child1), boost::proto::_data)
  >
{
};

/// Grammar for valid Eigen expressions, composed of primitives matching GrammarT
template<typename GrammarT, typename IntegersT>
struct EigenMath :
  boost::proto::or_
  <
    EigenMultiplication< boost::proto::or_<TransposeGrammar<GrammarT>, GrammarT> >,
    // Indexing
    EigenIndexing<GrammarT, IntegersT>,
    // Transpose
    TransposeGrammar<GrammarT>,
    // Setting to zero
    boost::proto::when
    <
      boost::proto::assign<GrammarT, boost::proto::terminal<ZeroTag> >,
      SetZero(GrammarT(boost::proto::_left))
    >,
    // Norm calculation
    boost::proto::when
    <
      boost::proto::function<boost::proto::terminal<NormTag>, boost::proto::_>,
      ComputeNorm(GrammarT(boost::proto::_right))
    >,
    MathOpDefault<GrammarT>
  >
{
};

} // namespace Proto
} // namespace actions
} // namespace solver
} // namespace cf3

#endif // cf3_solver_actions_Proto_EigenTransforms_hpp
