// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_Actions_Proto_ElementLooper_hpp
#define CF_Solver_Actions_Proto_ElementLooper_hpp

#include <boost/fusion/algorithm.hpp>
#include <boost/fusion/include/algorithm.hpp>
#include <boost/fusion/adapted/mpl.hpp>
#include <boost/fusion/include/mpl.hpp>
#include <boost/fusion/mpl.hpp>
#include <boost/fusion/container/vector/convert.hpp>
#include <boost/fusion/include/as_vector.hpp>
#include <boost/fusion/sequence/intrinsic/at.hpp>
#include <boost/fusion/sequence/intrinsic/empty.hpp>
#include <boost/fusion/sequence.hpp>

#include <boost/mpl/assert.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/range_c.hpp>
#include <boost/mpl/transform.hpp>
#include <boost/mpl/vector_c.hpp>

#include "ElementData.hpp"
#include "ElementGrammar.hpp"
#include "Transforms.hpp"

#include "Mesh/CMesh.hpp"

namespace CF {
namespace Solver {
namespace Actions {
namespace Proto {

/// Associated with variable types that depend on shape functions
struct SFDependent;

/// Associated with variable types that don't depend on shape functions
struct SFIndependent;

/// Define if a variable needs a shape function or not (by default, it doesn't)
template<typename T>
struct IsSFDependent
{
  typedef SFIndependent type;
};

/// Field variables need shape functions
template<typename T>
struct IsSFDependent< Field<T> >
{
  typedef SFDependent type;
};

/// Field variables need shape functions
template<typename T>
struct IsSFDependent< ConstField<T> >
{
  typedef SFDependent type;
};

template<>
struct IsSFDependent< VectorField >
{
  typedef SFDependent type;
};

template<Uint I>
struct IsSFDependent< ElementMatrix<I> >
{
  typedef SFDependent type;
};

/// Check SF dependency, using variable index
template<typename VariablesT, typename NbVarsT, typename VarIdxT>
struct SFDependency
{
  typedef typename boost::remove_reference<typename boost::fusion::result_of::at<VariablesT, VarIdxT>::type>::type VarT;
  typedef typename IsSFDependent<VarT>::type type;
};

/// Don't check dependency if we are beyond the last variable
template<typename VariablesT, typename NbVarsT>
struct SFDependency<VariablesT, NbVarsT, NbVarsT>
{
  typedef void type;
};

/// Associates helper data (things such as the element nodes) with each variable
template<typename ShapeFunctionsT, typename ExprT, typename SupportSF, typename VariablesT, typename VariablesDataT, typename NbVarsT, typename VarIdxT, typename NeedSFTrait>
struct ExpressionRunner;

/// Specialization for variables that don't depend on shape functions
template<typename ShapeFunctionsT, typename ExprT, typename SupportSF, typename VariablesT, typename VariablesDataT, typename NbVarsT, typename VarIdxT>
struct ExpressionRunner<ShapeFunctionsT, ExprT, SupportSF, VariablesT, VariablesDataT, NbVarsT, VarIdxT, SFIndependent>
{
  ExpressionRunner(VariablesT& vars, const ExprT& expr, Mesh::CElements& elems) : variables(vars), expression(expr), elements(elems) {}
  
  void run() const
  {
    typedef typename boost::remove_reference<typename boost::fusion::result_of::at<VariablesT, VarIdxT>::type>::type VarT;
    typedef typename boost::fusion::result_of::push_back
    <
      VariablesDataT,
      VariableData<VarT> *
    >::type NewVariablesDataT;
    typedef typename boost::mpl::next<VarIdxT>::type NextIdxT;
    typedef typename SFDependency<VariablesT, NbVarsT, NextIdxT>::type SFDependencyT;
    ExpressionRunner<ShapeFunctionsT, ExprT, SupportSF, VariablesT, NewVariablesDataT, NbVarsT, NextIdxT, SFDependencyT>(variables, expression, elements).run();
  }
  
  VariablesT& variables;
  const ExprT& expression;
  Mesh::CElements& elements;
};

/// Specialization for variables that depend on shape functions
template<typename ShapeFunctionsT, typename ExprT, typename SupportSF, typename VariablesT, typename VariablesDataT, typename NbVarsT, typename VarIdxT>
struct ExpressionRunner<ShapeFunctionsT, ExprT, SupportSF, VariablesT, VariablesDataT, NbVarsT, VarIdxT, SFDependent>
{ 
  ExpressionRunner(VariablesT& vars, const ExprT& expr, Mesh::CElements& elems) : variables(vars), expression(expr), elements(elems) {}
  
  void run() const
  {
    typedef typename boost::remove_reference<typename boost::fusion::result_of::at<VariablesT, VarIdxT>::type>::type VarT;
    boost::mpl::for_each<ShapeFunctionsT>( apply<VarT>(variables, expression, elements) );
  }
  
  template<typename VarT>
  struct apply
  {
    apply(VariablesT& vars, const ExprT& expr, Mesh::CElements& elems) : m_variables(vars), m_expression(expr), m_elements(elems) {}
    
    template <typename SF>
    void operator() ( SF& T ) const
    {
      const VarT& var = boost::fusion::at<VarIdxT>(m_variables);
      if(!Mesh::IsElementType<SF>()(var.element_type(m_elements)))
        return;
      
      typedef typename boost::fusion::result_of::push_back
      <
        VariablesDataT,
        SFVariableData<SF, VarT> *
      >::type NewVariablesDataT;
      typedef typename boost::mpl::next<VarIdxT>::type NextIdxT;
      typedef typename SFDependency<VariablesT, NbVarsT, NextIdxT>::type SFDependencyT;

      ExpressionRunner
      <
        ShapeFunctionsT,
        ExprT,
        SupportSF,
        VariablesT,
        NewVariablesDataT,
        NbVarsT,
        NextIdxT,
        SFDependencyT
      >(m_variables, m_expression, m_elements).run();
    }
    
    VariablesT& m_variables;
    const ExprT& m_expression;
    Mesh::CElements& m_elements;
    
  };
  
  template<Uint I>
  struct apply< ElementMatrix<I> >
  {
    apply(VariablesT& vars, const ExprT& expr, Mesh::CElements& elems) : m_variables(vars), m_expression(expr), m_elements(elems) {}
    
    template <typename SF>
    void operator() ( SF& T ) const
    {
      typedef typename boost::remove_reference<typename boost::fusion::result_of::at<VariablesT, VarIdxT>::type>::type VarT;
      if(!Mesh::IsElementType<SF>()(boost::fusion::at_c<I>(m_variables).element_type(m_elements)))
        return;
      
      typedef typename boost::fusion::result_of::push_back
      <
        VariablesDataT,
        SFVariableData<SF, VarT> *
      >::type NewVariablesDataT;
      typedef typename boost::mpl::next<VarIdxT>::type NextIdxT;
      typedef typename SFDependency<VariablesT, NbVarsT, NextIdxT>::type SFDependencyT;

      ExpressionRunner
      <
        ShapeFunctionsT,
        ExprT,
        SupportSF,
        VariablesT,
        NewVariablesDataT,
        NbVarsT,
        NextIdxT,
        SFDependencyT
      >(m_variables, m_expression, m_elements).run();
    }
    
    VariablesT& m_variables;
    const ExprT& m_expression;
    Mesh::CElements& m_elements;
    
  };
  
  VariablesT& variables;
  const ExprT& expression;
  Mesh::CElements& elements;
};

/// When we recursed to the last variable, actually run the expression
template<typename ShapeFunctionsT, typename ExprT, typename SupportSF, typename VariablesT, typename VariablesDataT, typename NbVarsT>
struct ExpressionRunner<ShapeFunctionsT, ExprT, SupportSF, VariablesT, VariablesDataT, NbVarsT, NbVarsT, void>
{
  ExpressionRunner(VariablesT& vars, const ExprT& expr, Mesh::CElements& elems) : variables(vars), expression(expr), elements(elems) {}
  
  void run() const
  {
    typedef typename boost::fusion::result_of::as_vector<VariablesDataT>::type VariablesDataVectorT; // the result of fusion::push_back operations is not random-access
    ElementData<VariablesT, VariablesDataVectorT, SupportSF> data(variables, elements);
    const Uint nb_elems = elements.size();
    ElementGrammar grammar;
    for(Uint elem = 0; elem != nb_elems; ++elem)
    {
      // Update the data for the element
      data.set_element(elem);
      // Run the expression using a proto transform, passing as arguments in the standard proto sense: the expression, a state and the data
      grammar(expression, elem, data);
    }
  }
  
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
  void operator() ( SF& T ) const
  {
    if(!Mesh::IsElementType<SF>()(m_elements.element_type()))
      return;
    
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
      boost::fusion::vector0<>, // Start with an empty vector for the per-variable data
      NbVarsT, // number of variables
      boost::mpl::int_<0>, // Start index, as MPL integral constant
      typename boost::mpl::if_ // Determine if the first var needs a shape function, if there are numbered variables
      <
        typename boost::fusion::result_of::empty<VariablesT>::type,
        void,
        typename IsSFDependent<typename boost::remove_reference
        <
          typename boost::fusion::result_of::front<VariablesT>::type
        >::type >::type
      >::type
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
  // IF COMPILATION FAILS HERE: the espression passed to for_each_element is invalid
  BOOST_MPL_ASSERT_MSG(
    (boost::proto::matches<ExprT, ElementGrammar>::value),
    INVALID_ELEMENT_EXPRESSION,
    (ElementGrammar));
  
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
