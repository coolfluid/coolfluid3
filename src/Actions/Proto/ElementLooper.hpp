// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Actions_Proto_ElementLooper_hpp
#define CF_Actions_Proto_ElementLooper_hpp

#include <boost/fusion/algorithm.hpp>
#include <boost/fusion/include/algorithm.hpp>
#include <boost/fusion/adapted/mpl.hpp>
#include <boost/fusion/include/mpl.hpp>
#include <boost/fusion/mpl.hpp>
#include <boost/fusion/container/vector/convert.hpp>
#include <boost/fusion/include/as_vector.hpp>
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

/// ConstNodes variables need shape functions
template<>
struct IsSFDependent<ConstNodes>
{
  typedef SFDependent type;
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
template<typename ShapeFunctionsT, typename ExprT, typename SupportIdxT, typename VariablesT, typename VariablesDataT, typename NbVarsT, typename VarIdxT, typename NeedSFTrait>
struct ExpressionRunner;

/// Specialization for variables that don't depend on shape functions
template<typename ShapeFunctionsT, typename ExprT, typename SupportIdxT, typename VariablesT, typename VariablesDataT, typename NbVarsT, typename VarIdxT>
struct ExpressionRunner<ShapeFunctionsT, ExprT, SupportIdxT, VariablesT, VariablesDataT, NbVarsT, VarIdxT, SFIndependent>
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
    ExpressionRunner<ShapeFunctionsT, ExprT, SupportIdxT, VariablesT, NewVariablesDataT, NbVarsT, NextIdxT, SFDependencyT>(variables, expression, elements).run();
  }
  
  VariablesT& variables;
  const ExprT& expression;
  Mesh::CElements& elements;
};

/// Specialization for variables that depend on shape functions
template<typename ShapeFunctionsT, typename ExprT, typename SupportIdxT, typename VariablesT, typename VariablesDataT, typename NbVarsT, typename VarIdxT>
struct ExpressionRunner<ShapeFunctionsT, ExprT, SupportIdxT, VariablesT, VariablesDataT, NbVarsT, VarIdxT, SFDependent>
{ 
  ExpressionRunner(VariablesT& vars, const ExprT& expr, Mesh::CElements& elems) : variables(vars), expression(expr), elements(elems) {}
  
  void run() const
  {
    boost::mpl::for_each<ShapeFunctionsT>(*this);
  }
  
  template < typename SF >
  void operator() ( SF& T ) const
  {
    typedef typename boost::remove_reference<typename boost::fusion::result_of::at<VariablesT, VarIdxT>::type>::type VarT;
    const VarT& var = boost::fusion::at<VarIdxT>(variables);
    if(!Mesh::IsElementType<SF>()(var.element_type(elements)))
      return;
    
    typedef typename boost::fusion::result_of::push_back
    <
      VariablesDataT,
      SFVariableData<SF, VarT> *
    >::type NewVariablesDataT;
    typedef typename boost::mpl::next<VarIdxT>::type NextIdxT;
    typedef typename SFDependency<VariablesT, NbVarsT, NextIdxT>::type SFDependencyT;
    
    // Determine if the numbered variable is a candidate to be the geometric support
    typedef boost::mpl::vector // types representing a support
    <
      ConstNodes
    > SupportTypesT;

    
    ExpressionRunner
    <
      ShapeFunctionsT,
      ExprT,
      typename boost::mpl::if_<boost::mpl::contains<SupportTypesT, VarT>, VarIdxT, SupportIdxT>::type,
      VariablesT,
      NewVariablesDataT,
      NbVarsT,
      NextIdxT,
      SFDependencyT
    >(variables, expression, elements).run();
  }
  
  VariablesT& variables;
  const ExprT& expression;
  Mesh::CElements& elements;
};

/// When we recursed to the last variable, actually run the expression
template<typename ShapeFunctionsT, typename ExprT, typename SupportIdxT, typename VariablesT, typename VariablesDataT, typename NbVarsT>
struct ExpressionRunner<ShapeFunctionsT, ExprT, SupportIdxT, VariablesT, VariablesDataT, NbVarsT, NbVarsT, void>
{
  ExpressionRunner(VariablesT& vars, const ExprT& expr, Mesh::CElements& elems) : variables(vars), expression(expr), elements(elems) {}
  
  void run() const
  {
    typedef typename boost::fusion::result_of::as_vector<VariablesDataT>::type VariablesDataVectorT; // the result of fusion::push_back operations is not random-access
    ElementData<VariablesT, VariablesDataVectorT, SupportIdxT> data(variables, elements);
    const Uint nb_elems = elements.elements_count();
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

template<typename ShapeFunctionsT, typename ExprT>
void for_each_element(const ExprT& expr)
{ 
  // Number of variables (integral constant)
  typedef typename boost::result_of<ExprVarArity(ExprT)>::type NbVarsT;
  
  // init empty vector that will store variable indices
  typedef boost::mpl::vector_c<Uint> EmptyRangeT;
  
  // Fill the vector with indices 0 to 9, so we allow 10 different (field or node related) variables in an expression
  typedef typename boost::mpl::copy<
      boost::mpl::range_c<int,0,NbVarsT::value>
    , boost::mpl::back_inserter< EmptyRangeT >
    >::type NbVarsRangeT;
  
  // Get the type for each variable that is used, or set to boost::mpl::void_ for unused indices
  typedef typename boost::mpl::transform<NbVarsRangeT, DefineTypeOp<boost::mpl::_1, ExprT > >::type VarTypesT;
  
  // Type of a fusion vector that can contain a copy of each variable that is used in the expression
  typedef typename boost::fusion::result_of::as_vector<VarTypesT>::type VariablesT;
  
  // Store the variables
  VariablesT vars;
  CopyNumberedVars<VariablesT> ctx(vars);
  boost::proto::eval(expr, ctx);
  
  // Use the first ConstNodes we can find to get the root region too loop over
  Mesh::CRegion& root_region = (*boost::fusion::find<ConstNodes>(vars)).region();
  
  // Traverse all CElements under the root and evaluate the expression
  BOOST_FOREACH(Mesh::CElements& elements, Common::find_components_recursively<Mesh::CElements>(root_region))
  {
    // Create an expression runner, taking rather many template arguments, and use it to run the expression.
    // This recurses through the variables, selecting the appropriate shape function for each variable
    ExpressionRunner
    <
      ShapeFunctionsT, // MPL vector with the shape functions to consider
      ExprT, // type of the expression
      boost::mpl::void_, // No support by default
      VariablesT, // type of the fusion vector with the variables
      boost::fusion::vector0<>, // Start with an empty vector for the per-variable data
      NbVarsT, // number of variables
      boost::mpl::int_<0>, // Start index, as MPL integral constant
      typename IsSFDependent< typename boost::remove_reference< typename boost::fusion::result_of::front<VariablesT>::type >::type >::type // Determine if the first var needs a shape function
    >(vars, expr, elements).run();
  }
};

} // namespace Proto
} // namespace Actions
} // namespace CF

#endif // CF_Actions_Proto_ElementLooper_hpp
