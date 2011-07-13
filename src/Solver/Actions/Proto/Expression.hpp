// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_Actions_Proto_Expression_hpp
#define CF_Solver_Actions_Proto_Expression_hpp

#include <map>

#include <boost/mpl/for_each.hpp>
//#include <boost/proto/deep_copy.hpp>

#include "Common/FindComponents.hpp"
#include "Common/Foreach.hpp"
#include "Common/Option.hpp"
#include "Common/OptionT.hpp"
#include "Common/OptionList.hpp"

#include "Mesh/CRegion.hpp"
#include "Mesh/CElements.hpp"

#include "Physics/PhysModel.hpp"

#include "ConfigurableConstant.hpp"
#include "ElementLooper.hpp"
#include "ElementMatrix.hpp"
#include "NodeLooper.hpp"
#include "Transforms.hpp"

namespace CF {
namespace Solver {
namespace Actions {
namespace Proto {

/// Abstract interface for classes that can hold a proto expression
class Expression
{
public:
  /// Pointer typedefs
  typedef boost::shared_ptr<Expression> Ptr;
  typedef boost::shared_ptr<Expression const> ConstPtr;

  /// Run the stored expression in a loop over the region
  virtual void loop(Mesh::CRegion& region) = 0;

  /// Generate the required options for configurable items in the expression
  /// If an option already existed, only a link will be created
  /// @param options The optionlist that will hold the generated options
  virtual void add_options(Common::OptionList& options) = 0;

  /// Register the variables that appear in the expression with a physical model
  virtual void register_variables(Physics::PhysModel& physical_model) = 0;
  
	virtual ~Expression() {}
};

/// Boilerplate implementation
template<typename ExprT>
class ExpressionBase : public Expression
{
public:

  ExpressionBase(const ExprT& expr) :
    m_constant_values(),
    m_expr( DeepCopy()( ReplaceConfigurableConstants()(expr, m_constant_values) ) )
  {
    // Store the variables
    CopyNumberedVars<VariablesT> ctx(m_variables);
    boost::proto::eval(expr, ctx);
  }

  void add_options(Common::OptionList& options)
  {
    // Add scalar options
    for(ConstantStorage::ScalarsT::iterator it = m_constant_values.m_scalars.begin(); it != m_constant_values.m_scalars.end(); ++it)
    {
      const std::string& name = it->first;
      Common::Option& option = options.check(name) ? options.option(name) : *options.add_option< Common::OptionT<Real> >(name, it->second);
      option.set_description(m_constant_values.descriptions[name]);
      option.link_to(&it->second);
    }

    // Add vector options
    for(ConstantStorage::VectorsT::iterator it = m_constant_values.m_vectors.begin(); it != m_constant_values.m_vectors.end(); ++it)
    {
      const std::string& name = it->first;
      Common::Option& option = options.check(name) ? options.option(name) : *options.add_option< Common::OptionT<RealVector> >(name, it->second);
      option.set_description(m_constant_values.descriptions[name]);
      option.link_to(&it->second);
    }
  }
  
  void register_variables(Physics::PhysModel& physical_model)
  {
    boost::mpl::for_each< boost::mpl::range_c<int, 0, NbVarsT::value> >( RegisterVariables(physical_model, m_variables) );
  }

private:
  /// Values for configurable constants
  ConstantStorage m_constant_values;
protected:

  /// Store a copy of the expression
  typedef typename boost::result_of< DeepCopy(typename boost::result_of<ReplaceConfigurableConstants(ExprT, ConstantStorage)>::type) >::type CopiedExprT;
  CopiedExprT m_expr;

  // Number of variables
  typedef typename ExpressionProperties<ExprT>::NbVarsT NbVarsT;

  // Type of a fusion vector that can contain a copy of each variable that is used in the expression
  typedef typename ExpressionProperties<ExprT>::VariablesT VariablesT;
  VariablesT m_variables;

  // True for the variables that are stored
  typedef typename EquationVariables<ExprT, NbVarsT>::type EquationVariablesT;

private:

  /// Functor to register variables in a physical model
  struct RegisterVariables
  {
    RegisterVariables(Physics::PhysModel& physical_model, VariablesT& variables) :
      m_physical_model(physical_model),
      m_variables(variables)
    {
    }

    /// Operator taking an MPL integral constant as type
    template<typename I>
    void operator()(const I&) const
    {
      apply(boost::fusion::at<I>(m_variables), boost::mpl::at<EquationVariablesT, I>::type::value);
    }

    /// non-op for non-existing variables in the list
    void apply(const boost::mpl::void_&, const bool) const
    {
    }

    /// Register a scalar
    void apply(ScalarField& field, const bool is_equation_var) const
    {
      m_physical_model.register_variable(field.name(), field.variable_name, field.field_name, Physics::PhysModel::SCALAR, is_equation_var);
    }

    /// Register a vector field
    void apply(VectorField& field, const bool is_equation_var) const
    {
      m_physical_model.register_variable(field.name(), field.variable_name, field.field_name, Physics::PhysModel::VECTOR, is_equation_var);
    }

  private:
    Physics::PhysModel& m_physical_model;
    VariablesT& m_variables;
  };
};

template< typename ExprT, typename ElementTypes >
class ElementsExpression : public ExpressionBase<ExprT>
{
  typedef ExpressionBase<ExprT> BaseT;
public:

  ElementsExpression(const ExprT& expr) : BaseT(expr)
  {
  }

  void loop(Mesh::CRegion& region)
  {
    // Traverse all CElements under the region and evaluate the expression
    BOOST_FOREACH(Mesh::CElements& elements, Common::find_components_recursively<Mesh::CElements>(region) )
    {
      boost::mpl::for_each<ElementTypes>( ElementLooper<ElementTypes, typename BaseT::CopiedExprT>(elements, BaseT::m_expr, BaseT::m_variables) );
    }
  }
};

/// Expression for looping over nodes
template<typename ExprT>
class NodesExpression : public ExpressionBase<ExprT>
{
  typedef ExpressionBase<ExprT> BaseT;
public:

  NodesExpression(const ExprT& expr) : BaseT(expr)
  {
  }

  void loop(Mesh::CRegion& region)
  {
    // IF COMPILATION FAILS HERE: the espression passed is invalid
    BOOST_MPL_ASSERT_MSG(
      (boost::proto::matches<typename BaseT::CopiedExprT, NodeGrammar>::value),
      INVALID_NODE_EXPRESSION,
      (NodeGrammar));

    boost::mpl::for_each< boost::mpl::range_c<Uint, 1, 4> >( NodeLooper<typename BaseT::CopiedExprT>(BaseT::m_expr, region, BaseT::m_variables) );
  }
};

/// Default element types supported by elements expressions
typedef boost::mpl::vector4<Mesh::SF::Line1DLagrangeP1, Mesh::SF::Triag2DLagrangeP1, Mesh::SF::Quad2DLagrangeP1, Mesh::SF::Hexa3DLagrangeP1> DefaultElementTypes;

/// Convenience method to construct an Expression to loop over elements
/// @returns a shared pointer to the constructed expression
template<typename ExprT, typename ElementTypes>
boost::shared_ptr< ElementsExpression<ExprT, ElementTypes> > elements_expression(ElementTypes, const ExprT& expr)
{
  return boost::shared_ptr< ElementsExpression<ExprT, ElementTypes> >(new ElementsExpression<ExprT, ElementTypes>(expr));
}

/// Convenience method to construct an Expression to loop over elements
/// @returns a shared pointer to the constructed expression
template<typename ExprT>
boost::shared_ptr< ElementsExpression<ExprT, DefaultElementTypes> > elements_expression(const ExprT& expr)
{
  return elements_expression(DefaultElementTypes(), expr);
}

/// Convenience method to construct an Expression to loop over elements
/// @returns a shared pointer to the constructed expression
template<typename ExprT>
boost::shared_ptr< NodesExpression<ExprT> > nodes_expression(const ExprT& expr)
{
  return boost::shared_ptr< NodesExpression<ExprT> >(new NodesExpression<ExprT>(expr));
}

} // namespace Proto
} // namespace Actions
} // namespace Solver
} // namespace CF

#endif // CF_Solver_Actions_Proto_Expression_hpp
