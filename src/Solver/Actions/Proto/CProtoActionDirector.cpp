// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/FindComponents.hpp"
#include "Common/OptionComponent.hpp"
#include "Common/OptionT.hpp"
#include "Common/URI.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"

#include "Solver/CPhysicalModel.hpp"

#include "CProtoActionDirector.hpp"

namespace CF {
namespace Solver {
namespace Actions {
namespace Proto {

using namespace Common;
using namespace Mesh;

struct CProtoActionDirector::Implementation
{
  Implementation(Component& component) :
    m_component(component)
  {
    m_component.options().add_option( OptionComponent<CPhysicalModel>::create("physical_model", &m_physical_model))
        ->set_description("Physical Model")
        ->mark_basic()
        ->attach_trigger(boost::bind(&Implementation::trigger_physical_model, this));

    m_component.options().add_option( OptionComponent<CRegion>::create("region", &m_region))
        ->set_description("Region over which the action executes")
        ->set_pretty_name("Region")
        ->mark_basic()
        ->attach_trigger(boost::bind(&Implementation::trigger_region, this));

    m_component.options().add_option< OptionT<bool> >("propagate_region", true )
        ->set_description("If true, changes to the region are propagated to the children of this component")
        ->set_pretty_name("Propagate Region")
        ->link_to(&m_propagate_region);
  }

  void trigger_region()
  {
    if(!m_propagate_region || m_region.expired())
      return;

    BOOST_FOREACH(Component& child, m_component)
    {
      if(child.options().check("region"))
        child.configure_option("region", m_component.option("region").value());
    }
  }

  void trigger_physical_model()
  {
    if(m_physical_model.expired())
      return;

    BOOST_FOREACH(Component& child, m_component)
    {
      if(child.options().check("physical_model"))
        child.configure_option("physical_model", m_component.option("physical_model").value());
    }
  }

  Component& m_component;
  boost::weak_ptr<CPhysicalModel> m_physical_model;
  boost::weak_ptr<CRegion> m_region;
  bool m_propagate_region;
};

CProtoActionDirector::CProtoActionDirector(const std::string& name) :
  CActionDirector(name),
  m_implementation(new CProtoActionDirector::Implementation(*this))
{
}

CProtoActionDirector::~CProtoActionDirector()
{
}

CAction& CProtoActionDirector::add_action(const std::string& name, const boost::shared_ptr< Expression >& expression)
{
  CProtoAction& action = create_component<CProtoAction>(name);

  action.set_expression(expression);

  m_implementation->trigger_physical_model();
  m_implementation->trigger_region();

  return action;
}

CPhysicalModel& CProtoActionDirector::physical_model()
{
  if(m_implementation->m_physical_model.expired())
    throw SetupError(FromHere(), "Physical model not set for component at " + uri().string());

  return *m_implementation->m_physical_model.lock();
}


} // namespace Proto
} // namespace Actions
} // namespace Solver
} // namespace CF
