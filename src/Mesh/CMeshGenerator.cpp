// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/OptionComponent.hpp"
#include "Common/OptionT.hpp"
#include "Common/OptionURI.hpp"
#include "Common/Core.hpp"
#include "Common/EventHandler.hpp"

#include "Common/XML/SignalOptions.hpp"

#include "Mesh/CMeshGenerator.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CMeshElements.hpp"

namespace CF {
namespace Mesh {

using namespace Common;
using namespace Common::XML;

////////////////////////////////////////////////////////////////////////////////

CMeshGenerator::CMeshGenerator ( const std::string& name  ) :
  CAction ( name ),
  m_name("mesh")
{
  mark_basic();

  m_options.add_option(OptionComponent<Component>::create("parent", &m_parent))
      ->description("Where the mesh will be generated into")
      ->pretty_name("Parent")
      ->mark_basic();

  m_options.add_option<OptionT<std::string> >("name", m_name)
      ->description("Name of the mesh that will be generated")
      ->pretty_name("Name")
      ->link_to(&m_name)
      ->mark_basic();

}

////////////////////////////////////////////////////////////////////////////////

void CMeshGenerator::mesh_loaded(CMesh& mesh)
{
  mesh.update_statistics();
  mesh.elements().update();

  // Raise an event to indicate that a mesh was loaded happened
  SignalOptions options;
  options.add_option< OptionURI >("mesh_uri", mesh.uri());

  SignalArgs f= options.create_frame();
  Core::instance().event_handler().raise_event( "mesh_loaded", f );
}

////////////////////////////////////////////////////////////////////////////////

CMeshGenerator::~CMeshGenerator()
{
}

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
