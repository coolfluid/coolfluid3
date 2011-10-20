// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/tuple/tuple.hpp>

#include "common/Foreach.hpp"
#include "common/Log.hpp"
#include "common/CBuilder.hpp"
#include "common/FindComponents.hpp"

#include "Mesh/CStencilComputerOcttree.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/ElementType.hpp"
#include "Mesh/Geometry.hpp"
#include "Mesh/COcttree.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace Mesh {

  using namespace common;

////////////////////////////////////////////////////////////////////////////////

cf3::common::ComponentBuilder < CStencilComputerOcttree, CStencilComputer, LibMesh > CStencilComputerOcttree_Builder;

//////////////////////////////////////////////////////////////////////////////

CStencilComputerOcttree::CStencilComputerOcttree( const std::string& name )
  : CStencilComputer(name), m_dim(0), m_nb_elems_in_mesh(0)
{
  option("mesh").attach_trigger(boost::bind(&CStencilComputerOcttree::configure_mesh,this));

  m_octtree = create_static_component_ptr<COcttree>("octtree");
  m_octtree->mark_basic();
}

//////////////////////////////////////////////////////////////////////

void CStencilComputerOcttree::configure_mesh()
{
  if (m_mesh.expired())
    throw SetupError(FromHere(), "Option \"mesh\" has not been configured");

  m_nb_elems_in_mesh = m_mesh.lock()->topology().recursive_filtered_elements_count(IsElementsVolume());
  m_dim = m_mesh.lock()->geometry().coordinates().row_size();

  m_octtree->configure_option("mesh",m_mesh.lock()->uri());
  m_octtree->create_octtree();
}

//////////////////////////////////////////////////////////////////////////////

void CStencilComputerOcttree::compute_stencil(const Uint unified_elem_idx, std::vector<Uint>& stencil)
{
  std::vector<Uint> octtree_cell(3);
  RealVector centroid(m_dim);
  Component::Ptr component;
  Uint elem_idx;
  boost::tie(component,elem_idx) = unified_elements().location(unified_elem_idx);
  CElements& elements = component->as_type<CElements>();
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

} // Mesh
} // cf3
