// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_actions_Proto_ElementLooper_hpp
#define cf3_solver_actions_Proto_ElementLooper_hpp

#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include <boost/fusion/adapted/mpl.hpp>
#include <boost/fusion/mpl.hpp>
#include <boost/fusion/sequence/intrinsic/at.hpp>

#include <boost/mpl/assert.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/filter_view.hpp>

#include "ElementData.hpp"
#include "ElementExpressionWrapper.hpp"
#include "ElementGrammar.hpp"

#include "mesh/Mesh.hpp"
#include "mesh/Space.hpp"
#include "mesh/ElementTypePredicates.hpp"

namespace cf3 {
namespace solver {
namespace actions {
namespace Proto {

/// Check if all variables are on fields with element type ETYPE
template<typename ETYPE>
struct CheckSameEtype
{
  CheckSameEtype(mesh::Elements& elems) : elements(elems) {}

  template <typename VarT>
  void operator() ( const VarT& var ) const
  {
    // Find the field group for the variable
    const mesh::Mesh& mesh = common::find_parent_component<mesh::Mesh>(elements);
    const mesh::Dictionary& var_dict = common::find_component_recursively_with_tag<mesh::Field>(mesh, var.field_tag()).dict();
    const mesh::Space& space = var_dict.space(elements);

    if(ETYPE::order != space.shape_function().order()) // TODO also check the same space (Lagrange, ...)
    {
      throw common::SetupError(FromHere(), "Needed shape function " + space.shape_function().derived_type_name() + " for variable " + var.name() + " but it was not in the compiled list");
    }
  }

  void operator() ( const boost::mpl::void_& ) const
  {
  }

  mesh::Elements& elements;
};

/// Find the concrete element type of each field variable
template<typename ElementTypesT, typename ExprT, typename SupportETYPE, typename VariablesT, typename VariablesEtypesT, typename NbVarsT, typename VarIdxT>
struct ExpressionRunner
{
  ExpressionRunner(VariablesT& vars, const ExprT& expr, mesh::Elements& elems) : variables(vars), expression(expr), elements(elems), m_nb_tests(0), m_found(false) {}

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
      VariablesEtypesT,
      boost::mpl::void_
    >::type NewVariablesEtypesT;

    typedef typename boost::mpl::next<VarIdxT>::type NextIdxT;

    ExpressionRunner
    <
      ElementTypesT,
      ExprT,
      SupportETYPE,
      VariablesT,
      NewVariablesEtypesT,
      NbVarsT,
      NextIdxT
    >(variables, expression, elements).run();
  }

  // Chosen otherwise
  void static_dispatch(boost::mpl::false_) const
  {
    boost::mpl::for_each<ElementTypesT>( *this );
  }

  template <typename ETYPE>
  void operator() ( ETYPE& T ) const
  {

    const VarT& var = boost::fusion::at<VarIdxT>(variables);

    // Find the field group for the variable
    const mesh::Mesh& mesh = common::find_parent_component<mesh::Mesh>(elements);
    const mesh::Dictionary& var_dict = common::find_component_recursively_with_tag<mesh::Field>(mesh, var.field_tag()).dict();
    const mesh::Space& space = var_dict.space(elements);

    ++m_nb_tests;

    // Compatibility is ensured higher-up, so we assume that the correct shape function is found if the order matches
    // This needs to be extended once non-Lagrange functions are added
    if(ETYPE::order != space.shape_function().order())
    {
      if(m_nb_tests == boost::mpl::size<ElementTypesT>::value && !m_found)
      {
        throw common::SetupError(FromHere(), "Needed element type " + space.support().element_type().derived_type_name() + " for variable " + var.name() + " but it was not in the compiled list");
      }

      return;
    }

    m_found = true;

    typedef typename boost::mpl::push_back
    <
      VariablesEtypesT,
      ETYPE
    >::type NewVariablesEtypesT;
    typedef typename boost::mpl::next<VarIdxT>::type NextIdxT;

    ExpressionRunner
    <
      ElementTypesT,
      ExprT,
      SupportETYPE,
      VariablesT,
      NewVariablesEtypesT,
      NbVarsT,
      NextIdxT
    >(variables, expression, elements).run();
  }

  VariablesT& variables;
  const ExprT& expression;
  mesh::Elements& elements;
  // Number of times we tried a shape function
  mutable Uint m_nb_tests;
  mutable bool m_found;
};



/// Helper struct to launch execution once all shape functions have been determined
template<typename DataT>
struct ElementLooperImpl
{
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
template<typename ElementTypesT, typename ExprT, typename SupportETYPE, typename VariablesT, typename VariablesEtypesT, typename NbVarsT>
struct ExpressionRunner<ElementTypesT, ExprT, SupportETYPE, VariablesT, VariablesEtypesT, NbVarsT, NbVarsT>
{
  ExpressionRunner(VariablesT& vars, const ExprT& expr, mesh::Elements& elems) : variables(vars), expression(expr), elements(elems) {}

  typedef ElementData<VariablesT, VariablesEtypesT, SupportETYPE, typename EquationVariables<ExprT, NbVarsT>::type> DataT;

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
  mesh::Elements& elements;
};

/// mpl::for_each compatible functor to loop over elements, using the correct shape function for the geometry
template<typename ElementTypesT, typename ExprT>
struct ElementLooper
{

  // Type of a fusion vector that can contain a copy of each variable that is used in the expression
  typedef typename ExpressionProperties<ExprT>::VariablesT VariablesT;

  ElementLooper(mesh::Elements& elements, const ExprT& expr, VariablesT& variables) :
    m_elements(elements),
    m_expr(expr),
    m_variables(variables)
  {
  }


  /// This looks up the support shape function
  template < typename ETYPE >
  void operator() (const ETYPE& sf) const
  {

    BOOST_MPL_ASSERT_MSG(
          ETYPE::order > 0
        , ZERO_ORDER_SUPPORT_NOT_ALLOWED
        , (ETYPE)
        );

    if(!mesh::IsElementType<ETYPE>()(m_elements.element_type()))
      return;

    dispatch(boost::mpl::int_<boost::mpl::size< boost::mpl::filter_view< ElementTypesT, mesh::IsCompatibleWith<ETYPE> > >::value>(), sf);
    
    FieldSynchronizer::instance().synchronize();
  }

  /// Static dispatch in case everything has the same ETYPE
  template<typename ETYPE>
  void dispatch(const boost::mpl::int_<1>&, const ETYPE& sf) const
  {
    typedef typename ExpressionProperties<ExprT>::NbVarsT NbVarsT;
    typedef ElementData<VariablesT, ETYPE, ETYPE, typename EquationVariables<ExprT, NbVarsT>::type> DataT;

    // Verify the types match, and throw an error if non-matching fields are found
    boost::fusion::for_each(m_variables, CheckSameEtype<ETYPE>(m_elements));

    DataT data(m_variables, m_elements);

    ElementLooperImpl<DataT>()(m_expr, data, m_elements.size());
  }

  /// Static dispatch in case different ETYPE are possible
  template<typename T, typename ETYPE>
  void dispatch(const T&, const ETYPE& sf) const
  {
    // Number of variables (integral constant)
    typedef typename ExpressionProperties<ExprT>::NbVarsT NbVarsT;

    // Create an expression runner, taking rather many template arguments, and use it to run the expression.
    // This recurses through the variables, selecting the appropriate shape function for each variable
    ExpressionRunner
    <
      boost::mpl::filter_view< ElementTypesT, mesh::IsCompatibleWith<ETYPE> >, // MPL vector with the element types to consider
      ExprT, // type of the expression
      ETYPE, // Element type for the support
      VariablesT, // type of the fusion vector with the variables
      boost::mpl::vector0<>, // Start with an empty vector for the per-variable element types
      NbVarsT, // number of variables
      boost::mpl::int_<0> // Start index, as MPL integral constant
    >(m_variables, m_expr, m_elements).run();
  }

private:
  mesh::Elements& m_elements;
  const ExprT& m_expr;
  VariablesT& m_variables;
};

template<typename ElementTypesT, typename ExprT>
void for_each_element(mesh::Region& root_region, const ExprT& expr)
{
  // Store the variables
  typedef typename ExpressionProperties<ExprT>::VariablesT VariablesT;
  VariablesT vars;
  CopyNumberedVars<VariablesT> ctx(vars); // This is a proto context
  boost::proto::eval(expr, ctx); // calling eval using the above context stores all variables in vars

  // Traverse all Elements under the root and evaluate the expression
  BOOST_FOREACH(mesh::Elements& elements, common::find_components_recursively<mesh::Elements>(root_region))
  {
    // We skip order 0 functions in the top-call, because first the support shape function is determined, and order 0 is not allowed there
    boost::mpl::for_each< boost::mpl::filter_view< ElementTypesT, mesh::IsMinimalOrder<1> > >( ElementLooper<ElementTypesT, ExprT>(elements, expr, vars) );
  }
};

} // namespace Proto
} // namespace actions
} // namespace solver
} // namespace cf3

#endif // cf3_solver_actions_Proto_ElementLooper_hpp
