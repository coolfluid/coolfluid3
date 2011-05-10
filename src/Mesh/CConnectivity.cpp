// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/OptionT.hpp"
#include "Common/FindComponents.hpp"
#include "Common/CLink.hpp"
#include "Common/CGroup.hpp"
#include "Common/Log.hpp"
#include "Common/CBuilder.hpp"


#include "Mesh/CConnectivity.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CMeshElements.hpp"

namespace CF {
namespace Mesh {

using namespace Common;

Common::ComponentBuilder < CConnectivity , Component, LibMesh > CConnectivity_Builder;

////////////////////////////////////////////////////////////////////////////////

CConnectivity::CConnectivity ( const std::string& name ) : 
  CTable<Uint>(name)
{
}

////////////////////////////////////////////////////////////////////////////////

CUnifiedData& CConnectivity::lookup()
{
  return *m_lookup;
}

////////////////////////////////////////////////////////////////////////////////

CUnifiedData& CConnectivity::create_lookup()
{
  if (is_not_null(m_lookup))
  {
    if (is_not_null(get_child_ptr(m_lookup->name())))
    {
      remove_component(m_lookup->name());
    }
  }
  if (is_not_null(m_lookup_link))
  {
    if (is_not_null(get_child_ptr(m_lookup_link->name())))
    {
      remove_component(*m_lookup_link);
    }
  }

  return *create_component_ptr<CUnifiedData>("lookup");
}

////////////////////////////////////////////////////////////////////////////////

void CConnectivity::set_lookup(CUnifiedData& lookup)
{
  if (is_not_null(m_lookup))
  {
    if (is_not_null(get_child_ptr(m_lookup->name())))
    {
      remove_component(*m_lookup);
    }
  }
  m_lookup = lookup.as_ptr<CUnifiedData>();
  m_lookup_link = create_component_ptr<CLink>("lookup");
  m_lookup_link->link_to(lookup);
}

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
