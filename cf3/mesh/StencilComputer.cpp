// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include "common/Builder.hpp"
#include "common/Foreach.hpp"
#include "common/FindComponents.hpp"
#include "common/OptionList.hpp"
#include "common/OptionT.hpp"
#include "common/OptionComponent.hpp"

#include "mesh/StencilComputer.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Space.hpp"
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

  options().add("dict",m_dict)
      .description("Dictionary used to find the element")
      .link_to(&m_dict);

  m_min_stencil_size=1;
  options().add("stencil_size", m_min_stencil_size)
      .description("The minimum amount of cells in a stencil")
      .pretty_name("Stencil Size")
      .link_to(&m_min_stencil_size);

}

//////////////////////////////////////////////////////////////////////

cf3::common::ComponentBuilder < StencilComputerOneCell, StencilComputer, LibMesh > stencilcomputeronecell_Builder;

////////////////////////////////////////////////////////////////////////////////

StencilComputerOneCell::StencilComputerOneCell( const std::string& name )
  : StencilComputer(name)
{

}

////////////////////////////////////////////////////////////////////////////////

void StencilComputerOneCell::compute_stencil(const SpaceElem& element, std::vector<SpaceElem>& stencil)
{
  stencil.assign(1,element);
}

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3
