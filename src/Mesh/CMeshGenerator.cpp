// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Signal.hpp"
#include "Common/OptionComponent.hpp"
#include "Common/CreateComponent.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CMeshGenerator.hpp"

namespace CF {
namespace Mesh {

using namespace Common;
using namespace Common::XML;

////////////////////////////////////////////////////////////////////////////////

CMeshGenerator::CMeshGenerator ( const std::string& name  ) :
  CAction ( name )
{
  mark_basic();

  properties().add_option(OptionComponent<CMesh>::create("mesh","Mesh","The mesh to be generated",&m_mesh))
    ->mark_basic();

}

////////////////////////////////////////////////////////////////////////////////

CMeshGenerator::~CMeshGenerator()
{
}

////////////////////////////////////////////////////////////////////////////////

void CMeshGenerator::set_mesh(CMesh& mesh)
{
  m_mesh = mesh.as_ptr<CMesh>();
}

////////////////////////////////////////////////////////////////////////////////

CMesh::Ptr CMeshGenerator::generate()
{
  CMesh::Ptr mesh_ptr = allocate_component<CMesh>("mesh");
  set_mesh(*mesh_ptr);
  execute();
  return mesh_ptr;
}

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
