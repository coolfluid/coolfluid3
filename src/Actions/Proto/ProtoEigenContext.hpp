// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Actions_ProtoEigenContext_hpp
#define CF_Actions_ProtoEigenContext_hpp

#include "Actions/Proto/ProtoFunctions.hpp"
#include "Actions/Proto/ProtoTransforms.hpp"
#include "Actions/Proto/ProtoVariables.hpp"

#include "Common/CF.hpp"

/// Required extension to eigen
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
namespace Actions {
namespace Proto {

template<typename ParentContextT>
struct EigenContext
{
  typedef EigenContext<ParentContextT> ThisContextT;
  
  EigenContext(ParentContextT& parent_ctx) : parent_context(parent_ctx) {}
  
  ParentContextT& parent_context;
  
  template<typename Expr,
           typename Tag = typename Expr::proto_tag,
           typename Arg = typename Expr::proto_child0>
  struct eval;
  
  /// Wrap transpose
  template<typename Expr, typename ChildT>
  struct eval<Expr, eigen_function_tag<transpose_tag>, ChildT >
  {
    typedef Eigen::Transpose
    <
      typename boost::remove_const
      <
        typename boost::remove_reference
        <
          typename boost::proto::result_of::eval
          <
            typename boost::proto::result_of::child<Expr>::type, ParentContextT
          >::type
        >::type
      >::type
    > result_type;

    inline result_type operator()(Expr& expr, const ThisContextT& ctx) const
    {
      return boost::proto::eval(boost::proto::child(expr), ctx.parent_context).transpose();
    }
  };
};

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

/// Multiplication of two matrices or vectors
template<typename LeftT, typename RightT>
struct EigenMultiplier
{
  typedef typename Eigen::ProductReturnType<LeftT, RightT>::Type ProductT;
  typedef Eigen::NestByValue<typename FilterProduct<ProductT>::type> result_type;
  inline result_type exec(const LeftT& l, const RightT& r)
  {
    return (l * r).nestByValue();
  }
};

/// Scalar on the left
template<typename RightT>
struct EigenMultiplier<Real, RightT>
{
  typedef Eigen::CwiseUnaryOp<Eigen::ei_scalar_multiple_op<Real>, RightT> result_type;
  inline result_type exec(const Real l, const RightT& r)
  {
    return (l * r);
  }
};

/// Scalar on the right
template<typename LeftT>
struct EigenMultiplier<LeftT, Real>
{
  typedef Eigen::CwiseUnaryOp<Eigen::ei_scalar_multiple_op<Real>, LeftT> result_type;
  inline result_type exec(const LeftT& l, const Real r)
  {
    return (l * r);
  }
};

/// Two scalars
template<>
struct EigenMultiplier<Real, Real>
{
  typedef Real result_type;
  inline result_type exec(const Real l, const Real r)
  {
    return (l * r);
  }
};

} // namespace Proto
} // namespace Actions
} // namespace CF

#endif // CF_Actions_ProtoEigenContext_hpp
