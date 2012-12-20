// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

#include "common/Builder.hpp"
#include "common/Log.hpp"
#include "common/OptionComponent.hpp"
#include "common/URI.hpp"

#include "mesh/Region.hpp"

#include "physics/PhysModel.hpp"

#include "solver/Tags.hpp"

#include "ProtoAction.hpp"
#include "Expression.hpp"

namespace cf3 {
namespace solver {
namespace actions {
namespace Proto {

using namespace common;
using namespace mesh;
using namespace physics;

ComponentBuilder < ProtoAction, common::Action, LibSolver > ProtoAction_Builder;

struct ProtoAction::Implementation
{
  Implementation(Component& comp, const Handle<PhysModel>& physical_model) :
    m_component(comp),
    m_physical_model(physical_model)
  {
    m_component.options().option(Tags::physical_model()).attach_trigger(boost::bind(&Implementation::trigger_physical_model, this));
  }

  void trigger_physical_model()
  {
    if(m_expression && is_not_null(m_physical_model))
    {
      CFdebug << "registering variables for " << m_component.uri().path() << CFendl;
      m_expression->register_variables(*m_physical_model);
      
      m_physics_links.clear();
      
      PhysicsConstantStorage& physics_constants = m_expression->physics_constants();
      for(PhysicsConstantStorage::ScalarsT::iterator it = physics_constants.scalars().begin(); it != physics_constants.scalars().end(); ++it)
      {
        m_physics_links.push_back(new PhysicsConstantLink(m_physical_model, it->first, it->second, m_component.uri().path()));
      }
    }
  }

  boost::shared_ptr< Expression > m_expression;
  Component& m_component;

  const Handle<PhysModel>& m_physical_model;

  struct PhysicsConstantLink
  {
    PhysicsConstantLink(const Handle<PhysModel>& physical_model, const std::string& constant_name, Real& value, const std::string& parent_path) :
      m_physical_model(physical_model),
      m_constant_name(constant_name),
      m_value(value),
      m_parent_path(parent_path)
    {
      cf3_assert(is_not_null(m_physical_model));
      m_trigger_id = m_physical_model->options().option(m_constant_name).attach_trigger_tracked(boost::bind(&PhysicsConstantLink::trigger, this));
      trigger();
    }
    
    ~PhysicsConstantLink()
    {
      if(is_not_null(m_physical_model))
	m_physical_model->options().option(m_constant_name).detach_trigger(m_trigger_id);
    }
    
    void trigger()
    {
      if(is_null(m_physical_model))
	throw common::SetupError(FromHere(), "Physical model for " + m_parent_path + " became null");

      m_value = m_physical_model->options().option(m_constant_name).value<Real>();
      
      CFdebug << "Updated physics constant " << m_constant_name << " for expression " << m_parent_path << " to value " << m_value << CFendl;
    }
    
    Handle<PhysModel> m_physical_model;
    const std::string m_constant_name;
    Real& m_value;
    const std::string m_parent_path;
    common::Option::TriggerID m_trigger_id;
  };
  
  boost::ptr_vector<PhysicsConstantLink> m_physics_links;
};

ProtoAction::ProtoAction(const std::string& name) :
  Action(name),
  m_implementation(new Implementation(*this, m_physical_model))
{
}

ProtoAction::~ProtoAction()
{
}

void ProtoAction::execute()
{
  if(m_loop_regions.empty())
    CFwarn << "No regions to loop over for action " << uri().string() << CFendl;

  boost_foreach(const Handle< Region >& region, m_loop_regions)
  {
    if(is_null(m_implementation->m_expression))
      throw SetupError(FromHere(), "Expression for ProtoAction " + uri().path() + " is not set.");
    CFdebug << "  Action " << name() << ": running over region " << region->uri().path() << CFendl;
    m_implementation->m_expression->loop(*region);
  }
}

void ProtoAction::set_expression(const boost::shared_ptr< Expression >& expression)
{
  m_implementation->m_expression = expression;
  expression->add_options(options());
  m_implementation->trigger_physical_model();
}

void ProtoAction::insert_field_info(std::map<std::string, std::string>& tags) const
{
  m_implementation->m_expression->insert_field_info(tags);
}


boost::shared_ptr< ProtoAction > create_proto_action(const std::string& name, const boost::shared_ptr< Expression >& expression)
{
  boost::shared_ptr<ProtoAction> action = allocate_component<ProtoAction>(name);
  action->set_expression(expression);

  return action;
}


} // namespace Proto
} // namespace actions
} // namespace solver
} // namespace cf3
