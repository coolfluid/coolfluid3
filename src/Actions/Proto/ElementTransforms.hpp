// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Actions_Proto_ElementTransforms_hpp
#define CF_Actions_Proto_ElementTransforms_hpp

#include "Mesh/Integrators/Gauss.hpp"

#include "EigenTransforms.hpp"
#include "ElementData.hpp"
#include "ElementVariables.hpp"
#include "Terminals.hpp"
#include "Transforms.hpp"

/// @file 
/// Transforms used in element-wise expression evaluation

namespace CF {
namespace Actions {
namespace Proto {

/// Evaluate a shape function at the supplied mapped coordinates
struct EvalSF :
  boost::proto::transform< EvalSF >
{
  template<typename ExprT, typename MappedCoordsT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, MappedCoordsT, DataT>
  { 
    
    typedef typename boost::remove_reference<DataT>::type::EvalT result_type;
  
    result_type operator ()(
                typename impl::expr_param expr
              , typename impl::state_param mapped_coords
              , typename impl::data_param data
    ) const
    {
      return data.eval(mapped_coords);
    }
  };
};

/// Get the tag type from an SFFunction
struct SFFunctionTag :
  boost::proto::transform< SFFunctionTag >
{
  /// Specialise this for each of the existing function types
  template<typename T>
  struct ExtractTagImpl;
  
  template<typename TagT>
  struct ExtractTagImpl< SFGlobalFunction<TagT> >
  {
    typedef TagT type;
  };
  
  template<typename TagT>
  struct ExtractTagImpl< SFSupportFunction<TagT> >
  {
    typedef TagT type;
  };
  
  template<typename TagT>
  struct ExtractTagImpl< SFFieldFunction<TagT> >
  {
    typedef TagT type;
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
    typedef typename ExtractTagImpl<TermValueT>::type result_type;
    
    result_type operator ()(typename impl::expr_param, typename impl::state_param, typename impl::data_param) const
    {
      return result_type();
    }
  };
};

/// Evaluate shape function member functions pertaining to geometry only
struct EvalSFGlobalFunction :
  boost::proto::transform< EvalSFGlobalFunction >
{
  template<typename TagT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<TagT, StateT, DataT>
  {
    /// DataT is also a functor that adheres to the TR1 result_of protocol, so we can easily determine the result type in a generic way
    typedef typename boost::result_of<typename boost::remove_reference<DataT>::type(TagT)>::type result_type;
  
    result_type operator ()(
                typename impl::expr_param tag
              , typename impl::state_param state
              , typename impl::data_param data
    ) const
    {
      return data(tag);
    }
  };
};

/// Evaluate shape function member functions pertaining to geometry and mapped coordinates
struct EvalSFSupportFunction :
  boost::proto::transform< EvalSFSupportFunction >
{
  template<typename TagT, typename MappedCoordsT, typename DataT>
  struct impl : boost::proto::transform_impl<TagT, MappedCoordsT, DataT>
  {
    /// DataT is also a functor that adheres to the TR1 result_of protocol, so we can easily determine the result type in a generic way
    typedef typename boost::result_of<typename boost::remove_reference<DataT>::type(TagT)>::type result_type;
  
    result_type operator ()(
                typename impl::expr_param tag
              , typename impl::state_param mapped_coords
              , typename impl::data_param data
    ) const
    {
      return data(tag, mapped_coords);
    }
  };
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

/// Matches types that represent geometry
struct SFSupportVariables :
  boost::proto::terminal< Var< boost::proto::_, ConstNodes > >
{
};

/// Matches types that represent field data
struct SFFieldVariables :
  boost::proto::or_
  <
    SFSupportVariables, // all support variables also can be used as fields
    boost::proto::terminal< Var< boost::proto::_, ConstField<boost::proto::_> > >,
    boost::proto::terminal< Var< boost::proto::_, Field<boost::proto::_> > >
  >
{
};

/// Evaluate shape function member functions pertaining to fields and mapped coordinates
struct EvalSFFieldFunction :
  boost::proto::transform< EvalSFFieldFunction >
{
  /// Helper struct to do the actual evaluations. Intermediate data is stored in the data associated with the field
  template<typename SupportDataT, typename FieldDataT>
  struct FieldEvaluator
  {
    /// Gradient type
    typedef typename FieldDataT::GradientT GradientT;
    
    /// Laplacian type
    typedef typename FieldDataT::LaplacianT LaplacianT;
    
    /// Mapped coords
    typedef typename FieldDataT::ShapeFunctionT::MappedCoordsT MappedCoordsT;
    
    /// result of implementation
    template<typename Signature>
    struct result;

    template<typename ThisT>
    struct result<ThisT(GradientTag)>
    {
      typedef const GradientT& type;
    };
    
    template<typename ThisT>
    struct result<ThisT(LaplacianTag)>
    {
      typedef const LaplacianT& type;
    };
    
    /// Return the gradient
    const GradientT& operator()(const GradientTag&, SupportDataT& support, FieldDataT& field, const MappedCoordsT& mapped_coords)
    {
      field.gradient.noalias() = support(JacobianTag(), mapped_coords).inverse() * field.mapped_gradient(mapped_coords);
      return field.gradient;
    }
    
    /// Return the laplacian
    const LaplacianT& operator()(const LaplacianTag&, SupportDataT& support, FieldDataT& field, const MappedCoordsT& mapped_coords)
    {
      // Calculate the gradient
      operator()(GradientTag(), support, field, mapped_coords);
      field.laplacian.noalias() = field.gradient.transpose() * field.gradient;
      return field.laplacian;
    }
  };
  
  template<typename ExprT, typename MappedCoordsT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, MappedCoordsT, DataT>
  { 
    // The complexity here is needed because we can only pass one data item, but we need both the field and support data, so lookup of both is done
    // manually from the top-level global data structure.
    /// function argument 0 contains the terminal representing the function, and thus also the tag
    typedef typename boost::proto::result_of::child_c<ExprT, 0>::type TagExprT;
    /// Arg 1 is the support
    typedef typename boost::proto::result_of::child_c<ExprT, 1>::type SupportExprT;
    /// Arg 2 is the field
    typedef typename boost::proto::result_of::child_c<ExprT, 2>::type FieldExprT;
    
    /// Tag type
    typedef typename boost::result_of<SFFunctionTag(TagExprT)>::type TagT;
    /// Data type for support
    typedef typename boost::remove_reference<typename boost::result_of<NumberedData(SupportExprT, int, DataT)>::type>::type SupportDataT;
    /// Data type for field
    typedef typename boost::remove_reference<typename boost::result_of<NumberedData(FieldExprT, int, DataT)>::type>::type FieldDataT;
    
    typedef typename boost::result_of<FieldEvaluator<SupportDataT, FieldDataT>(TagT)>::type result_type;
  
    result_type operator ()(
                typename impl::expr_param expr
              , typename impl::state_param mapped_coords
              , typename impl::data_param data
    ) const
    {
      return FieldEvaluator<SupportDataT, FieldDataT>()
      (
        TagT(),
        NumberedData()(boost::proto::child_c<1>(expr), 0, data),
        NumberedData()(boost::proto::child_c<2>(expr), 0, data),
        mapped_coords
      );
    }
  };
};
  
/// Evaluation of global shape functions (independent of mapped coords)
struct SFFunctionsGlobal :
  boost::proto::when
  <
    boost::proto::function< boost::proto::terminal< SFGlobalFunction< boost::proto::_ > >, SFSupportVariables>,
    EvalSFGlobalFunction
    (
      SFFunctionTag(boost::proto::_child_c<0>), // tag of the function
      boost::proto::_state, // unused (no mapped coords needed)
      NumberedData(boost::proto::_child_c<1>) // data for the given numbered variable
    )
  >
{
};

/// Matches different types of shape function-related calls. Mapped coordinates are provided explicitely
struct SFFunctionsExplicit :
  boost::proto::or_
  <
    // Evaluation of fields in terms of mapped coordinates (explicit)
    boost::proto::when
    <
      boost::proto::function<boost::proto::or_<SFSupportVariables, SFFieldVariables>, MappedCoordinate>,
      EvalSF( boost::proto::_expr, boost::proto::_value(boost::proto::_right), NumberedData(boost::proto::_left) )
    >,
    SFFunctionsGlobal,
    // Evaluation of support shape functions. Mapped coords explicit.
    boost::proto::when
    <
      boost::proto::function< boost::proto::terminal< SFSupportFunction< boost::proto::_ > >, SFSupportVariables, MappedCoordinate>,
      EvalSFSupportFunction
      (
        SFFunctionTag(boost::proto::_child_c<0>), // tag of the function
        boost::proto::_value(boost::proto::_child_c<2>), // mapped coordinates go to the state
        NumberedData(boost::proto::_child_c<1>) // data for the given numbered variable
      )
    >,
    // Evaluation of field shape functions. Mapped coords explicit.
    boost::proto::when
    <
      boost::proto::function< boost::proto::terminal< SFFieldFunction< boost::proto::_ > >, SFSupportVariables, SFFieldVariables, MappedCoordinate>,
      EvalSFFieldFunction
      (
        boost::proto::_expr, // pass the whole expression, to allow extracting the support and field in the primitive transform
        boost::proto::_value(boost::proto::_child_c<3>), // mapped coordinates go to the state
        boost::proto::_data // We pass the global data, since we need data from two different numbered vars
      )
    >
  >
{
};

/// Matches different types of shape function-related calls. Mapped coordinates are provided implicitly
struct SFFunctionsImplicit :
  boost::proto::or_
  <
    // Evaluation of fields in terms of mapped coordinates (implict)
    boost::proto::when
    <
      boost::proto::function< boost::proto::or_<SFSupportVariables, SFFieldVariables> >,
      EvalSF( boost::proto::_expr, boost::proto::_state, NumberedData(boost::proto::_left) )
    >,
    SFFunctionsGlobal,
    // Evaluation of support shape functions. Mapped coords implicit.
    boost::proto::when
    <
      boost::proto::function< boost::proto::terminal< SFSupportFunction< boost::proto::_ > >, SFSupportVariables>,
      EvalSFSupportFunction
      (
        SFFunctionTag(boost::proto::_child_c<0>), // tag of the function
        boost::proto::_state, // mapped coordinates go to the state
        NumberedData(boost::proto::_child_c<1>) // data for the given numbered variable
      )
    >,
    // Evaluation of field shape functions. Mapped coords explicit.
    boost::proto::when
    <
      boost::proto::function< boost::proto::terminal< SFFieldFunction< boost::proto::_ > >, SFSupportVariables, SFFieldVariables>,
      EvalSFFieldFunction
    >
  >
{
};

/// Forward declaration
struct GaussIntegral;
  
/// Matches expressions that can be used as terms in math formulas for element expressions
struct ElementMath :
  boost::proto::or_
  <
    SFFunctionsExplicit, // Evaluation of shape function functions
    SFFunctionsImplicit, // TODO: Should only be inside integration
    MathTerminals, // Scalars and matrices
    EigenMath<ElementMath>, // Special Eigen functions and Eigen multiplication (overrides default product)
    // Handle integration
    boost::proto::when
    <
      boost::proto::function<boost::proto::terminal< IntegralTag<boost::proto::_> >, ElementMath >,
      GaussIntegral
    >,
    MathOpDefault<ElementMath> // Default evaluation of certain math expressions
  >
{
};

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
          ElementMath
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
    
    typedef typename ValueT::type result_type;
    
    struct integration_ftor
    {
      integration_ftor(ChildT expr, const MappedCoordsT& mapped_coords, typename impl::data_param data) :
        m_expr(expr),
        m_mapped_coords(mapped_coords),
        m_data(data)
      {
      }
      
      inline EigenExprT operator()() const
      {
        return ElementMath()(m_expr, m_mapped_coords, m_data);
      }
      
      ChildT m_expr;
      const MappedCoordsT& m_mapped_coords;
      typename impl::data_param m_data;
    };
       
    result_type operator ()(typename impl::expr_param expr, typename impl::state_param, typename impl::data_param data) const
    {
      MappedCoordsT mapped_coords;
      result_type r;
      ValueT::set_zero(r);
      Mesh::Integrators::gauss_integrate<order, ShapeFunctionT::shape>(integration_ftor(boost::proto::child_c<1>(expr), mapped_coords, data), mapped_coords, r);
      return r;
    }
  };  
};
  
} // namespace Proto
} // namespace Actions
} // namespace CF

#endif // CF_Actions_Proto_ElementTransforms_hpp
