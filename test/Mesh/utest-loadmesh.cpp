// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::Mesh::LoadMesh"

#include <boost/test/unit_test.hpp>

#include "Common/CreateComponent.hpp"
#include "Common/FindComponents.hpp"
#include "Common/Log.hpp"
#include "Common/CLink.hpp"
#include "Common/Foreach.hpp"

#include "Common/XML/SignalOptions.hpp"

#include "Mesh/CDomain.hpp"
#include "Mesh/CMeshWriter.hpp"

#include "Mesh/LoadMesh.hpp"

using namespace CF;
using namespace CF::Common;
using namespace CF::Common::XML;
using namespace CF::Mesh;

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( LoadMesh_Suite )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( constructor )
{
  BOOST_CHECK(true);

  CDomain::Ptr domain = Core::instance().root().create_component<CDomain>("Domain");

  BOOST_CHECK(true);

  LoadMesh::Ptr load_mesh = Core::instance().root().create_component<LoadMesh>("load_mesh");

  BOOST_CHECK(true);

  SignalFrame frame("Target", "//Root", "//Root");
  SignalOptions options( frame );

  // everything is OK
  std::vector<URI> files;
  files.push_back( "file:rotation-tg-p1.neu" );
  options.add("Parent Component", URI( domain->full_path().string()) );
  options.add<std::string>("Name", std::string("Mesh") );
  options.add("Files", files, " ; ");

  load_mesh->signal_load_mesh(frame);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( output )
{
  CDomain& domain = find_component_recursively<CDomain>(Core::instance().root());
  CMesh::Ptr mesh = domain.get_child_ptr_checked("Mesh")->as_ptr<CMesh>();
  CMeshWriter::Ptr mesh_writer = create_component_abstract_type<CMeshWriter> ( "CF.Mesh.Gmsh.CWriter", "GmshWriter" );
  boost::filesystem::path file ("utest-loadmesh-result.msh");
  mesh_writer->write_from_to(mesh,file);
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

