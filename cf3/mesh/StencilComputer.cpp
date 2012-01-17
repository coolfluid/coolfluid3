// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include "common/Foreach.hpp"
#include "common/FindComponents.hpp"
#include "common/OptionList.hpp"
#include "common/OptionT.hpp"
#include "common/OptionComponent.hpp"

#include "mesh/StencilComputer.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Elements.hpp"
#include "mesh/ElementType.hpp"
#include "mesh/Dictionary.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {

  using namespace common;

//////////////////////////////////////////////////////////////////////////////

StencilComputer::StencilComputer( const std::string& name )
  : Component(name)
{

  options().add_option("mesh", m_mesh)
      .description("Mesh to create octtree from")
      .pretty_name("Mesh")
      .attach_trigger(boost::bind(&StencilComputer::configure_mesh,this))
      .mark_basic()
      .link_to(&m_mesh);

  m_elements = create_component<UnifiedData>("elements");

  m_min_stencil_size=1;
  options().add_option("stencil_size", m_min_stencil_size)
      .description("The minimum amount of cells in a stencil")
      .pretty_name("Stencil Size")
      .link_to(&m_min_stencil_size);

}

//////////////////////////////////////////////////////////////////////

void StencilComputer::configure_mesh()
{
  if (is_null(m_mesh))
    throw SetupError(FromHere(), "Option \"mesh\" has not been configured");

  boost_foreach (Elements& elements, find_components_recursively_with_filter<Elements>(*m_mesh,IsElementsVolume()))
    m_elements->add(elements);
}

////////////////////////////////////////////////////////////////////////////////

void StencilComputer::set_mesh(Mesh& mesh)
{
  m_mesh = Handle<Mesh>(mesh.handle<Component>());
  configure_mesh();
}

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3
