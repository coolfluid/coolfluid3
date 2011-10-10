// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/Log.hpp"
#include "Common/OptionComponent.hpp"
#include "Common/URI.hpp"

#include "Mesh/CRegion.hpp"

#include "Physics/PhysModel.hpp"

#include "Solver/Tags.hpp"

#include "CProtoAction.hpp"
#include "Expression.hpp"

namespace CF {
namespace Solver {
namespace Actions {
namespace Proto {

using namespace Common;
using namespace Mesh;
using namespace Physics;

ComponentBuilder < CProtoAction, CAction, LibSolver > CProtoAction_Builder;

struct CProtoAction::Implementation
{
  Implementation(Component& comp, const boost::weak_ptr<PhysModel>& physical_model) :
    m_component(comp),
    m_physical_model(physical_model)
  {
    m_component.option(Tags::physical_model()).attach_trigger(boost::bind(&Implementation::trigger_physical_model, this));
  }

  void trigger_physical_model()
  {
    if(m_expression && !m_physical_model.expired())
      m_expression->register_variables(*m_physical_model.lock());
  }

  Expression::Ptr m_expression;
  Component& m_component;

  const boost::weak_ptr<PhysModel>& m_physical_model;
};

CProtoAction::CProtoAction(const std::string& name) :
  Action(name),
  m_implementation(new Implementation(*this, m_physical_model))
{
}

CProtoAction::~CProtoAction()
{
}

void CProtoAction::execute()
{
  if(m_loop_regions.empty())
    CFwarn << "No regions to loop over for action " << uri().string() << CFendl;

  boost_foreach(const CRegion::Ptr& region, m_loop_regions)
  {
    m_implementation->m_expression->loop(*region);
  }
}

void CProtoAction::set_expression(const boost::shared_ptr< Expression >& expression)
{
  m_implementation->m_expression = expression;
  expression->add_options(options());
  m_implementation->trigger_physical_model();
}

void CProtoAction::insert_tags(std::set< std::string >& tags) const
{
  m_implementation->m_expression->insert_tags(tags);
}


CProtoAction::Ptr create_proto_action(const std::string& name, const boost::shared_ptr< Expression >& expression)
{
  CProtoAction::Ptr action = allocate_component<CProtoAction>(name);
  action->set_expression(expression);

  return action;
}


} // namespace Proto
} // namespace Actions
} // namespace Solver
} // namespace CF
