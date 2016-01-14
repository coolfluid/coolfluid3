// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_actions_Proto_EigenTransforms_hpp
#define cf3_solver_actions_Proto_EigenTransforms_hpp

#include <boost/type_traits/is_reference.hpp>

#include <boost/mpl/equal_to.hpp>
#include <boost/proto/core.hpp>

#include "IndexLooping.hpp"
#include "Partial.hpp"
#include "Terminals.hpp"
#include "Transforms.hpp"

#include "math/MatrixTypes.hpp"

/// @file EigenTransforms.hpp
/// @brief Transforms related to Eigen matrix library functionality

namespace cf3 {
namespace solver {
namespace actions {
namespace Proto {

/// Evaluate the product LeftT * RightT
struct EigenProductEval : boost::proto::callable
{
  template<typename Signature> using result = generic_result<Signature>;

  template<typename ExprT, typename LeftT, typename RightT>
  auto operator()(const ExprT& expr, const LeftT& left, const RightT& right) const -> decltype((left*right).eval())&
  {
    expr.value.noalias() = left*right;
    return expr.value;
  }

  template<typename ExprT>
  Real& operator()(const ExprT& expr, const Real left, const Real right) const
  {
    expr.value = left*right;
    return expr.value;
  }
};

/// Evaluate the product of partial derivatives in index notation, taking care of the summation rules
struct PartialProductEval : boost::proto::callable
{
  template<typename Signature> using result = generic_result<Signature>;

  // Possible repetition counts
  typedef boost::mpl::int_<0> _0;
  typedef boost::mpl::int_<1> _1;
  typedef boost::mpl::int_<2> _2;

  template<typename LeftICountT, typename RightICountT, typename LeftJCountT, typename RightJCountT, typename LeftT, typename RightT>
  struct result_computer
  {
    static_assert(LeftICountT::value != RightICountT::value && LeftICountT::value == RightICountT::value, "Invalid index repetition in product");
  };

  template<typename LeftT, typename RightT>
  struct result_computer<_1, _1, _1, _1, LeftT, RightT>
  {
    typedef Real type;
  };

  template<typename LeftT, typename RightT>
  struct result_computer<_1, _1, _0, _0, LeftT, RightT>
  {
    typedef Real type;
  };

  template<typename LeftT, typename RightT>
  struct result_computer<_0, _0, _1, _1, LeftT, RightT>
  {
    typedef Real type;
  };

  template<typename LeftT, typename RightT>
  struct result_computer<_2, _0, _0, _2, LeftT, RightT>
  {
    typedef Real type;
  };

  template<typename LeftT, typename RightT>
  struct result_computer<_0, _2, _2, _0, LeftT, RightT>
  {
    typedef Real type;
  };

  template<typename ExprT, typename LeftT, typename RightT>
  auto operator()(const ExprT& expr, const LeftT& left, const RightT& right) const ->
  typename result_computer
  <
    decltype(count_repeating_index<0>(boost::proto::left(expr))),
    decltype(count_repeating_index<0>(boost::proto::right(expr))),
    decltype(count_repeating_index<1>(boost::proto::left(expr))),
    decltype(count_repeating_index<1>(boost::proto::right(expr))),
    LeftT,
    RightT
  >::type&
  {
    return eval_summation
    (
      count_repeating_index<0>(boost::proto::left(expr)),
      count_repeating_index<0>(boost::proto::right(expr)),
      count_repeating_index<1>(boost::proto::left(expr)),
      count_repeating_index<1>(boost::proto::right(expr)),
      expr.value,
      left,
      right
    );
  }

  template<typename ExprT>
  Real& operator()(const ExprT& expr, const Real left, const Real right) const
  {
    expr.value = left*right;
    return expr.value;
  }

  // Fallback for invalid cases
  template<typename LeftICountT, typename RightICountT, typename LeftJCountT, typename RightJCountT, typename StoredT, typename LeftT, typename RightT>
  void eval_summation(LeftICountT, RightICountT, LeftJCountT, RightJCountT, StoredT&, const LeftT&, const RightT&) const
  {
    static_assert(LeftICountT::value != RightICountT::value && LeftICountT::value == RightICountT::value, "Invalid index repetition in product");
  }

  template<typename StoredT, typename LeftT, typename RightT>
  Real& eval_summation(_1, _1, _1, _1, StoredT& stored, const LeftT& left, const RightT& right) const
  {
    stored = (left.array() * right.array()).sum();
    return stored;
  }

  template<typename StoredT, typename LeftT, typename RightT>
  Real& eval_summation(_1, _1, _0, _0, StoredT& stored, const LeftT& left, const RightT& right) const
  {
    stored = (left.array() * right.array()).sum();
    return stored;
  }

  template<typename StoredT, typename LeftT, typename RightT>
  Real& eval_summation(_0, _0, _1, _1, StoredT& stored, const LeftT& left, const RightT& right) const
  {
    stored = (left.array() * right.array()).sum();
    return stored;
  }

};

/// Evaluate A += B*C, which is a special case needed in Gauss integrator. If B or C is a scalar, it's also faster
struct EigenPlusAssignProductEval : boost::proto::callable
{
  typedef void result_type;

  template<typename LHST, typename LMatT, typename RMatT>
  void operator()(LHST& lhs, const LMatT& left_mat, const RMatT& right_mat)
  {
    typedef typename std::remove_const<LHST>::type non_const_lhs_t;
    const_cast<non_const_lhs_t&>(lhs).noalias() += left_mat * right_mat;
  }

  template<typename LMatT, typename RMatT>
  void operator()(Real& lhs, const LMatT& left_mat, const RMatT& right_mat)
  {
    lhs += left_mat * right_mat;
  }
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
      boost::proto::multiplies<PartialExpressions, PartialExpressions>,
      PartialProductEval(boost::proto::_expr, PartialExpressions(boost::proto::_left), PartialExpressions(boost::proto::_right))
    >,
    boost::proto::when
    <
      boost::proto::multiplies<GrammarT, GrammarT>,
      EigenProductEval(boost::proto::_expr, GrammarT(boost::proto::_left), GrammarT(boost::proto::_right))
    >,
    boost::proto::when
    <
      boost::proto::plus_assign< GrammarT, boost::proto::multiplies<GrammarT, GrammarT> >,
      EigenPlusAssignProductEval(GrammarT(boost::proto::_left), GrammarT(boost::proto::_left(boost::proto::_right)), GrammarT(boost::proto::_right(boost::proto::_right))) // EigenPlusAssignProductEvalOld<GrammarT>
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

/// Terminal to indicate we want a diagonal
struct ExtractDiagTag
{
};

boost::proto::terminal<ExtractDiagTag>::type const diagonal = {{}};

/// Primitive transform to perform the transpose
struct ExtractDiag :
  boost::proto::transform< ExtractDiag >
{
  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  {
    typedef typename boost::remove_reference<ExprT>::type MatrixT;

    typedef Eigen::Diagonal<MatrixT> result_type;

    result_type operator ()(typename impl::expr_param expr, typename impl::state_param state, typename impl::data_param data) const
    {
      return expr.diagonal();
    }
  };
};

/// Primitive transform to access matrix elements using operator()
struct MatrixElementAccess : boost::proto::callable
{
  typedef Real result_type;

  template<typename MatrixT>
  result_type operator ()(const MatrixT& mat, const Uint i, const Uint j) const
  {
    return mat(i, j);
  }
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
    typedef typename boost::remove_reference<typename boost::tr1_result_of<GrammarT(LeftExprT, StateT, DataT)>::type>::type LeftT;
    typedef typename boost::proto::result_of::right<ExprT>::type IdxExprT;

    // True if the passed expression for the index is a looping index
    typedef boost::proto::matches< IdxExprT, boost::proto::terminal< IndexTag<boost::proto::_> > > IsLoopingIdxT;

    static const bool is_vector = LeftT::IsVectorAtCompileTime;
    typedef typename boost::mpl::if_<typename boost::is_const<LeftT>::type, Real, Real&>::type ScalarTypeT;
    typedef typename boost::mpl::if_c<is_vector, ScalarTypeT, typename LeftT::ConstRowXpr>::type subscript_result_type;
    typedef typename boost::mpl::and_<IsLoopingIdxT, boost::mpl::bool_<boost::remove_reference<DataT>::type::dimension == 1 && (LeftT::MaxRowsAtCompileTime > 1 || LeftT::MaxColsAtCompileTime > 1)> >::type IgnoreLoopingT;
    typedef typename boost::mpl::if_
    <
      IgnoreLoopingT,
      typename boost::tr1_result_of<GrammarT(LeftExprT, StateT, DataT)>::type,
      subscript_result_type
    >::type result_type;

    template<typename MatrixT>
    struct MatrixRef
    {
      typedef MatrixT& type;
    };

    template<typename MatrixT>
    struct MatrixRef< Eigen::Transpose<MatrixT> >
    {
      typedef const Eigen::Transpose<MatrixT> type;
    };

    /// Static dispatch through 2 versions of do_eval, in order to avoid compile errors
    template<typename MatrixT, typename IndexT>
    inline ScalarTypeT do_eval(boost::mpl::true_, typename MatrixRef<MatrixT>::type matrix, const IndexT idx) const // used for vectors
    {
      return matrix[idx];
    }

    template<typename MatrixT, typename IndexT>
    inline typename LeftT::ConstRowXpr do_eval(boost::mpl::false_, typename MatrixRef<MatrixT>::type matrix, const IndexT idx) const // used for matrices
    {
      return matrix.row(idx);
    }

    template<typename MatrixT, typename IndexT>
    inline result_type apply(boost::mpl::false_, typename MatrixRef<MatrixT>::type matrix, const IndexT idx) const
    {
      return do_eval<MatrixT>(boost::mpl::bool_<is_vector>(), matrix, idx);
    }

    template<typename MatrixT, typename IndexT>
    inline result_type apply(boost::mpl::true_, typename MatrixRef<MatrixT>::type matrix, const IndexT idx) const
    {
      return matrix;
    }

    result_type operator ()(typename impl::expr_param expr, typename impl::state_param state, typename impl::data_param data) const
    {
      // go through overloaded do_eval to get the correct expression, depending on if we are subscripting a vector or a matrix
      return apply<LeftT>(IgnoreLoopingT(), GrammarT()(boost::proto::left(expr), state, data), IntegersT()(boost::proto::right(expr), state, data));
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

    typedef typename ExprValT::ConstRowXpr result_type;

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

struct SetIdentity :
boost::proto::transform< SetIdentity >
{
  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  {
    typedef void result_type;

    template<typename MatrixT>
    result_type operator ()(MatrixT& expr, typename impl::state_param, typename impl::data_param) const
    {
      expr.setIdentity();
    }
  };

  template<typename MatT, int R, int C, typename StateT, typename DataT>
  struct impl<Eigen::Block<MatT, R, C>, StateT, DataT> : boost::proto::transform_impl<Eigen::Block<MatT, R, C>, StateT, DataT>
  {
    typedef void result_type;

    result_type operator ()(Eigen::Block<MatT, R, C> expr, typename impl::state_param, typename impl::data_param) const
    {
      expr.setIdentity();
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

struct LumpTag
{
};

static boost::proto::terminal<LumpTag>::type const lump = {};

/// Lump the matrix
struct Lump :
  boost::proto::transform< Lump >
{
  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  {
    typedef void result_type;

    template<typename MatrixT>
    result_type operator ()(const MatrixT& mat, typename impl::state_param, typename impl::data_param) const
    {
      for(Uint i = 0; i != MatrixT::RowsAtCompileTime; ++i)
      {
        const Real rowsum = mat.row(i).sum();
        const_cast<MatrixT&>(mat).row(i).setZero();
        const_cast<MatrixT&>(mat)(i,i) = rowsum;
      }
    }
  };
};

struct ApplyWeightTag
{
};

/// Apply a weight coefficient-wise: apply_weight(matrix_to_modify, weights)
static boost::proto::terminal<ApplyWeightTag>::type const apply_weight = {};

/// Lump the matrix
struct ApplyWeight : boost::proto::callable
{
  typedef void result_type;

  template<typename MatrixT, typename WeightsT>
  void operator()(MatrixT& mat, const WeightsT& weights)
  {
    mat.array() *= weights.array();
  }

  template<typename MatrixT, typename WeightsT>
  void operator()(MatrixT& mat, const WeightsT& weights, const Real threshold)
  {
    mat.array() *= (weights.array() > threshold).template cast<Real>();
  }
};


/// Indexing into Eigen expressions
template<typename GrammarT, typename IntegersT>
struct EigenIndexing :
  boost::proto::or_
  <
    boost::proto::when
    <
      boost::proto::function< GrammarT, IntegersT, IntegersT >,
      MatrixElementAccess( GrammarT(boost::proto::_child0), IntegersT(boost::proto::_child1), IntegersT(boost::proto::_child2) )
    >,
    boost::proto::when
    <
      boost::proto::function< boost::proto::terminal<RowTag>, GrammarT, IntegersT >,
      MatrixRowAccess( GrammarT(boost::proto::_child1), IntegersT(boost::proto::_child2) )
    >,
    boost::proto::when
    <
      boost::proto::function< boost::proto::terminal<ColTag>, boost::proto::_, IntegersT >,
      MatrixColAccess( GrammarT(boost::proto::_child1), IntegersT(boost::proto::_child2) )
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
    PartialExpressions,
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
    boost::proto::when
    <
      boost::proto::assign<GrammarT, boost::proto::terminal<IdentityTag> >,
      SetIdentity(GrammarT(boost::proto::_left))
    >,
    boost::proto::or_
    <
        // Norm calculation
        boost::proto::when
        <
          boost::proto::function<boost::proto::terminal<NormTag>, boost::proto::_>,
          ComputeNorm(GrammarT(boost::proto::_right))
        >,
        // Row-sum lumping
        boost::proto::when
        <
          boost::proto::function<boost::proto::terminal<LumpTag>, boost::proto::_>,
          Lump(GrammarT(boost::proto::_right))
        >,
        // Diagonal extraction
        boost::proto::when
        <
          boost::proto::function<boost::proto::terminal<ExtractDiagTag>, boost::proto::_>,
          ExtractDiag(GrammarT(boost::proto::_right))
        >,
        // Coefficient-wise weighing
        boost::proto::when
        <
          boost::proto::function<boost::proto::terminal<ApplyWeightTag>, boost::proto::_, boost::proto::_>,
          ApplyWeight(GrammarT(boost::proto::_child1), GrammarT(boost::proto::_child2))
        >,
        boost::proto::when
        <
          boost::proto::function<boost::proto::terminal<ApplyWeightTag>, boost::proto::_, boost::proto::_, boost::proto::_>,
          ApplyWeight(GrammarT(boost::proto::_child1), GrammarT(boost::proto::_child2), GrammarT(boost::proto::_child3))
        >
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
