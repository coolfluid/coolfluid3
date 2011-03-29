// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_Actions_Proto_ElementLooper_hpp
#define CF_Solver_Actions_Proto_ElementLooper_hpp

#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include <boost/fusion/adapted/mpl.hpp>
#include <boost/fusion/mpl.hpp>
#include <boost/fusion/sequence/intrinsic/at.hpp>

#include <boost/mpl/assert.hpp>
#include <boost/mpl/for_each.hpp>

#include "ElementData.hpp"
#include "ElementExpressionWrapper.hpp"
#include "ElementGrammar.hpp"

#include "Mesh/CMesh.hpp"

namespace CF {
namespace Solver {
namespace Actions {
namespace Proto {
  
/// Find the shape function of each field variable
template<typename ShapeFunctionsT, typename ExprT, typename SupportSF, typename VariablesT, typename VariablesSFT, typename NbVarsT, typename VarIdxT>
struct ExpressionRunner
{ 
  ExpressionRunner(VariablesT& vars, const ExprT& expr, Mesh::CElements& elems) : variables(vars), expression(expr), elements(elems) {}
  
  typedef typename boost::remove_reference<typename boost::fusion::result_of::at<VariablesT, VarIdxT>::type>::type VarT;
  
  void run() const
  {
    static_dispatch(boost::mpl::is_void_< VarT>());
  }
  
  // Chosen if VarT is void
  void static_dispatch(boost::mpl::true_) const
  {
    typedef typename boost::mpl::push_back
    <
      VariablesSFT,
      boost::mpl::void_
    >::type NewVariablesSFT;
    
    typedef typename boost::mpl::next<VarIdxT>::type NextIdxT;

    ExpressionRunner
    <
      ShapeFunctionsT,
      ExprT,
      SupportSF,
      VariablesT,
      NewVariablesSFT,
      NbVarsT,
      NextIdxT
    >(variables, expression, elements).run();
  }
  
  // Chosen otherwise
  void static_dispatch(boost::mpl::false_) const
  {
    boost::mpl::for_each<ShapeFunctionsT>( *this );
  }
  
  template <typename SF>
  void operator() ( SF& T ) const
  {
    
    const VarT& var = boost::fusion::at<VarIdxT>(variables);
    if(!Mesh::IsElementType<SF>()(var.element_type(elements)))
      return;
    
    typedef typename boost::mpl::push_back
    <
      VariablesSFT,
      SF
    >::type NewVariablesSFT;
    typedef typename boost::mpl::next<VarIdxT>::type NextIdxT;

    ExpressionRunner
    <
      ShapeFunctionsT,
      ExprT,
      SupportSF,
      VariablesT,
      NewVariablesSFT,
      NbVarsT,
      NextIdxT
    >(variables, expression, elements).run();
  }
  
  VariablesT& variables;
  const ExprT& expression;
  Mesh::CElements& elements;
};



/// Helper struct to launch execution once all shape functions have been determined
template<typename DataT>
struct ElementLooperImpl
{ 
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
      boost::proto::nary_expr< boost::proto::_, boost::proto::vararg<WrapExpression> >
    >
  {};
  
  
  template<typename ExprT>
  void operator()(const ExprT& expr, DataT& data, const Uint nb_elems) const
  {
    const typename DataT::SupportShapeFunction::MappedCoordsT mapped_coords; // needed to deduce proper return type when wrapping
    run(WrapExpression()(expr, mapped_coords, data), data, nb_elems);
  }
  
private:
  template<typename FilteredExprT>
  void run(const FilteredExprT& expr, DataT& data, const Uint nb_elems) const
  {
    ElementGrammar grammar;
    
    for(Uint elem = 0; elem != nb_elems; ++elem)
    {
      // Update the data for the element
      data.set_element(elem);
      // Run the expression using a proto transform, passing as arguments in the standard proto sense: the expression, a state and the data
      grammar(expr, elem, data);
    }
  }
};

/// When we recursed to the last variable, actually run the expression
template<typename ShapeFunctionsT, typename ExprT, typename SupportSF, typename VariablesT, typename VariablesSFT, typename NbVarsT>
struct ExpressionRunner<ShapeFunctionsT, ExprT, SupportSF, VariablesT, VariablesSFT, NbVarsT, NbVarsT>
{
  ExpressionRunner(VariablesT& vars, const ExprT& expr, Mesh::CElements& elems) : variables(vars), expression(expr), elements(elems) {}

  typedef ElementData<VariablesT, VariablesSFT, SupportSF, typename EquationVariables<ExprT, NbVarsT>::type> DataT;
  
  void run() const
  {
    // IF COMPILATION FAILS HERE: the expression passed to for_each_element is invalid
    BOOST_MPL_ASSERT_MSG(
      (boost::proto::matches<ExprT, ElementGrammar>::value),
      INVALID_ELEMENT_EXPRESSION,
      (ElementGrammar));
    
    DataT data(variables, elements);
    
    ElementLooperImpl<DataT>()(expression, data, elements.size());
  }
  
private:
  VariablesT& variables;
  const ExprT& expression;
  Mesh::CElements& elements;
};

/// mpl::for_each compatible functor to loop over elements, using the correct shape function for the geometry
template<typename ShapeFunctionsT, typename ExprT>
struct ElementLooper
{
  
  // Type of a fusion vector that can contain a copy of each variable that is used in the expression
  typedef typename ExpressionProperties<ExprT>::VariablesT VariablesT;
  
  ElementLooper(Mesh::CElements& elements, const ExprT& expr, VariablesT& variables) :
    m_elements(elements),
    m_expr(expr),
    m_variables(variables)
  {
  }
  
  
  template < typename SF >
  void operator() (const SF& sf) const
  {
    if(!Mesh::IsElementType<SF>()(m_elements.element_type()))
      return;
    
    dispatch(boost::mpl::int_<boost::mpl::size< boost::mpl::filter_view< ShapeFunctionsT, Mesh::SF::IsCompatibleWith<SF> > >::value>(), sf);
  }
  
  /// Static dispatch in case everything has the same SF
  template<typename SF>
  void dispatch(const boost::mpl::int_<1>&, const SF& sf) const
  {
    typedef typename ExpressionProperties<ExprT>::NbVarsT NbVarsT;
    typedef ElementData<VariablesT, SF, SF, typename EquationVariables<ExprT, NbVarsT>::type> DataT;
    
    // TODO: add static assert?
    
    DataT data(m_variables, m_elements);
    
    ElementLooperImpl<DataT>()(m_expr, data, m_elements.size());
  }
  
  /// Static dispatch in case different SF are possible
  template<typename T, typename SF>
  void dispatch(const T&, const SF& sf) const
  {
    print_error<SF>(); // remove this if you really need multiple-sf support
    // Number of variables (integral constant)
    typedef typename ExpressionProperties<ExprT>::NbVarsT NbVarsT;
    
    // Create an expression runner, taking rather many template arguments, and use it to run the expression.
    // This recurses through the variables, selecting the appropriate shape function for each variable
    ExpressionRunner
    <
      boost::mpl::filter_view< ShapeFunctionsT, Mesh::SF::IsCompatibleWith<SF> >, // MPL vector with the shape functions to consider
      ExprT, // type of the expression
      SF, // Shape function for the support
      VariablesT, // type of the fusion vector with the variables
      boost::mpl::vector0<>, // Start with an empty vector for the per-variable shape functions
      NbVarsT, // number of variables
      boost::mpl::int_<0> // Start index, as MPL integral constant
    >(m_variables, m_expr, m_elements).run();
  }
  
private:
  Mesh::CElements& m_elements;
  const ExprT& m_expr;
  VariablesT& m_variables;
};

template<typename ShapeFunctionsT, typename ExprT>
void for_each_element(Mesh::CRegion& root_region, const ExprT& expr)
{  
  // Store the variables
  typedef typename ExpressionProperties<ExprT>::VariablesT VariablesT;
  VariablesT vars;
  CopyNumberedVars<VariablesT> ctx(vars); // This is a proto context
  boost::proto::eval(expr, ctx); // calling eval using the above context stores all variables in vars
  
  // Traverse all CElements under the root and evaluate the expression
  BOOST_FOREACH(Mesh::CElements& elements, Common::find_components_recursively<Mesh::CElements>(root_region))
  {
    boost::mpl::for_each<ShapeFunctionsT>( ElementLooper<ShapeFunctionsT, ExprT>(elements, expr, vars) );
  }
};

} // namespace Proto
} // namespace Actions
} // namespace Solver
} // namespace CF

#endif // CF_Solver_Actions_Proto_ElementLooper_hpp
