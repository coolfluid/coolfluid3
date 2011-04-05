// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Mesh/CRegion.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CMeshElements.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

  using namespace Common;
    
////////////////////////////////////////////////////////////////////////////////

CMeshElements::CMeshElements(const std::string& name) :
  CUnifiedData<CEntities>(name)
{
}

////////////////////////////////////////////////////////////////////////////////

void CMeshElements::update()
{
  CMesh& mesh = find_parent_component<CMesh>(*this);
  boost_foreach(CEntities& elements, find_components_recursively<CEntities>(mesh.topology()))
    add(elements);
}

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
