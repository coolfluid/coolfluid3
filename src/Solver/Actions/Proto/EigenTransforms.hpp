// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_Actions_Proto_EigenTransforms_hpp
#define CF_Solver_Actions_Proto_EigenTransforms_hpp

#include <boost/proto/proto.hpp>

#include "Math/MatrixTypes.hpp"

#include "Transforms.hpp"

struct C;
/// @file Transforms related to Eigen matrix library functionality

/// Required extension to eigen, in order to pass matrix product expressions by value
namespace Eigen {

template<typename Lhs, typename Rhs, int Mode> class NestByValue<GeneralProduct<Lhs,Rhs,Mode> >
  : public ProductBase<NestByValue<GeneralProduct<Lhs,Rhs,Mode> >,
                       typename GeneralProduct<Lhs,Rhs,Mode>::_LhsNested,
                       typename GeneralProduct<Lhs,Rhs,Mode>::_RhsNested>
{
public:

  typedef GeneralProduct<Lhs,Rhs,Mode> NestedProduct;
  
  typedef ProductBase<NestByValue,
                      typename NestedProduct::_LhsNested,
                      typename NestedProduct::_RhsNested> Base;
  typedef typename Base::Scalar Scalar;
  typedef typename Base::PlainObject PlainObject;

  NestByValue(const NestedProduct& prod)
    : Base(prod.lhs(),prod.rhs()), m_prod(prod) {}
  
  template<typename Dest>
  inline void evalTo(Dest& dst) const { dst.setZero(); scaleAndAddTo(dst,Scalar(1)); }

  template<typename Dest>
  inline void addTo(Dest& dst) const { scaleAndAddTo(dst,Scalar(1)); }

  template<typename Dest>
  inline void subTo(Dest& dst) const { scaleAndAddTo(dst,-Scalar(1)); }

  template<typename Dest>
  inline void scaleAndAddTo(Dest& dst,Scalar alpha) const { m_prod.derived().scaleAndAddTo(dst,Scalar(1)); }

  operator const NestedProduct&() const { return m_prod; }
    
protected:
  const NestedProduct m_prod;
};

template<typename T>
struct ei_nested< NestByValue<T> >
{
  typedef NestByValue<T> const type;
};

} // namespace Eigen

namespace CF {
namespace Solver {
namespace Actions {
namespace Proto {

/// Converts product type if necessary
template<typename T>
struct FilterProduct
{
  typedef T type;
};

/// Inner products become a 1x1 matrix, but ProductReturnType does not resolve that
template<typename Lhs, typename Rhs>
struct FilterProduct< Eigen::GeneralProduct<Lhs,Rhs,Eigen::InnerProduct> >
{
  typedef Eigen::Matrix<Real, 1, 1> type;
};

/// Object transform to create a product that can be nested by value, avoiding dangling references
template<typename LeftT, typename RightT>
struct EigenProduct
{
  typedef typename Eigen::ProductReturnType<LeftT, RightT>::Type ProductT;
  typedef Eigen::NestByValue<typename FilterProduct<ProductT>::type> type;
};

/// Scalar on the left
template<typename RightT>
struct EigenProduct<Real, RightT>
{
  typedef Eigen::CwiseUnaryOp<Eigen::ei_scalar_multiple_op<Real>, RightT> type;
};

/// Scalar on the right
template<typename LeftT>
struct EigenProduct<LeftT, Real>
{
  typedef Eigen::CwiseUnaryOp<Eigen::ei_scalar_multiple_op<Real>, LeftT> type;
};

/// Scalar - scalar
template<>
struct EigenProduct<Real, Real>
{
  typedef Real type;
};

/// Extract the real value type of a given type, which might be an Eigen expression
template<typename T>
struct ValueType
{
  typedef typename Eigen::MatrixBase<T>::PlainObject type;
  typedef Eigen::NestByValue<type> NestedT;
};

/// Specialize for Eigen matrices
template<int I, int J>
struct ValueType< Eigen::Matrix<Real, I, J> >
{
  typedef Eigen::Matrix<Real, I, J> type;
  typedef Eigen::NestByValue<type> NestedT;
};

/// Specialise for reals
template<>
struct ValueType<Real>
{
  typedef Real type;
  typedef Real NestedT;
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
    typedef Eigen::Transpose<typename boost::remove_const<typename boost::remove_reference<StateT>::type>::type> result_type;
    
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
    typedef typename boost::remove_reference<ExprT>::type ExprValT;
    
    typedef typename boost::mpl::if_c<ExprValT::ColsAtCompileTime == 1, Real, typename ExprValT::RowXpr>::type result_type;
    
    /// Static dispatch through 2 versions of do_eval, in order to avoid compile errors
    inline Real do_eval(boost::mpl::true_, ExprT expr, StateT state) const
    {
      return expr[state];
    }
    
    inline typename ExprValT::RowXpr do_eval(boost::mpl::false_, ExprT expr, StateT state) const
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
    
    typedef typename ExprValT::ColXpr result_type;
    
    result_type operator ()(typename impl::expr_param expr, typename impl::state_param state, typename impl::data_param data) const
    {
      return expr.col(state);
    }
  };
};

/// Handle multiplication cases
template<typename GrammarT>
struct EigenMultiplication :
  boost::proto::when
  <
    boost::proto::multiplies<GrammarT, GrammarT>,
    EigenProduct
    <
      GrammarT(boost::proto::_left),
      GrammarT(boost::proto::_right)
    >(boost::proto::_default<GrammarT>)
  >
{
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

/// Grammar for valid Eigen expressions, composed of primitives matching GrammarT
template<typename GrammarT>
struct EigenMath :
  boost::proto::or_
  <
    EigenMultiplication<GrammarT>,
    // Indexing
    boost::proto::when
    <
      boost::proto::function< GrammarT, Integers, Integers >,
      MatrixElementAccess( boost::proto::_expr, GrammarT(boost::proto::_child0) )
    >,
    boost::proto::when
    <
      boost::proto::function< GrammarT, boost::proto::terminal<RowTag>, Integers >,
      MatrixRowAccess( GrammarT(boost::proto::_child0), Integers(boost::proto::_child2) )
    >,
    boost::proto::when
    <
      boost::proto::function< GrammarT, boost::proto::terminal<ColTag>, Integers >,
      MatrixColAccess( GrammarT(boost::proto::_child0), Integers(boost::proto::_child2) )
    >,
    // Subscripting
    boost::proto::when
    <
      boost::proto::subscript< GrammarT, Integers >,
      MatrixSubscript( GrammarT(boost::proto::_left), Integers(boost::proto::_right) )
    >,
    // Transpose
    boost::proto::when
    <
      boost::proto::function< boost::proto::terminal<TransposeFunction>, GrammarT >,
      TransposeTransform(boost::proto::_expr, GrammarT(boost::proto::_child1), boost::proto::_data)
    >
  >
{
};

} // namespace Proto
} // namespace Actions
} // namespace Solver
} // namespace CF

#endif // CF_Solver_Actions_Proto_EigenTransforms_hpp
