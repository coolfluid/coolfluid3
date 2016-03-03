// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_actions_Proto_ElementIntegration_hpp
#define cf3_solver_actions_Proto_ElementIntegration_hpp

#include <boost/mpl/assert.hpp>
#include <boost/mpl/max_element.hpp>

#include <boost/proto/transform/lazy.hpp>

#include "mesh/Integrators/Gauss.hpp"

#include "ElementMatrix.hpp"
#include "ElementTransforms.hpp"
#include "IndexLooping.hpp"

/// @file
/// Transforms used in element-wise expression evaluation

namespace cf3 {
namespace solver {
namespace actions {
namespace Proto {

namespace detail
{
  // Helper to call eval on reals and matrix types
  template<typename T>
  inline auto do_eval(T&& matexpr) -> decltype(matexpr.eval())
  {
    return matexpr.eval();
  }

  inline Real& do_eval(Real&& r)
  {
    return r;
  }
}

template<typename DataT, typename VarT>
struct GetOrderFromData
{
  static const Uint value = DataT::EtypeT::order;
};

template<typename DataT>
struct GetOrderFromData<DataT, boost::mpl::void_>
{
  static const Uint value = 0;
};

/// Get the order of variable I
template<typename I, typename DataT, typename ExprT>
struct GetOrder
{
  // The type of the data associated with the variable, containing info about the order. Void if the variable doesn't exist
  typedef typename DataT::template DataType<I>::type VarDataT;
  // The type of the variable. Void if the variable doesn't appear in the current subexpression
  typedef typename boost::tr1_result_of<DefineType<I::value>(ExprT)>::type VarT;
  typedef boost::mpl::int_<GetOrderFromData<VarDataT, VarT>::value> type;
};

/// Get the maximum order of the shape functions used in Expr
template<typename ExprT, typename DataT, Uint SupportOrder>
struct MaxOrder
{
  typedef typename boost::tr1_result_of<ExprVarArity(ExprT)>::type NbVarsT;

  typedef typename boost::mpl::deref<typename boost::mpl::max_element
  <
    typename boost::mpl::transform
    <
      typename boost::mpl::copy<boost::mpl::range_c<int,0,NbVarsT::value>, boost::mpl::back_inserter< boost::mpl::vector_c<Uint> > >::type, //range from 0 to NbVarsT
      GetOrder<boost::mpl::_1, DataT, ExprT>
    >::type
  >::type>::type type;

  static constexpr Uint value = boost::mpl::if_<boost::mpl::is_void_<type>, boost::mpl::int_<SupportOrder>, type>::type::value;
};

/// Determine integration order based on the order of the shape function
template<Uint>
struct IntegrationOrder
{
};

template<>
struct IntegrationOrder<1>
{
  static const Uint value = 2;
};

template<>
struct IntegrationOrder<2>
{
  static const Uint value = 4;
};

/// Tag for an integral, wit the order provided as an MPL integral constant
template<typename OrderT>
struct IntegralTag
{
};

template<Uint Order, typename ExprT>
inline typename boost::proto::result_of::make_expr< boost::proto::tag::function, IntegralTag< boost::mpl::int_<Order> >, ExprT const & >::type const
integral(ExprT const & expr)
{
  return boost::proto::make_expr<boost::proto::tag::function>( IntegralTag< boost::mpl::int_<Order> >(), boost::ref(expr) );
}

/// Primitive transform that evaluates an integral using the Gauss points and returns the result as a reference to a stored matrix (or scalar)
struct GaussIntegral :
  boost::proto::transform< GaussIntegral >
{
/// Helper structs
private:
  /// Helper to extract the integration order
  template<typename T>
  struct GetOrder;

  template<int I>
  struct GetOrder< IntegralTag< boost::mpl::int_<I> > >
  {
    static const int value = I;
  };

public:

  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  {
    /// Mapped coordinate type to use is obtained from the support
    typedef typename boost::remove_reference<DataT>::type::SupportT::EtypeT ShapeFunctionT;
    typedef typename ShapeFunctionT::MappedCoordsT MappedCoordsT;

    /// function argument 0 contains the terminal representing the function, and thus also the integration order
    typedef typename boost::remove_const
    <
      typename boost::remove_reference
      <
        typename boost::proto::result_of::value
        <
          typename boost::proto::result_of::child_c<ExprT, 0>::type
        >::type
      >::type
    >::type IntegralTagT;

    // Whew, we got the order
    static const int order = GetOrder<IntegralTagT>::value;

    typedef typename boost::proto::result_of::child_c<ExprT, 1>::type ChildT;

    // Basic result of the expression to integrate might be an Eigen expression
    typedef typename boost::remove_const
    <
      typename boost::remove_reference
      <
        typename boost::tr1_result_of
        <
          ElementMathImplicit
          (
            ChildT,
            const MappedCoordsT&,
            DataT
          )
        >::type
      >::type
    >::type EigenExprT;



    typedef const typename std::remove_const<typename std::remove_reference<decltype(detail::do_eval(std::declval<EigenExprT>()))>::type>::type& result_type;

    result_type operator ()(typename impl::expr_param expr, typename impl::state_param state, typename impl::data_param data) const
    {
      typedef mesh::Integrators::GaussMappedCoords<order, ShapeFunctionT::shape> GaussT;
      ChildT e = boost::proto::child_c<1>(expr); // expression to integrate
      data.precompute_element_matrices(GaussT::instance().coords.col(0), expr);
      expr.value = GaussT::instance().weights[0] * ElementMathImplicit()(e, state, data);
      for(Uint i = 1; i != GaussT::nb_points; ++i)
      {
        data.precompute_element_matrices(GaussT::instance().coords.col(i), expr);
        expr.value += GaussT::instance().weights[i] * ElementMathImplicit()(e, state, data);
      }
      return expr.value;
    }

  };
};

template<typename GrammarT>
struct ElementQuadratureEval :
  boost::proto::transform< ElementQuadratureEval<GrammarT> >
{
  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  {

    typedef void result_type;

    /// Fusion functor to evaluate each child expression using the GrammarT supplied in the template argument
    struct evaluate_expr
    {
      evaluate_expr(typename impl::expr_param expr, typename impl::state_param state, typename impl::data_param data, const Real weight) :
        m_expr(expr),
        m_state(state),
        m_data(data),
        m_weight(weight * data.support().jacobian_determinant())
      {
      }

      template<typename I>
      void operator()(const I&) const
      {
        evaluate_child(boost::proto::child_c<I::value>(m_expr));
      }

      template<typename ChildExprT>
      void evaluate_child(ChildExprT& expr) const
      {
        tag_dispatch(typename boost::proto::tag_of<ChildExprT>::type(), expr);
      }

      template<typename ChildExprT>
      void tag_dispatch(const boost::proto::tag::plus_assign, ChildExprT& expr) const
      {
        GrammarT()(boost::proto::left(expr) += m_weight * boost::proto::right(expr), m_state, m_data);
      }

      // Issue an error message if a tag was not supported
      template<typename TagT, typename ChildExprT>
      void tag_dispatch(const TagT, ChildExprT& expr) const
      {
        BOOST_MPL_ASSERT(( boost::is_same<TagT, boost::proto::tag::plus_assign> ));
      }

    private:
      typename impl::expr_param m_expr;
      typename impl::state_param  m_state;
      typename impl::data_param m_data;
      const Real m_weight; // The integration weight (gauss point weight * jacobian determinant)
    };

    void operator ()(
                typename impl::expr_param expr
              , typename impl::state_param state
              , typename impl::data_param data
    ) const
    {
      typedef typename boost::remove_reference<DataT>::type UnrefDataT;
      typedef typename UnrefDataT::SupportT::EtypeT SupportShapeFunctionT;
      typedef typename SupportShapeFunctionT::MappedCoordsT MappedCoordsT;

      static const Uint max_order = MaxOrder<ExprT, UnrefDataT, SupportShapeFunctionT::order>::value;
      typedef mesh::Integrators::GaussMappedCoords<IntegrationOrder<max_order>::value, SupportShapeFunctionT::shape> GaussT;

      for(Uint i = 0; i != GaussT::nb_points; ++i)
      {
        // Precompute the primitive element matrices (shape function values, gradients, ...) for the current Gauss point
        data.precompute_element_matrices(GaussT::instance().coords.col(i), expr);
        boost::mpl::for_each< boost::mpl::range_c<int, 1, boost::proto::arity_of<ExprT>::value> >
        (
          evaluate_expr(expr, state, data, GaussT::instance().weights[i])
        );
      }
    }
  };
};

/// Use element_quadrature(expr1, expr2, ..., exprN) to evaluate a group of expressions
static boost::proto::terminal<ElementQuadratureTag>::type element_quadrature = {};


/// Grammar that allows looping over I and J indices
template<typename I, typename J>
struct ElementMathImplicitIndexed :
  boost::proto::or_
  <
    SFOps< boost::proto::call< ElementMathImplicitIndexed<I, J> > >,
    boost::proto::when
    <
      boost::proto::function<boost::proto::terminal<NodalValuesTag>, FieldTypes>,
      VarValue(boost::proto::_value(boost::proto::_child1))
    >,
    FieldInterpolation,
    MathTerminals,
    ElementMatrixGrammar,
    ElementMatrixGrammarIndexed<I, J>,
    EigenMath<boost::proto::call< ElementMathImplicitIndexed<I, J> >, boost::proto::or_< Integers, IndexValues<I, J> > >
  >
{
};

struct ElementIntegration :
  boost::proto::when
  <
    boost::proto::function< boost::proto::terminal< IntegralTag<boost::proto::_> >, ElementMathImplicit >,
    GaussIntegral
  >
{
};

struct ElementQuadrature :
  boost::proto::when
  <
    boost::proto::function< boost::proto::terminal<ElementQuadratureTag>, boost::proto::vararg<boost::proto::_> >,
    ElementQuadratureEval< boost::proto::call< IndexLooper<ElementMathImplicitIndexed> > >
  >
{
};

} // namespace Proto
} // namespace actions
} // namespace solver
} // namespace cf3

#endif // cf3_solver_actions_Proto_ElementIntegration_hpp
