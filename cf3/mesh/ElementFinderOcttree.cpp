// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include "common/Builder.hpp"
#include "common/FindComponents.hpp"
#include "common/OptionList.hpp"
#include "common/OptionT.hpp"
#include "common/OptionComponent.hpp"

#include "mesh/Octtree.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Elements.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Space.hpp"
#include "mesh/ElementFinderOcttree.hpp"


#include "common/OSystem.hpp"
#include "common/OSystemLayer.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {

  using namespace common;

//////////////////////////////////////////////////////////////////////////////

cf3::common::ComponentBuilder < ElementFinderOcttree, ElementFinder, LibMesh > ElementFinderOcttree_Builder;

////////////////////////////////////////////////////////////////////////////////

ElementFinderOcttree::ElementFinderOcttree(const std::string &name) : ElementFinder(name)
{
  options().option("dict").attach_trigger( boost::bind( &ElementFinderOcttree::configure_octtree, this ) );
}

////////////////////////////////////////////////////////////////////////////////

void ElementFinderOcttree::configure_octtree()
{
  Handle<Mesh> mesh = find_parent_component_ptr<Mesh>(*m_dict);
//  std::cout << "configuring octtree for mesh" << mesh->uri() << std::endl;

//  std::cout << OSystem::instance().layer()->back_trace() << std::endl;

  if (is_null(mesh))
    throw SetupError(FromHere(),"Mesh was not found as parent of "+m_dict->uri().string());

  if (Handle<Component> found = mesh->get_child("octtree"))
    m_octtree = Handle<Octtree>(found);
  else
  {
    m_octtree = mesh->create_component<Octtree>("octtree");
    m_octtree->options().set("mesh",mesh);
  }
}

////////////////////////////////////////////////////////////////////////////////

bool ElementFinderOcttree::find_element(const RealVector& target_coord, SpaceElem& element)
{
  cf3_assert(m_octtree);
  bool found = m_octtree->find_element(target_coord,m_tmp);
  if (!found)
    return false;

  element = SpaceElem(*const_cast<Space*>(&m_dict->space(*m_tmp.comp)),m_tmp.idx);
  return true;
}

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3
