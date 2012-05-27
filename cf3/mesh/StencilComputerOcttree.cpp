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
  options().option("dict").attach_trigger( boost::bind( &StencilComputerOcttree::configure_octtree, this ) );
}

//////////////////////////////////////////////////////////////////////

void StencilComputerOcttree::configure_octtree()
{
  Handle<Mesh> mesh = find_parent_component_ptr<Mesh>(*m_dict);
  if (is_null(mesh))
    throw SetupError(FromHere(),"Mesh was not found as parent of "+m_dict->uri().string());

  m_nb_elems_in_mesh = mesh->topology().recursive_filtered_elements_count(IsElementsVolume(),true);
  m_dim = m_dict->coordinates().row_size();
  m_centroid.resize(m_dim);
  m_octtree_cell.resize(3);

  if (Handle<Component> found = mesh->get_child("octtree"))
    m_octtree = Handle<Octtree>(found);
  else
  {
    m_octtree = mesh->create_component<Octtree>("octtree");
    m_octtree->options().set("mesh",mesh);
  }
}

//////////////////////////////////////////////////////////////////////////////

void StencilComputerOcttree::compute_stencil(const SpaceElem& element, std::vector<SpaceElem>& stencil)
{
  cf3_assert(m_octtree);
  RealMatrix coordinates = element.comp->support().geometry_space().get_coordinates(element.idx);
  element.comp->support().element_type().compute_centroid(coordinates,m_centroid);
  m_stencil.resize(0);
  if (m_octtree->find_octtree_cell(m_centroid,m_octtree_cell))
  {
    for (Uint ring=0; m_stencil.size() < m_min_stencil_size; ++ring)
    {
      m_octtree->gather_elements_around_idx(m_octtree_cell,ring,m_stencil);
      if (m_stencil.size() >= m_nb_elems_in_mesh )
        break;
    }
  }
  stencil.resize(m_stencil.size());
  for (Uint e=0; e<stencil.size(); ++e)
  {
    stencil[e]=SpaceElem(*const_cast<Space*>(&m_dict->space(*m_stencil[e].comp)),m_stencil[e].idx);
  }
}

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3
