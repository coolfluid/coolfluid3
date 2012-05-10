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
#include "common/OptionList.hpp"

#include "mesh/StencilComputerOcttree.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Region.hpp"
#include "mesh/Elements.hpp"
#include "mesh/ElementType.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Field.hpp"
#include "mesh/Space.hpp"
#include "mesh/Octtree.hpp"


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
}

//////////////////////////////////////////////////////////////////////

void StencilComputerOcttree::configure_mesh()
{
  if (is_null(m_mesh))
    throw SetupError(FromHere(), "Option \"mesh\" has not been configured");

  m_nb_elems_in_mesh = m_mesh->topology().recursive_filtered_elements_count(IsElementsVolume(),true);
  m_dim = m_mesh->geometry_fields().coordinates().row_size();

  if (Handle<Component> found = m_mesh->get_child("octtree"))
    m_octtree = Handle<Octtree>(found);
  else
  {
    m_octtree = m_mesh->create_component<Octtree>("octtree");
    m_octtree->options().configure_option("mesh",m_mesh);
  }
}

//////////////////////////////////////////////////////////////////////////////

void StencilComputerOcttree::compute_stencil(const Entity& element, std::vector<Entity>& stencil)
{
  if ( is_null(m_octtree) )
    throw SetupError(FromHere(), "Option \"mesh\" has not been configured");

  std::vector<Uint> octtree_cell(3);
  RealVector centroid(m_dim);

  RealMatrix coordinates = element.get_coordinates();
  element.element_type().compute_centroid(coordinates,centroid);
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
