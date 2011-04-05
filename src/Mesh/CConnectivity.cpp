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
  m_connected = create_static_component<CGroup>("connected");
}

////////////////////////////////////////////////////////////////////////////////

void CConnectivity::add(Component& new_connected)
{
  ComponentIterator<Component> it = m_connected->begin();
  ComponentIterator<Component> it_end = m_connected->end();
  bool found = false;
  Uint count=0;
  for (; it != it_end; ++it, ++count )
  {
    if (it->follow() == new_connected.follow())
    {
      found = true;
      break;
    }
  }
  if (found == false)
    m_connected->create_component<CLink>("link_"+Common::to_str(count))->link_to(new_connected);
}

////////////////////////////////////////////////////////////////////////////////

void CConnectivity::build_connectivity()
{
  
}



////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
