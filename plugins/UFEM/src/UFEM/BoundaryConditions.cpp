// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"
#include "common/Log.hpp"
#include "common/OptionArray.hpp"
#include "common/Signal.hpp"
#include "common/Builder.hpp"
#include "common/OptionT.hpp"

#include "common/XML/SignalOptions.hpp"

#include "math/VariableManager.hpp"
#include "math/VariablesDescriptor.hpp"

#include "math/LSS/System.hpp"

#include "mesh/Dictionary.hpp"
#include "mesh/Field.hpp"

#include "solver/actions/Proto/BlockAccumulator.hpp"
#include "solver/actions/Proto/DirichletBC.hpp"
#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"

#include "solver/Tags.hpp"

#include "BoundaryConditions.hpp"
#include "ParsedFunctionExpression.hpp"
#include "Tags.hpp"

namespace cf3 {
namespace UFEM {

using namespace common;
using namespace common::XML;
using namespace math;
using namespace mesh;
using namespace solver;
using namespace solver::actions;
using namespace solver::actions::Proto;

////////////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < BoundaryConditions, ActionDirector, LibUFEM > BoundaryConditions_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

struct BoundaryConditions::Implementation
{
  Implementation(ActionDirector& comp) :
    m_component(comp),
    m_physical_model(),
    dirichlet(m_component.options().add("lss", Handle<LSS::System>())
              .pretty_name("LSS")
              .description("The referenced linear system solver"))
  {
    m_component.options().add(solver::Tags::regions(), std::vector<URI>())
      .pretty_name("Regions")
      .description("Regions the boundary condition applies to")
      .link_to(&m_region_uris);
    m_component.options().add< Handle<physics::PhysModel> >(solver::Tags::physical_model())
      .pretty_name("Physical Model")
      .description("Physical Model")
      .link_to(&m_physical_model);
  }

  boost::shared_ptr< Action > create_constant_scalar_bc(const std::string& region_name, const std::string& variable_name)
  {
    FieldVariable<0, ScalarField> var(variable_name, m_solution_tag);
    ConfigurableConstant<Real> value("value", "Value for constant boundary condition");

    return create_proto_action("BC"+region_name+variable_name, nodes_expression(dirichlet(var) = value));
  }

  boost::shared_ptr< Action > create_constant_vector_bc(const std::string& region_name, const std::string& variable_name)
  {
    FieldVariable<0, VectorField> var(variable_name, m_solution_tag);
    ConfigurableConstant<RealVector> value("value", "Value for constant boundary condition");

    return create_proto_action("BC"+region_name+variable_name, nodes_expression(dirichlet(var) = value));
  }

  void add_bc_signature(SignalArgs& node)
  {
    SignalOptions options( node );

    options.add("region_name", std::string())
        .description("Default region name for this BC");

    options.add("variable_name", std::string())
        .description("Variable name for this BC");
  }

  void create_bc_action_signature(SignalArgs& node)
  {
    SignalOptions options( node );

    options.add("region_name", std::string())
        .description("Default region name for this BC");

    options.add("builder_name", std::string())
        .description("Name of the builder to create the BC component");
  }
  
  void add_bc_component_signature(SignalArgs& node)
  {
    SignalOptions options( node );

    options.add("region_name", std::string())
        .description("Default region name for this BC");

    options.add("variable_name", std::string())
        .description("Variable name for this BC");

    options.add("component", 0u)
        .description("Component of the vector for which the BC is applied");
  }

  // Checked access to the physical model
  physics::PhysModel& physical_model()
  {
    if(is_null(m_physical_model))
      throw SetupError(FromHere(), "Error accessing physical_model from " + m_component.uri().string());

    return *m_physical_model;
  }

  void configure_bc(Action& bc_action, const std::string& region_name)
  {
    std::vector<URI> bc_regions;
    // find the URIs of all the regions that match the region_name
    boost_foreach(const URI& region_uri, m_region_uris)
    {
      Handle< Component > region_comp = m_component.access_component(region_uri);
      if(!region_comp)
        throw SetupError(FromHere(), "No component found at " + region_uri.string() + " when reading regions from " + m_component.uri().string());
      Handle< Region > root_region(region_comp);
      if(!root_region)
        throw SetupError(FromHere(), "Component at " + region_uri.string() + " is not a region when reading regions from " + m_component.uri().string());

      Handle< Region > region = find_component_ptr_recursively_with_name<Region>(*root_region, region_name);
      if(region)
        bc_regions.push_back(region->uri());
    }

    // debug output
    if(bc_regions.empty())
    {
      CFdebug << "No default regions found for region name " << region_name << CFendl;
    }
    else
    {
      CFdebug << "Region name " << region_name << " resolved to:\n";
      boost_foreach(const URI& uri, bc_regions)
      {
        CFdebug << " " << uri.string() << CFendl;
      }
    }

    bc_action.options().set(solver::Tags::regions(), bc_regions);
    bc_action.configure_option_recursively(solver::Tags::physical_model(), m_physical_model);
  }

  ActionDirector& m_component;
  Handle<physics::PhysModel> m_physical_model;
  DirichletBC dirichlet;
  std::vector<URI> m_region_uris;
  std::string m_solution_tag;
};

BoundaryConditions::BoundaryConditions(const std::string& name) :
  ActionDirector(name),
  m_implementation( new Implementation(*this) )
{
  regist_signal( "add_constant_bc" )
    .connect( boost::bind( &BoundaryConditions::signal_create_constant_bc, this, _1 ) )
    .description("Create a constant Dirichlet BC")
    .pretty_name("Add Constant BC")
    .signature( boost::bind(&Implementation::add_bc_signature, m_implementation.get(), _1) );

  regist_signal( "add_constant_component_bc" )
    .connect( boost::bind( &BoundaryConditions::signal_create_constant_component_bc, this, _1 ) )
    .description("Create a constant Dirichlet BC for one component of a vector")
    .pretty_name("Add Constant Component BC")
    .signature( boost::bind(&Implementation::add_bc_component_signature, m_implementation.get(), _1) );

  regist_signal( "add_function_bc" )
    .connect( boost::bind( &BoundaryConditions::signal_create_function_bc, this, _1 ) )
    .description("Create a Dirichlet BC that can be set using an analytical function")
    .pretty_name("Add Function BC")
    .signature( boost::bind(&Implementation::add_bc_signature, m_implementation.get(), _1) );
    
  regist_signal( "create_bc_action" )
    .connect( boost::bind( &BoundaryConditions::signal_create_bc_action, this, _1 ) )
    .description("Create a boundary condition using the supplied builder name")
    .pretty_name("Create BC Action")
    .signature( boost::bind(&Implementation::create_bc_action_signature, m_implementation.get(), _1) );

  set_solution_tag(UFEM::Tags::solution());
}

BoundaryConditions::~BoundaryConditions()
{
}

Handle<common::Action> BoundaryConditions::add_constant_bc(const std::string& region_name, const std::string& variable_name)
{
  const VariablesDescriptor& descriptor = find_component_with_tag<VariablesDescriptor>(m_implementation->physical_model().variable_manager(), m_implementation->m_solution_tag);

  boost::shared_ptr< common::Action > result = descriptor.dimensionality(variable_name) == VariablesDescriptor::Dimensionalities::SCALAR ?
    m_implementation->create_constant_scalar_bc(region_name, variable_name) :
    m_implementation->create_constant_vector_bc(region_name, variable_name);


  add_component(result); // Append action

  m_implementation->configure_bc(*result, region_name);

  return Handle<Action>(result);
}

Handle<common::Action> BoundaryConditions::add_constant_bc(const std::string& region_name, const std::string& variable_name, const boost::any default_value)
{
  Handle<common::Action> result = add_constant_bc(region_name, variable_name);
  result->options().set("value", default_value);
  return result;
}

Handle< common::Action > BoundaryConditions::add_constant_component_bc(const std::string& region_name, const std::string& variable_name, const Uint component_idx, const Real default_value)
{
  FieldVariable<0, VectorField> var(variable_name, m_implementation->m_solution_tag);
  ConfigurableConstant<Real> value("value", "Value for constant boundary condition", default_value);

  boost::shared_ptr< common::Action > result = create_proto_action("BC"+region_name+variable_name,
                                                nodes_expression(m_implementation->dirichlet(var[component_idx]) = value));

  add_component(result);
  m_implementation->configure_bc(*result, region_name);

  return Handle<common::Action>(result);
}


Handle< common::Action > BoundaryConditions::add_function_bc(const std::string& region_name, const std::string& variable_name)
{
  Handle<ParsedFunctionExpression> result = create_component<ParsedFunctionExpression>("BC"+region_name+variable_name);

  FieldVariable<0, VectorField> var(variable_name, m_implementation->m_solution_tag);
  result->set_expression( nodes_expression( m_implementation->dirichlet(var) = result->vector_function() ) );

  m_implementation->configure_bc(*result, region_name);

  return result;
}

Handle< common::Action > BoundaryConditions::create_bc_action(const std::string& region_name, const std::string& builder_name)
{
  Handle<common::Action> result(create_component(builder_name+region_name, builder_name));
  m_implementation->configure_bc(*result, region_name);
  if(result->options().check("lss"))
    result->options().set("lss", options().option("lss").value());
  return result;
}


void BoundaryConditions::signal_create_constant_bc(SignalArgs& node)
{
  SignalOptions options( node );

  SignalFrame reply = node.create_reply(uri());
  SignalOptions reply_options(reply);
  reply_options.add("created_component", add_constant_bc(options.value<std::string>("region_name"), options.value<std::string>("variable_name"))->uri());
}

void BoundaryConditions::signal_create_function_bc ( SignalArgs& node )
{
  SignalOptions options( node );

  SignalFrame reply = node.create_reply(uri());
  SignalOptions reply_options(reply);
  reply_options.add("created_component", add_function_bc(options.value<std::string>("region_name"), options.value<std::string>("variable_name"))->uri());
}

void BoundaryConditions::signal_create_constant_component_bc(SignalArgs& node)
{
  SignalOptions options( node );

  SignalFrame reply = node.create_reply(uri());
  SignalOptions reply_options(reply);
  reply_options.add("created_component", add_constant_component_bc(options.value<std::string>("region_name"),
                                                                          options.value<std::string>("variable_name"),
                                                                          options.value<Uint>("component"))->uri());
}

void BoundaryConditions::signal_create_bc_action(SignalArgs& node)
{
  SignalOptions options( node );

  SignalFrame reply = node.create_reply(uri());
  SignalOptions reply_options(reply);
  reply_options.add("created_component", create_bc_action(options.value<std::string>("region_name"),
                                                                          options.value<std::string>("builder_name"))->uri());
}


void BoundaryConditions::set_solution_tag(const std::string& solution_tag)
{
  m_implementation->m_solution_tag = solution_tag;
}


} // UFEM
} // cf3
