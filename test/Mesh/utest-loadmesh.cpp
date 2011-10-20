// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for cf3::Mesh::LoadMesh"

#include <boost/test/unit_test.hpp>


#include "common/FindComponents.hpp"
#include "common/Log.hpp"
#include "common/CLink.hpp"
#include "common/Foreach.hpp"
#include "common/Core.hpp"
#include "common/CRoot.hpp"
#include "common/OptionArray.hpp"
#include "common/OptionT.hpp"
#include "common/OptionURI.hpp"

#include "common/XML/SignalFrame.hpp"
#include "common/XML/SignalOptions.hpp"

#include "Mesh/CDomain.hpp"
#include "Mesh/CMeshWriter.hpp"

#include "Mesh/LoadMesh.hpp"

using namespace cf3;
using namespace cf3::common;
using namespace cf3::common::XML;
using namespace cf3::Mesh;

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( LoadMesh_Suite )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( constructor )
{
  BOOST_CHECK(true);

  CDomain::Ptr domain = Core::instance().root().create_component_ptr<CDomain>("Domain");

  BOOST_CHECK(true);

  LoadMesh::Ptr load_mesh = Core::instance().root().create_component_ptr<LoadMesh>("load_mesh");

  BOOST_CHECK(true);

  SignalFrame frame;
  SignalOptions options;

  // everything is OK
  std::vector<URI> files;
  files.push_back( "file:rotation-tg-p1.neu" );
  options.add_option< OptionURI >("location", URI( domain->uri().string()) );
  options.add_option< OptionT<std::string> >("name", std::string("Mesh") );
  options.add_option< OptionArrayT<URI> >("files", files);

  frame = options.create_frame("Target", "//Root", "//Root");

  load_mesh->signal_load_mesh(frame);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( output )
{
  CDomain& domain = find_component_recursively<CDomain>(Core::instance().root());
  CMesh::Ptr mesh = domain.get_child_ptr_checked("Mesh")->as_ptr<CMesh>();
  CMeshWriter::Ptr mesh_writer = build_component_abstract_type<CMeshWriter> ( "CF.Mesh.Gmsh.CWriter", "GmshWriter" );
  mesh_writer->write_from_to(*mesh,"utest-loadmesh-result.msh");
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

