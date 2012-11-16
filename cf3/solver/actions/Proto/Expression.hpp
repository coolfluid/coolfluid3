// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_actions_Proto_Expression_hpp
#define cf3_solver_actions_Proto_Expression_hpp

#include <map>

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/fusion/algorithm/iteration/for_each.hpp>

#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"
#include "common/Option.hpp"
#include "common/OptionT.hpp"
#include "common/OptionArray.hpp"
#include "common/OptionList.hpp"

#include "math/VariableManager.hpp"
#include "math/VariablesDescriptor.hpp"

#include "mesh/Region.hpp"
#include "mesh/Elements.hpp"
#include "mesh/LagrangeP1/ElementTypes.hpp"
#include "physics/PhysModel.hpp"

#include "ConfigurableConstant.hpp"
#include "ElementLooper.hpp"
#include "ElementMatrix.hpp"
#include "NodeLooper.hpp"
#include "NodeGrammar.hpp"
#include "PhysicsConstant.hpp"
#include "Transforms.hpp"

namespace cf3 {
namespace solver {
namespace actions {
namespace Proto {

/// Abstract interface for classes that can hold a proto expression
class Expression
{
public:
  /// Run the stored expression in a loop over the region
  virtual void loop(mesh::Region& region) = 0;

  /// Generate the required options for configurable items in the expression
  /// If an option already existed, only a link will be created
  /// @param options The optionlist that will hold the generated options
  virtual void add_options(common::OptionList& options) = 0;

  /// Returns the list of physics constants referred to by this expression
  virtual PhysicsConstantStorage& physics_constants() = 0;

  /// Register the variables that appear in the expression with a physical model
  virtual void register_variables(physics::PhysModel& physical_model) = 0;

  /// Complete the passed map with field information used by the stored expression. The map is built as follows:
  /// key: the tag for the field to use or create
  /// value: space library name, to indicate what kind of field is expected
  virtual void insert_field_info(std::map<std::string, std::string>& tags) const = 0;

  virtual ~Expression() {}
};

/// Boilerplate implementation
template<typename ExprT>
class ExpressionBase : public Expression
{
public:

  ExpressionBase(const ExprT& expr) :
    m_constant_values(),
    m_expr( DeepCopy()( ReplaceConfigurableConstants()(ReplacePhysicsConstants()(expr, m_physics_values), m_constant_values) ) )
  {
    // Store the variables
    CopyNumberedVars<VariablesT> ctx(m_variables);
    boost::proto::eval(expr, ctx);
  }

  void add_options(common::OptionList& options)
  {
    // Add scalar options
    for(ConstantStorage::ScalarsT::iterator it = m_constant_values.m_scalars.begin(); it != m_constant_values.m_scalars.end(); ++it)
    {
      const std::string& name = it->first;

      if(options.check(name))
      {
        options.erase(name);
      }

      common::Option& option = options.add(name, it->second);
      option.description(m_constant_values.descriptions[name]);
      option.link_to(&it->second);
      option.mark_basic();
    }

    // Add vector options
    for(ConstantStorage::VectorsT::iterator it = m_constant_values.m_vectors.begin(); it != m_constant_values.m_vectors.end(); ++it)
    {
      // Initialize proxy with the default value
      std::vector<Real>& vec_proxy = m_constant_values.m_vector_proxies[it->first];
      const RealVector& real_vec = it->second;
      const Uint nb_comps = real_vec.size();
      vec_proxy.resize(nb_comps);
      for(Uint i = 0; i != nb_comps; ++i)
      {
        vec_proxy[i] = real_vec[i];
      }

      const std::string& name = it->first;

      if(options.check(name))
      {
        options.erase(name);
      }
      common::Option& option = options.add(name, vec_proxy);

      option.description(m_constant_values.descriptions[name]);
      option.link_to(&vec_proxy);
      option.attach_trigger(boost::bind(&ConstantStorage::convert_vector_proxy, &m_constant_values));
      option.mark_basic();
    }
  }

  virtual PhysicsConstantStorage& physics_constants()
  {
    return m_physics_values;
  }

  void register_variables(physics::PhysModel& physical_model)
  {
    boost::fusion::for_each(m_variables, RegisterVariables(physical_model));
  }

  void insert_field_info(std::map<std::string, std::string>& tags) const
  {
    boost::fusion::for_each(m_variables, AppendTags(tags));
  }

private:
  /// Values for configurable constants
  ConstantStorage m_constant_values;
  /// Values for physics constants
  PhysicsConstantStorage m_physics_values;
protected:

  /// Store a copy of the expression
  typedef typename boost::result_of< DeepCopy(typename boost::result_of<ReplaceConfigurableConstants(typename boost::result_of<ReplacePhysicsConstants(ExprT, PhysicsConstantStorage)>::type, ConstantStorage)>::type) >::type CopiedExprT;
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
    RegisterVariables(physics::PhysModel& physical_model) :
      m_physical_model(physical_model)
    {
    }

    /// Register a scalar
    void operator()(ScalarField& field) const
    {
      get_descriptor(field.field_tag()).push_back(field.name(), math::VariablesDescriptor::Dimensionalities::SCALAR);
    }

    /// Register a vector field
    void operator()(VectorField& field) const
    {
      get_descriptor(field.field_tag()).push_back(field.name(), math::VariablesDescriptor::Dimensionalities::VECTOR);
    }

    /// Skip unused variables
    void operator()(const boost::mpl::void_&) const
    {
    }

  private:
    /// Get the VariablesDescriptor with the given tag
    math::VariablesDescriptor& get_descriptor(const std::string& tag) const
    {
      math::VariablesDescriptor* result = 0;
      BOOST_FOREACH(math::VariablesDescriptor& descriptor, common::find_components_with_tag<math::VariablesDescriptor>(m_physical_model.variable_manager(), tag))
      {
        if(is_not_null(result))
          throw common::SetupError(FromHere(), "Variablemanager " + m_physical_model.variable_manager().uri().string() + " has multiple descriptors with tag " + tag);
        result = &descriptor;
      }

      if(is_null(result))
      {
        result = m_physical_model.variable_manager().template create_component<math::VariablesDescriptor>(tag).get();
        result->add_tag(tag);
      }

      return *result;
    }

    physics::PhysModel& m_physical_model;
  };

  /// Functor to store the tags used by a field
  struct AppendTags
  {
    AppendTags(std::map<std::string, std::string>& tags) :
      m_tags(tags)
    {
    }

    /// Register a scalar
    void operator()(const FieldBase& field) const
    {
      if(m_tags.insert( std::make_pair(field.field_tag(), field.space_lib_name()) ).first->second != field.space_lib_name())
        throw common::SetupError(FromHere(), "Field with tag " + field.field_tag() + " uses two different space libs");
    }

    /// Skip unused variables
    void operator()(const boost::mpl::void_&) const
    {
    }

    std::map<std::string, std::string>& m_tags;
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

  void loop(mesh::Region& region)
  {
    // Traverse all Elements under the region and evaluate the expression
    BOOST_FOREACH(mesh::Elements& elements, common::find_components_recursively<mesh::Elements>(region) )
    {
      boost::mpl::for_each<boost::mpl::filter_view< ElementTypes, mesh::IsMinimalOrder<1> > >( ElementLooper<ElementTypes, typename BaseT::CopiedExprT>(elements, BaseT::m_expr, BaseT::m_variables) );
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

  void loop(mesh::Region& region)
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
typedef mesh::LagrangeP1::CellTypes DefaultElementTypes;

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
} // namespace actions
} // namespace solver
} // namespace cf3

#endif // cf3_solver_actions_Proto_Expression_hpp
