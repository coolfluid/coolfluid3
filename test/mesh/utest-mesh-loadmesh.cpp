// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for cf3::mesh::LoadMesh"

#include <boost/test/unit_test.hpp>


#include "common/FindComponents.hpp"
#include "common/Log.hpp"
#include "common/OptionList.hpp"
#include "common/Link.hpp"
#include "common/Foreach.hpp"
#include "common/Core.hpp"
#include "common/OptionArray.hpp"
#include "common/OptionT.hpp"
#include "common/OptionURI.hpp"

#include "common/XML/SignalFrame.hpp"
#include "common/XML/SignalOptions.hpp"

#include "mesh/Domain.hpp"
#include "mesh/MeshWriter.hpp"

#include "mesh/LoadMesh.hpp"

using namespace cf3;
using namespace cf3::common;
using namespace cf3::common::XML;
using namespace cf3::mesh;

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( LoadMesh_Suite )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( constructor )
{
  BOOST_CHECK(true);

  Handle<Domain> domain = Core::instance().root().create_component<Domain>("Domain");

  BOOST_CHECK(true);

  Handle<LoadMesh> load_mesh = Core::instance().root().create_component<LoadMesh>("load_mesh");

  BOOST_CHECK(true);

  SignalFrame frame;
  SignalOptions options;

  // everything is OK
  std::vector<URI> files;
  files.push_back( "file:../../resources/rotation-tg-p1.neu" );
  options.add("mesh", domain->uri()/URI("Mesh") );
  options.add("files", files);

  frame = options.create_frame("Target", "/", "/");

  load_mesh->signal_load_mesh(frame);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( output )
{
  Domain& domain = find_component_recursively<Domain>(Core::instance().root());
  Handle< Mesh > mesh(domain.get_child_checked("Mesh"));
  boost::shared_ptr< MeshWriter > mesh_writer = build_component_abstract_type<MeshWriter> ( "cf3.mesh.gmsh.Writer", "GmshWriter" );
  mesh_writer->write_from_to(*mesh,"utest-loadmesh-result.msh");
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

