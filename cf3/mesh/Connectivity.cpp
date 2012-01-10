// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/OptionList.hpp"
#include "common/OptionT.hpp"
#include "common/FindComponents.hpp"
#include "common/Link.hpp"
#include "common/Group.hpp"
#include "common/Log.hpp"
#include "common/Builder.hpp"


#include "mesh/Connectivity.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/MeshElements.hpp"

namespace cf3 {
namespace mesh {

using namespace common;

common::ComponentBuilder < Connectivity , Component, LibMesh > Connectivity_Builder;

////////////////////////////////////////////////////////////////////////////////

Connectivity::Connectivity ( const std::string& name ) :
  common::Table<Uint>(name)
{
}

////////////////////////////////////////////////////////////////////////////////

UnifiedData& Connectivity::lookup()
{
  return *m_lookup;
}

////////////////////////////////////////////////////////////////////////////////

UnifiedData& Connectivity::create_lookup()
{
  if (is_not_null(m_lookup))
  {
    if (is_not_null(get_child(m_lookup->name())))
    {
      remove_component(m_lookup->name());
    }
  }
  if (is_not_null(m_lookup_link))
  {
    if (is_not_null(get_child(m_lookup_link->name())))
    {
      remove_component(*m_lookup_link);
    }
  }
  set_lookup( *create_component<UnifiedData>("lookup") );
  return lookup();
}

////////////////////////////////////////////////////////////////////////////////

void Connectivity::set_lookup(UnifiedData& lookup)
{
  if (is_not_null(m_lookup))
  {
    if (is_not_null(get_child(m_lookup->name())))
    {
      remove_component(*m_lookup);
    }
  }
  m_lookup = Handle<UnifiedData>(lookup.handle<Component>());

  m_lookup = lookup.handle<UnifiedData>();
  m_lookup_link = create_component<Link>("lookup_link");
  m_lookup_link->link_to(lookup);
}

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3
