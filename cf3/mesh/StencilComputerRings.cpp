// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <set>

#include <boost/tuple/tuple.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "common/Foreach.hpp"
#include "common/Log.hpp"
#include "common/Builder.hpp"
#include "common/FindComponents.hpp"
#include "common/OptionList.hpp"
#include "common/OptionT.hpp"

#include "mesh/Connectivity.hpp"
#include "mesh/StencilComputerRings.hpp"
#include "mesh/NodeElementConnectivity.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Region.hpp"
#include "mesh/Space.hpp"
#include "mesh/Elements.hpp"
#include "mesh/ElementType.hpp"
#include "mesh/Dictionary.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {

  using namespace common;

////////////////////////////////////////////////////////////////////////////////

cf3::common::ComponentBuilder < StencilComputerRings, StencilComputer, LibMesh > StencilComputerRings_Builder;

//////////////////////////////////////////////////////////////////////////////

StencilComputerRings::StencilComputerRings( const std::string& name )
  : StencilComputer(name), m_nb_rings(0)
{
  options().add("nb_rings", m_nb_rings)
      .description("Number of neighboring rings of elements in stencil")
      .pretty_name("Number of Rings")
      .link_to(&m_nb_rings);
}

//////////////////////////////////////////////////////////////////////////////

void StencilComputerRings::compute_stencil(const SpaceElem& element, std::vector<SpaceElem>& stencil)
{
  std::set<SpaceElem> included;
  visited_nodes.clear();
  compute_neighbors(included,element);

  if (included.size() < m_min_stencil_size)
    CFwarn << "stencil size computed for element " << element << " is " << included.size() <<". This is smaller than the requested " << m_min_stencil_size << "." << CFendl;

  stencil.clear(); stencil.reserve(included.size());
  boost_foreach (const SpaceElem& elem, included)
    stencil.push_back(elem);
}

////////////////////////////////////////////////////////////////////////////////

void StencilComputerRings::compute_neighbors(std::set<SpaceElem>& included, const SpaceElem& element, const Uint level)
{
  included.insert(element);

  if (level < m_nb_rings)
  {
    boost_foreach(Uint node_idx, element.nodes())
    {
      boost_foreach(const SpaceElem& neighbor_elem, m_dict->connectivity()[node_idx])
      {
        compute_neighbors(included,neighbor_elem,level+1);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3
