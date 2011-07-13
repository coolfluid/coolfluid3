// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/Log.hpp"
#include "Common/OptionComponent.hpp"
#include "Common/URI.hpp"

#include "Mesh/CRegion.hpp"

#include "Solver/CPhysicalModel.hpp"

#include "CProtoAction.hpp"
#include "Expression.hpp"

namespace CF {
namespace Solver {
namespace Actions {
namespace Proto {

using namespace Common;
using namespace Mesh;

ComponentBuilder < CProtoAction, CAction, LibSolver > CProtoAction_Builder;

struct CProtoAction::Implementation
{
  Implementation(Component& comp) :
    m_component(comp)
  {
    m_component.options().add_option( OptionComponent<CPhysicalModel>::create("physical_model",&m_physical_model))
        ->set_description("Physical model")
        ->set_pretty_name("Physical Model")
        ->mark_basic()
        ->attach_trigger(boost::bind(&Implementation::trigger_physical_model, this));

    m_component.options().add_option( OptionComponent<CRegion>::create("region", &m_region))
        ->set_description("Region over which the action executes")
        ->set_pretty_name("Region")
        ->mark_basic();
  }

  void trigger_physical_model()
  {
    if(m_expression && !m_physical_model.expired())
      m_expression->register_variables(*m_physical_model.lock());
  }

  Expression::Ptr m_expression;
  Component& m_component;

  boost::weak_ptr<CPhysicalModel> m_physical_model;
  boost::weak_ptr<CRegion> m_region;
};

CProtoAction::CProtoAction(const std::string& name) :
  CAction(name),
  m_implementation(new Implementation(*this))
{
}

CProtoAction::~CProtoAction()
{
}

void CProtoAction::execute()
{
  if(m_implementation->m_region.expired())
    throw SetupError(FromHere(), "Region is not set for action " + uri().string());

  CFdebug << "Running action " << uri().string() << " on region " << m_implementation->m_region.lock()->uri().string() << CFendl;

  m_implementation->m_expression->loop(*m_implementation->m_region.lock());
}

void CProtoAction::set_expression(const boost::shared_ptr< Expression >& expression)
{
  m_implementation->m_expression = expression;
  expression->add_options(options());
  m_implementation->trigger_physical_model();
}

} // namespace Proto
} // namespace Actions
} // namespace Solver
} // namespace CF
