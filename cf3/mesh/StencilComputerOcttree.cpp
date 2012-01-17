// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/tuple/tuple.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "common/Foreach.hpp"
#include "common/Log.hpp"
#include "common/Builder.hpp"
#include "common/FindComponents.hpp"

#include "mesh/StencilComputerOcttree.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Region.hpp"
#include "mesh/Elements.hpp"
#include "mesh/ElementType.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Field.hpp"
#include "mesh/Octtree.hpp"
#include "common/OptionList.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {

  using namespace common;

////////////////////////////////////////////////////////////////////////////////

cf3::common::ComponentBuilder < StencilComputerOcttree, StencilComputer, LibMesh > StencilComputerOcttree_Builder;

//////////////////////////////////////////////////////////////////////////////

StencilComputerOcttree::StencilComputerOcttree( const std::string& name )
  : StencilComputer(name), m_dim(0), m_nb_elems_in_mesh(0)
{
  options().option("mesh").attach_trigger(boost::bind(&StencilComputerOcttree::configure_mesh,this));

  m_octtree = create_static_component<Octtree>("octtree");
  m_octtree->mark_basic();
}

//////////////////////////////////////////////////////////////////////

void StencilComputerOcttree::configure_mesh()
{
  if (is_null(m_mesh))
    throw SetupError(FromHere(), "Option \"mesh\" has not been configured");

  m_nb_elems_in_mesh = m_mesh->topology().recursive_filtered_elements_count(IsElementsVolume());
  m_dim = m_mesh->geometry_fields().coordinates().row_size();

  m_octtree->options().configure_option("mesh",m_mesh);
  m_octtree->create_octtree();
}

//////////////////////////////////////////////////////////////////////////////

void StencilComputerOcttree::compute_stencil(const Uint unified_elem_idx, std::vector<Uint>& stencil)
{
  std::vector<Uint> octtree_cell(3);
  RealVector centroid(m_dim);
  Handle< Component > component;
  Uint elem_idx;
  boost::tie(component,elem_idx) = unified_elements().location(unified_elem_idx);
  Elements& elements = dynamic_cast<Elements&>(*component);
  RealMatrix coordinates = elements.get_coordinates(elem_idx);
  elements.element_type().compute_centroid(coordinates,centroid);
  stencil.resize(0);
  if (m_octtree->find_octtree_cell(centroid,octtree_cell))
  {
    for (Uint ring=0; stencil.size() < m_min_stencil_size; ++ring)
    {
      m_octtree->gather_elements_around_idx(octtree_cell,ring,stencil);
      if (stencil.size() >= m_nb_elems_in_mesh )
        return;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3
