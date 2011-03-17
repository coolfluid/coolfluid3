// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_Actions_Proto_ElementTransforms_hpp
#define CF_Solver_Actions_Proto_ElementTransforms_hpp

#include <boost/mpl/assert.hpp>

#include <boost/proto/transform/lazy.hpp>

#include "Mesh/Integrators/Gauss.hpp"

#include "Solver/Actions/Proto/EigenTransforms.hpp"
#include "Solver/Actions/Proto/ElementOperations.hpp"
#include "Solver/Actions/Proto/Terminals.hpp"

/// @file 
/// Transforms used in element-wise expression evaluation

namespace CF {
namespace Solver {
namespace Actions {
namespace Proto {

  
/// Matches types that represent field data
struct SFFieldVariables :
  boost::proto::or_
  <
    boost::proto::terminal< Var< boost::proto::_, ScalarField > >,
    boost::proto::terminal< Var< boost::proto::_, VectorField > >
  >
{
};    

/// Matches possible storage for mapped coordinates
struct MappedCoordinate :
  boost::proto::or_
  <
    boost::proto::terminal< Eigen::Matrix<Real, 1, 1 > >,
    boost::proto::terminal< Eigen::Matrix<Real, 2, 1 > >,
    boost::proto::terminal< Eigen::Matrix<Real, 3, 1 > >,
    boost::proto::terminal< Eigen::Matrix<Real, Eigen::Dynamic, 1 > >
  >
{
};

/// Evaluates Shape Fuction operations with explicitely defined mapped coordinates
struct SFOpsExplicit :
  boost::proto::or_
  <
    boost::proto::when // Functions depending only on geometry
    <
      SFSupportOp,
      boost::proto::lazy<boost::proto::_value>
    >,
    boost::proto::when // Functions depending on a variable
    <
      boost::proto::function<SFFieldOp, SFFieldVariables>,
      boost::proto::lazy< boost::proto::call<boost::proto::_value(boost::proto::_child0)>(boost::proto::_value(boost::proto::_child1)) >
    >,
    boost::proto::when // Functions depending only on geometry and mapped coords
    <
      boost::proto::function<SFSupportMappedOp, MappedCoordinate>,
      boost::proto::lazy< boost::proto::call<boost::proto::_value(boost::proto::_child0)>(boost::proto::_expr, boost::proto::_value(boost::proto::_child_c<1>)) >
    >,
    boost::proto::when // Functions depending on geometry and a field
    <
      boost::proto::function<SFSupportFieldMappedOp, SFFieldVariables, MappedCoordinate>,
      boost::proto::lazy< boost::proto::call<boost::proto::_value(boost::proto::_child0)>(boost::proto::_value(boost::proto::_child_c<1>), boost::proto::_value(boost::proto::_child_c<2>)) >
    >,
    boost::proto::when // As a special case, fields can be used as a functor taking mapped coordinates. In this case, the interpolated value is returned
    <
      boost::proto::function<SFFieldVariables, MappedCoordinate>,
      InterpolationOp(boost::proto::_value(boost::proto::_child_c<0>), boost::proto::_value(boost::proto::_child_c<1>) )
    >
  >
{
};

/// Evaluates Shape Fuction operations where the mapped coordinates are implicit (i.e. during integration)
struct SFOpsImplicit :
  boost::proto::or_
  <
    boost::proto::when // Functions depending only on geometry
    <
      SFSupportOp,
      boost::proto::lazy<boost::proto::_value>
    >,
    boost::proto::when // Functions depending on a variable
    <
      boost::proto::function<SFFieldOp, SFFieldVariables>,
      boost::proto::lazy< boost::proto::call<boost::proto::_value(boost::proto::_child0)>(boost::proto::_value(boost::proto::_child1)) >
    >,
    boost::proto::when // Functions depending only on geometry and mapped coords
    <
      SFSupportMappedOp,
      boost::proto::lazy<boost::proto::_value>
    >,
    boost::proto::when // Functions depending on geometry and a field
    <
      boost::proto::function<SFSupportFieldMappedOp, SFFieldVariables>,
      boost::proto::lazy< boost::proto::call<boost::proto::_value(boost::proto::_child0)>(boost::proto::_value(boost::proto::_child_c<1>)) >
    >,
    boost::proto::when // As a special case, fields can be used as a functor taking mapped coordinates. In this case, the interpolated value is returned
    <
      SFFieldVariables,
      InterpolationOp(boost::proto::_value)
    >
  >
{
};

/// Expressions with implicitely known mapped coordinates, valid in i.e. integration, where the integrator
/// is responsible for setting the mapped coordinates of the points to evaluate
struct ElementMathImplicit :
  boost::proto::or_
  <
    SFOpsImplicit,
    MathTerminals,
    EigenMath<ElementMathImplicit>
  >
{
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

/// Primitive transform that evaluates an integral using the Gauss points
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
    typedef typename boost::remove_reference<DataT>::type::SupportT::SF ShapeFunctionT;
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
        typename boost::result_of
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
    
    // Converter to get a real matrix type that can hold the result
    typedef ValueType
    <
      EigenExprT
    > ValueT;
    
    typedef const typename ValueT::type& result_type;
    
    result_type operator ()(typename impl::expr_param expr, typename impl::state_param, typename impl::data_param data) const
    {
      typedef Mesh::Integrators::GaussMappedCoords<order, ShapeFunctionT::shape> GaussT;
      ChildT e = boost::proto::child_c<1>(expr); // expression to integrate
      expr.value = GaussT::instance().weights[0] * ElementMathImplicit()(e, GaussT::instance().coords.col(0), data);
      for(Uint i = 1; i != GaussT::nb_points; ++i)
      {
        expr.value += GaussT::instance().weights[i] * ElementMathImplicit()(e, GaussT::instance().coords.col(i), data);
      }
      return expr.value;
    }
    
  };  
};

struct ElementIntegration :
  boost::proto::when
  <
    boost::proto::function< boost::proto::terminal< IntegralTag<boost::proto::_> >, ElementMathImplicit >,
    GaussIntegral
  >
{
};

struct ElementMath :
  boost::proto::or_
  <
    SFOpsExplicit,
    MathTerminals,
    ElementIntegration,
    ElementMatrixGrammar<ElementMath>,
    EigenMath<ElementMath>
  >
{
};
  
} // namespace Proto
} // namespace Actions
} // namespace Solver
} // namespace CF

#endif // CF_Solver_Actions_Proto_ElementTransforms_hpp
