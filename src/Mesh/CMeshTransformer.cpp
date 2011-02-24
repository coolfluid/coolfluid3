// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/OptionComponent.hpp"

#include "Mesh/CMeshTransformer.hpp"
#include "Mesh/CMesh.hpp"

namespace CF {
namespace Mesh {

using namespace Common;

////////////////////////////////////////////////////////////////////////////////

CMeshTransformer::CMeshTransformer ( const std::string& name  ) :
  CAction ( name )
{
  mark_basic();

  properties().add_option(OptionComponent<CMesh>::create("Mesh","The mesh to be transformed",&m_mesh))
    ->mark_basic();
}

////////////////////////////////////////////////////////////////////////////////

void CMeshTransformer::set_mesh(CMesh::Ptr mesh)
{
  m_mesh=mesh;
}

////////////////////////////////////////////////////////////////////////////////

CMeshTransformer::~CMeshTransformer()
{
}

////////////////////////////////////////////////////////////////////////////////

void CMeshTransformer::transform(CMesh::Ptr mesh)
{
  set_mesh(mesh);
  execute();
}

//////////////////////////////////////////////////////////////////////////////


} // Mesh
} // CF
