// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/tuple/tuple.hpp>

#include "Common/Foreach.hpp"
#include "Common/Log.hpp"
#include "Common/CBuilder.hpp"
#include "Common/FindComponents.hpp"

#include "Mesh/CStencilComputerOcttree.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/ElementType.hpp"
#include "Mesh/CNodes.hpp"
#include "Mesh/COcttree.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

  using namespace Common;
  
////////////////////////////////////////////////////////////////////////////////

CF::Common::ComponentBuilder < CStencilComputerOcttree, CStencilComputer, LibMesh > CStencilComputerOcttree_Builder;

//////////////////////////////////////////////////////////////////////////////

CStencilComputerOcttree::CStencilComputerOcttree( const std::string& name )
  : CStencilComputer(name), m_dim(0), m_nb_elems_in_mesh(0)
{  
  property("mesh").as_option().attach_trigger(boost::bind(&CStencilComputerOcttree::configure_mesh,this));
    
  m_octtree = create_static_component<COcttree>("octtree");
  m_octtree->mark_basic();
}

//////////////////////////////////////////////////////////////////////

void CStencilComputerOcttree::configure_mesh()
{
  if (m_mesh.expired())
    throw SetupError(FromHere(), "Option \"mesh\" has not been configured");
    
  m_nb_elems_in_mesh = m_mesh.lock()->topology().recursive_filtered_elements_count(IsElementsVolume());
  m_dim = m_mesh.lock()->nodes().coordinates().row_size();
  
  m_octtree->configure_property("mesh",m_mesh.lock()->full_path());
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
} // CF
