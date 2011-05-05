// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#include "Common/Foreach.hpp"
#include "Common/FindComponents.hpp"
#include "Common/OptionT.hpp"
#include "Common/OptionComponent.hpp"

#include "Mesh/CStencilComputer.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/ElementType.hpp"
#include "Mesh/CNodes.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

  using namespace Common;

//////////////////////////////////////////////////////////////////////////////

CStencilComputer::CStencilComputer( const std::string& name )
  : Component(name)
{
  
  m_properties.add_option(OptionComponent<CMesh>::create("mesh","Mesh","Mesh to create octtree from",&m_mesh))
    ->attach_trigger(boost::bind(&CStencilComputer::configure_mesh,this))
    ->mark_basic();

  m_elements = create_component_ptr<CUnifiedData>("elements");
  
  m_min_stencil_size=1;
  m_properties.add_option(OptionT<Uint>::create("stencil_size","Stencil Size","The minimum amount of cells in a stencil", m_min_stencil_size ))
    ->link_to(&m_min_stencil_size);
  
}

//////////////////////////////////////////////////////////////////////

void CStencilComputer::configure_mesh()
{
  if (m_mesh.expired())
    throw SetupError(FromHere(), "Option \"mesh\" has not been configured");
  
  boost_foreach (CElements& elements, find_components_recursively_with_filter<CElements>(*m_mesh.lock(),IsElementsVolume()))
    m_elements->add(elements);
}

////////////////////////////////////////////////////////////////////////////////

void CStencilComputer::set_mesh(CMesh& mesh)
{
  m_mesh = mesh.as_ptr<CMesh>();
  configure_mesh();
}

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
