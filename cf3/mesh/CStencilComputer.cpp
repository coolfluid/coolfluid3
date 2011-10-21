// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#include "common/Foreach.hpp"
#include "common/FindComponents.hpp"
#include "common/OptionT.hpp"
#include "common/OptionComponent.hpp"

#include "mesh/CStencilComputer.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/CElements.hpp"
#include "mesh/ElementType.hpp"
#include "mesh/Geometry.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {

  using namespace common;

//////////////////////////////////////////////////////////////////////////////

CStencilComputer::CStencilComputer( const std::string& name )
  : Component(name)
{

  m_options.add_option(OptionComponent<Mesh>::create("mesh", &m_mesh))
      ->description("Mesh to create octtree from")
      ->pretty_name("Mesh")
      ->attach_trigger(boost::bind(&CStencilComputer::configure_mesh,this))
      ->mark_basic();

  m_elements = create_component_ptr<CUnifiedData>("elements");

  m_min_stencil_size=1;
  m_options.add_option(OptionT<Uint>::create("stencil_size", m_min_stencil_size ))
      ->description("The minimum amount of cells in a stencil")
      ->pretty_name("Stencil Size")
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

void CStencilComputer::set_mesh(Mesh& mesh)
{
  m_mesh = mesh.as_ptr<Mesh>();
  configure_mesh();
}

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3
