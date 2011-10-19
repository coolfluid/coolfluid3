// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
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

namespace cf3 {
namespace Mesh {

using namespace common;

common::ComponentBuilder < CMeshTransformer, CMeshTransformer, LibMesh> CMeshTransformer_Builder;

////////////////////////////////////////////////////////////////////////////////

CMeshTransformer::CMeshTransformer ( const std::string& name  ) :
  CAction ( name )
{
  mark_basic();

  m_options.add_option(OptionComponent<CMesh>::create("mesh", &m_mesh))
      ->description( "The mesh to be transformed" )
      ->pretty_name( "Mesh" )
      ->mark_basic();
}

////////////////////////////////////////////////////////////////////////////////

std::string CMeshTransformer::help() const
{
  return "  " + properties().value<std::string>("brief") + "\n" +
      properties().value<std::string>("description");
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
} // cf3
