// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_Actions_Proto_ElementTransforms_hpp
#define CF_Solver_Actions_Proto_ElementTransforms_hpp

#include <boost/mpl/assert.hpp>

#include "Mesh/Integrators/Gauss.hpp"

#include "Solver/Actions/Proto/EigenTransforms.hpp"
#include "Solver/Actions/Proto/ElementData.hpp"
#include "Solver/Actions/Proto/ElementVariables.hpp"
#include "Solver/Actions/Proto/Terminals.hpp"
#include "Solver/Actions/Proto/Transforms.hpp"

/// @file 
/// Transforms used in element-wise expression evaluation

namespace CF {
namespace Solver {
namespace Actions {
namespace Proto {

/// Primitive transform to apply a given shape function operation
struct ApplySFOp :
  boost::proto::transform< ApplySFOp >
{
  /// Helper struct to get the data type and data
  template<typename DataT, typename I>
  struct GetData
  {
    BOOST_MPL_ASSERT(( boost::is_reference<DataT> ));
    typedef typename boost::remove_reference<DataT>::type::template DataType<I>::type type;
    
    static type& data(DataT d)
    {
      return d.template var_data<I>();
    }
  };
  
  /// When the variable has no number, return the global data
  template<typename DataT>
  struct GetData<DataT, boost::mpl::void_>
  {
    BOOST_MPL_ASSERT(( boost::is_reference<DataT> ));
    typedef typename boost::remove_reference<DataT>::type type;
    
    static type& data(DataT d)
    {
      return d;
    }
  };
  
  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  {
    // ExprT should be OpTypeForVar<OpT, I>
    typedef typename ExprT::OpType OpT; // Operation type
    typedef typename ExprT::VarIdxT I; // Variable index
    
    typedef typename OpT::template apply
    <
      typename boost::remove_reference<DataT>::type::SupportT,
      StateT,
      typename GetData<DataT, I>::type&
    > ApplyT;
    
    typedef typename ApplyT::result_type result_type;
    
    // state: mapped coords, if any, data: global data associated with geometric support and numbered variables
    result_type operator ()(typename impl::expr_param, typename impl::state_param state, typename impl::data_param data) const
    {
      return ApplyT()( data.support(), state, GetData<DataT, I>::data(data) );
    }
  };
};
  
/// Stores the type of the operation, and the number of the variable (if applicable)
template<typename OpT, typename I=boost::mpl::void_>
struct OpTypeForVar
{
  /// Type of operation
  typedef OpT OpType;
  
  /// Integral constant for the variable, or void if not applicable
  typedef I VarIdxT;
};
 
/// Primitive transform to get the operation type from a terminal representing a shape function operation, as well as the variable index it applies to
/// The terminal index should be supplied as a state parameter (as MPL integral constant)
struct ExtractOpType :
  boost::proto::transform< ExtractOpType >
{
  template<typename T>
  struct ExtractOpImpl;
  
  template<typename OpT>
  struct ExtractOpImpl< SFOp<OpT> >
  {
    typedef OpT type;
  };
  
  template<typename T>
  struct ExtractIdx
  {
    typedef boost::mpl::void_ type;
  };
  
  template<int I>
  struct ExtractIdx< boost::mpl::int_<I> >
  {
    typedef boost::mpl::int_<I> type;
  };
  
  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  {
    /// type of the value of the examined terminal
    typedef typename boost::remove_const
    <
      typename boost::remove_reference
      <
        typename boost::proto::result_of::value<ExprT>::type
      >::type
    >::type TermValueT;
    
    /// Actual tag type
    typedef OpTypeForVar
    <
      typename ExtractOpImpl<TermValueT>::type,
      typename ExtractIdx<typename boost::remove_reference<StateT>::type>::type
    > result_type;
    
    result_type operator ()(typename impl::expr_param, typename impl::state_param, typename impl::data_param) const
    {
      return result_type();
    }
  };
};
  
/// Matches types that represent field data
struct SFFieldVariables :
  boost::proto::or_
  <
    boost::proto::terminal< Var< boost::proto::_, ConstField<boost::proto::_> > >,
    boost::proto::terminal< Var< boost::proto::_, Field<boost::proto::_> > >
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

/// SF operations needing only a support
struct SFSupportOp :
  boost::proto::terminal< SFOp<VolumeOp> >
{
};

/// SF operations needing a support and mapped coordinates
struct SFSupportMappedOp :
  boost::proto::or_
  <
    boost::proto::terminal< SFOp<CoordinatesOp> >,
    boost::proto::terminal< SFOp<JacobianOp> >,
    boost::proto::terminal< SFOp<JacobianDeterminantOp> >,
    boost::proto::terminal< SFOp<NormalOp> >
  >
{
};

/// SF operations needing a support, a field and mapped coordinates
struct SFSupportFieldMappedOp :
  boost::proto::or_
  <
    boost::proto::terminal< SFOp<OuterProductOp> >,
    boost::proto::terminal< SFOp<GradientOp> >,
    boost::proto::terminal< SFOp<LaplacianOp> >
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
      ApplySFOp(ExtractOpType)
    >,
    boost::proto::when // Functions depending only on geometry and mapped coords
    <
      boost::proto::function<SFSupportMappedOp, MappedCoordinate>,
      ApplySFOp( ExtractOpType(boost::proto::_child_c<0>), boost::proto::_value(boost::proto::_child_c<1>) )
    >,
    boost::proto::when // Functions depending on geometry and a field
    <
      boost::proto::function<SFSupportFieldMappedOp, SFFieldVariables, MappedCoordinate>,
      ApplySFOp( ExtractOpType( boost::proto::_child_c<0>, VarNumber(boost::proto::_child_c<1>) ), boost::proto::_value(boost::proto::_child_c<2>) )
    >,
    boost::proto::when // As a special case, fields can be used as a functor taking mapped coordinates. In this case, the interpolated value is returned
    <
      boost::proto::function<SFFieldVariables, MappedCoordinate>,
      ApplySFOp( ExtractOpType( boost::proto::terminal< SFOp<InterpolationOp> >(), VarNumber(boost::proto::_child_c<0>) ), boost::proto::_value(boost::proto::_child_c<1>) )
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
      ApplySFOp(ExtractOpType)
    >,
    boost::proto::when // Functions depending only on geometry and mapped coords
    <
      SFSupportMappedOp,
      ApplySFOp(ExtractOpType)
    >,
    boost::proto::when // Functions depending on geometry and a field
    <
      boost::proto::function<SFSupportFieldMappedOp, SFFieldVariables>,
      ApplySFOp( ExtractOpType( boost::proto::_child_c<0>, VarNumber(boost::proto::_child_c<1>) ) )
    >,
    boost::proto::when // As a special case, fields can be used as a functor taking mapped coordinates. In this case, the interpolated value is returned
    <
      SFFieldVariables,
      ApplySFOp( ExtractOpType( boost::proto::terminal< SFOp<InterpolationOp> >(), VarNumber ) )
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
    EigenMath<ElementMathImplicit>,
    MathOpDefault<ElementMathImplicit>
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
  // IF COMPILATION FAILS HERE: the espression passed to for_each_element is invalid
  BOOST_MPL_ASSERT_MSG(
    (boost::proto::matches<ExprT, ElementMathImplicit>::value),
    INVALID_EXPRESSION_FOR_INTEGRAL,
    (ElementMathImplicit));

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
    typedef typename boost::remove_reference<DataT>::type::SupportT::ShapeFunctionT ShapeFunctionT;
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
    
    typedef typename ValueT::NestedT result_type;
    
    result_type operator ()(typename impl::expr_param expr, typename impl::state_param, typename impl::data_param data) const
    {
      typedef Mesh::Integrators::GaussMappedCoords<order, ShapeFunctionT::shape> GaussT;
      ChildT e = boost::proto::child_c<1>(expr); // expression to integrate
      typename ValueT::type r = GaussT::instance().weights[0] * ElementMathImplicit()(e, GaussT::instance().coords.col(0), data);
      for(Uint i = 1; i != GaussT::nb_points; ++i)
        r += GaussT::instance().weights[i] * ElementMathImplicit()(e, GaussT::instance().coords.col(i), data);
      return r;
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

struct ElementMathBase :
  boost::proto::or_
  <
    SFOpsExplicit,
    MathTerminals,
    ElementIntegration,
    EigenMath<ElementMathBase>
  >
{
};

struct ElementMath :
  boost::proto::or_
  <
    ElementMathBase,
    MathOpDefault<ElementMathBase>
  >
{
};
  
} // namespace Proto
} // namespace Actions
} // namespace Solver
} // namespace CF

#endif // CF_Solver_Actions_Proto_ElementTransforms_hpp
