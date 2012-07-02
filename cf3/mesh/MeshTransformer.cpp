// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/OptionComponent.hpp"
#include "common/Foreach.hpp"
#include "common/FindComponents.hpp"
#include "common/Builder.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"
#include "common/Signal.hpp"
#include "common/XML/SignalOptions.hpp"

#include "mesh/MeshTransformer.hpp"
#include "mesh/Mesh.hpp"

namespace cf3 {
namespace mesh {

using namespace common;
using namespace common::XML;

common::ComponentBuilder < MeshTransformer, MeshTransformer, LibMesh> MeshTransformer_Builder;

////////////////////////////////////////////////////////////////////////////////

MeshTransformer::MeshTransformer ( const std::string& name  ) :
  Action ( name )
{
  mark_basic();

  options().add("mesh", m_mesh)
      .description( "The mesh to be transformed" )
      .pretty_name( "Mesh" )
      .mark_basic()
      .link_to(&m_mesh);
}

////////////////////////////////////////////////////////////////////////////////

std::string MeshTransformer::help() const
{
  return "  " + properties().value<std::string>("brief") + "\n" +
      properties().value<std::string>("description");
}

////////////////////////////////////////////////////////////////////////////////

void MeshTransformer::set_mesh(Handle< Mesh > mesh)
{
  set_mesh(*mesh);
}

////////////////////////////////////////////////////////////////////////////////

void MeshTransformer::set_mesh(Mesh& mesh)
{
  m_mesh=Handle<Mesh>(mesh.handle<Component>());
  boost_foreach(MeshTransformer& meshtransformer, find_components<MeshTransformer>(*this))
    meshtransformer.set_mesh(mesh);

}

////////////////////////////////////////////////////////////////////////////////

MeshTransformer::~MeshTransformer()
{
}

////////////////////////////////////////////////////////////////////////////////

void MeshTransformer::transform(Handle< Mesh > mesh)
{
  transform(*mesh);
}

////////////////////////////////////////////////////////////////////////////////

void MeshTransformer::transform(Mesh& mesh)
{
  set_mesh(mesh);
  execute();
}

////////////////////////////////////////////////////////////////////////////////

void MeshTransformer::execute()
{
  boost_foreach(MeshTransformer& meshtransformer, find_components<MeshTransformer>(*this))
    meshtransformer.transform(*m_mesh);
}

//////////////////////////////////////////////////////////////////////////////


} // mesh
} // cf3
