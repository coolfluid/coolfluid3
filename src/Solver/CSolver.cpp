// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/BasicExceptions.hpp"
#include "Common/OptionT.hpp"
#include "Common/OptionURI.hpp"
#include "Common/Signal.hpp"

#include "Mesh/CDomain.hpp"

#include "Solver/CSolver.hpp"

namespace CF {
namespace Solver {

using namespace Common;
using namespace Mesh;

////////////////////////////////////////////////////////////////////////////////

struct CSolver::Implementation {

  Implementation(Component& component) : m_component(component) {}

  void trigger_domain()
  {
    // This will set the domain to null if the URI is invalid,
    // errors are reported through domain() on access.
    // Rationale: the URI may be set before the domain is created
    m_domain = boost::dynamic_pointer_cast<CDomain>(m_component.access_component_ptr(m_domain_uri));
  }

  // Checked access to the domain
  CDomain& domain()
  {
    if(m_domain.expired())
    {
      Component::Ptr comp = m_component.access_component_ptr(m_domain_uri);

      if(!comp)
        throw SetupError(FromHere(), "No component found at URI " + m_domain_uri.string() + " when acessing domain from " + m_component.uri().string());

      m_domain = boost::dynamic_pointer_cast<CDomain>(comp);

      if(m_domain.expired())
        throw SetupError(FromHere(), "Error while acessing domain from " + m_component.uri().string() + ": component at " + m_domain_uri.string() + " is not a domain");
    }

    return *m_domain.lock();
  }

  Component& m_component;
  URI m_domain_uri;
  boost::weak_ptr<CDomain> m_domain;

};

////////////////////////////////////////////////////////////////////////////////

CSolver::CSolver ( const std::string& name  ) :
  CActionDirector ( name ),
  m_implementation(new Implementation(*this))
{
  mark_basic();

  // properties

  m_properties["brief"] = std::string("Solver");
  m_properties["description"] = std::string("");

  // options

  m_options.add_option< OptionURI > ("domain", URI("cpath:../Domain"))
      ->set_description("Domain to solve")
      ->set_pretty_name("Domain")
      ->link_to(&m_implementation->m_domain_uri)
      ->attach_trigger(boost::bind(&Implementation::trigger_domain, m_implementation.get()));

}



CSolver::~CSolver()
{
}


void CSolver::mesh_changed(CMesh& mesh)
{
}


CDomain& CSolver::domain()
{
  return m_implementation->domain();
}

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF
