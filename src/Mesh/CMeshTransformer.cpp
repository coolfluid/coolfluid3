// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/OptionComponent.hpp"
#include "Common/Foreach.hpp"
#include "Common/FindComponents.hpp"
#include "Common/CBuilder.hpp"

#include "Mesh/CMeshTransformer.hpp"
#include "Mesh/CMesh.hpp"

namespace CF {
namespace Mesh {

using namespace Common;

Common::ComponentBuilder < CMeshTransformer, CMeshTransformer, LibMesh> CMeshTransformer_Builder;

////////////////////////////////////////////////////////////////////////////////

CMeshTransformer::CMeshTransformer ( const std::string& name  ) :
  CAction ( name )
{
  mark_basic();

  m_options.add_option(OptionComponent<CMesh>::create("mesh","Mesh","The mesh to be transformed",&m_mesh))
    ->mark_basic();
}

////////////////////////////////////////////////////////////////////////////////

std::string CMeshTransformer::help() const
{
  return "  " + properties()["brief"].value<std::string>() + "\n" + properties()["description"].value<std::string>();
}

////////////////////////////////////////////////////////////////////////////////

void CMeshTransformer::set_mesh(CMesh::Ptr mesh)
{
  set_mesh(*mesh);
}

////////////////////////////////////////////////////////////////////////////////

void CMeshTransformer::set_mesh(CMesh& mesh)
{
  m_mesh=mesh.as_ptr<CMesh>();
  boost_foreach(CMeshTransformer& meshtransformer, find_components<CMeshTransformer>(*this))
    meshtransformer.set_mesh(mesh);

}

////////////////////////////////////////////////////////////////////////////////

CMeshTransformer::~CMeshTransformer()
{
}

////////////////////////////////////////////////////////////////////////////////

void CMeshTransformer::transform(CMesh::Ptr mesh)
{
  transform(*mesh);
}

////////////////////////////////////////////////////////////////////////////////

void CMeshTransformer::transform(CMesh& mesh)
{
  set_mesh(mesh);
  execute();
}

////////////////////////////////////////////////////////////////////////////////

void CMeshTransformer::execute()
{
  boost_foreach(CMeshTransformer& meshtransformer, find_components<CMeshTransformer>(*this))
    meshtransformer.transform(*m_mesh.lock());
}

//////////////////////////////////////////////////////////////////////////////


} // Mesh
} // CF
