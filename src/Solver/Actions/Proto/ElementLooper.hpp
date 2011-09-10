// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
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
#include <boost/mpl/filter_view.hpp>

#include "ElementData.hpp"
#include "ElementExpressionWrapper.hpp"
#include "ElementGrammar.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CSpace.hpp"
#include "Mesh/ElementTypePredicates.hpp"
#include "Mesh/ShapeFunctionT.hpp"

namespace CF {
namespace Solver {
namespace Actions {
namespace Proto {

/// Find the shape function of each field variable
template<typename ShapeFunctionsT, typename ExprT, typename SupportSF, typename VariablesT, typename VariablesSFT, typename NbVarsT, typename VarIdxT>
struct ExpressionRunner
{
  ExpressionRunner(VariablesT& vars, const ExprT& expr, Mesh::CElements& elems) : variables(vars), expression(expr), elements(elems), m_nb_tests(0), m_found(false) {}

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

    // Find the field group for the variable
    Mesh::CMesh& mesh = Common::find_parent_component<Mesh::CMesh>(elements);
    const Mesh::FieldGroup& var_field_group = Common::find_component_recursively_with_tag<Mesh::Field>(mesh, var.field_tag()).field_group();
    Mesh::CSpace& space = var_field_group.space(elements);

    ++m_nb_tests;

    // Compatibility is ensured higher-up, so we assume that the correct shape function is found if the order matches
    // This needs to be extended once non-Lagrange functions are added
    if(SF::order != space.shape_function().order())
    {
      if(m_nb_tests == boost::mpl::size<ShapeFunctionsT>::value && !m_found)
      {
        throw Common::SetupError(FromHere(), "Needed shape function " + space.shape_function().name() + " for variable " + var.name() + " but it was not in the compiled list");
      }

      return;
    }

    m_found = true;

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


  /// This looks up the support shape function
  template < typename SF >
  void operator() (const SF& sf) const
  {

    BOOST_MPL_ASSERT_MSG(
          SF::order > 0
        , ZERO_ORDER_SUPPORT_NOT_ALLOWED
        , (SF)
        );

    if(is_null(dynamic_cast<const Mesh::ShapeFunctionT<typename SF::SF>*>(&m_elements.element_type().shape_function())))
      return;

    dispatch(boost::mpl::int_<boost::mpl::size< boost::mpl::filter_view< ShapeFunctionsT, Mesh::IsCompatibleWith<SF> > >::value>(), sf);
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
    // Number of variables (integral constant)
    typedef typename ExpressionProperties<ExprT>::NbVarsT NbVarsT;

    // Create an expression runner, taking rather many template arguments, and use it to run the expression.
    // This recurses through the variables, selecting the appropriate shape function for each variable
    ExpressionRunner
    <
      boost::mpl::filter_view< ShapeFunctionsT, Mesh::IsCompatibleWith<SF> >, // MPL vector with the shape functions to consider
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
    // We skip order 0 functions in the top-call, because first the support shape function is determined, and order 0 is not allowed there
    boost::mpl::for_each< boost::mpl::filter_view< ShapeFunctionsT, Mesh::IsMinimalOrder<1> > >( ElementLooper<ShapeFunctionsT, ExprT>(elements, expr, vars) );
  }
};

} // namespace Proto
} // namespace Actions
} // namespace Solver
} // namespace CF

#endif // CF_Solver_Actions_Proto_ElementLooper_hpp
