// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/BasicExceptions.hpp"
#include "common/OptionComponent.hpp"
#include "common/OptionT.hpp"
#include "common/OptionList.hpp"
#include "common/OptionURI.hpp"
#include "common/PropertyList.hpp"
#include "common/Signal.hpp"

#include "math/VariableManager.hpp"

#include "mesh/Domain.hpp"
#include "mesh/FieldManager.hpp"

#include "physics/PhysModel.hpp"

#include "solver/Solver.hpp"
#include "solver/Tags.hpp"

namespace cf3 {
namespace solver {
  
using namespace common;
using namespace mesh;

RegistTypeInfo<Solver, LibSolver> regist_Solver_type;

////////////////////////////////////////////////////////////////////////////////

struct Solver::Implementation {

  Implementation(Component& component) :
    m_component(component),
    m_field_manager(*component.create_static_component<FieldManager>("FieldManager"))
  {
  }

  void trigger_domain()
  {
    // This will set the domain to null if the URI is invalid,
    // errors are reported through domain() on access.
    // Rationale: the URI may be set before the domain is created
    m_domain = Handle<Domain>(m_component.access_component(m_domain_uri));
  }

  // Checked access to the domain
  Domain& domain()
  {
    if(is_null(m_domain))
    {
      Handle< Component > comp = m_component.access_component(m_domain_uri);

      if(!comp)
        throw SetupError(FromHere(), "No component found at URI " + m_domain_uri.string() + " when acessing domain from " + m_component.uri().string());

      m_domain = Handle<Domain>(comp);

      if(is_null(m_domain))
        throw SetupError(FromHere(), "Error while acessing domain from " + m_component.uri().string() + ": component at " + m_domain_uri.string() + " is not a domain");
    }

    return *m_domain;
  }

  Component& m_component;
  URI m_domain_uri;
  Handle<Domain> m_domain;
  FieldManager& m_field_manager;
};

////////////////////////////////////////////////////////////////////////////////

Solver::Solver ( const std::string& name  ) :
  ActionDirector ( name ),
  m_implementation(new Implementation(*this))
{
  mark_basic();

  // properties

  properties()["brief"] = std::string("Solver");
  properties()["description"] = std::string("");

  // options

  options().add(Tags::domain(), URI("cpath:../Domain"))
      .description("Domain to solve")
      .pretty_name("Domain")
      .link_to(&m_implementation->m_domain_uri)
      .attach_trigger(boost::bind(&Implementation::trigger_domain, m_implementation.get()));
      
  options().add(Tags::physical_model(), m_physics)
      .pretty_name("Physical Model")
      .description("Physical Model")
      .link_to(&m_physics)
      .attach_trigger(boost::bind(&Solver::trigger_physical_model, this));
}



Solver::~Solver()
{
}


void Solver::mesh_loaded(Mesh& mesh) {}

void Solver::mesh_changed(Mesh& mesh) {}

FieldManager& Solver::field_manager()
{
  return m_implementation->m_field_manager;
}


Domain& Solver::domain()
{
  return m_implementation->domain();
}

physics::PhysModel& Solver::physics()
{
  if(is_null(m_physics))
    throw SetupError(FromHere(), "No physical model configured for " + uri().string());

  return *m_physics;
}

void Solver::trigger_physical_model()
{
  m_implementation->m_field_manager.options().set("variable_manager", m_physics->variable_manager().handle<math::VariableManager>());
}



////////////////////////////////////////////////////////////////////////////////

} // solver
} // cf3
