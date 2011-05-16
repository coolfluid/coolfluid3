// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Tests mesh interpolation"

#include <boost/test/unit_test.hpp>

#include "Common/FindComponents.hpp"
#include "Common/CreateComponent.hpp"
#include "Common/Core.hpp"
#include "Common/CRoot.hpp"

#include "Common/XML/SignalOptions.hpp"

#include "Mesh/LoadMesh.hpp"
#include "Mesh/CMeshReader.hpp"
#include "Mesh/CDomain.hpp"
#include "Mesh/Actions/CBubbleEnrich.hpp"
#include "Mesh/Actions/CBubbleRemove.hpp"
#include "Mesh/CCells.hpp"

using namespace boost;
using namespace CF;
using namespace CF::Common;
using namespace CF::Common::XML;
using namespace CF::Mesh;
using namespace CF::Mesh::Actions;

////////////////////////////////////////////////////////////////////////////////

struct BubbleEnrich_Fixture
{
  /// common setup for each test case
  BubbleEnrich_Fixture() {}

  /// common tear-down for each test case
  ~BubbleEnrich_Fixture() {}
};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( BubbleEnrich_TestSuite, BubbleEnrich_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( constructors )
{
  Core::instance().initiate(boost::unit_test::framework::master_test_suite().argc,
                            boost::unit_test::framework::master_test_suite().argv);

  CMeshTransformer::Ptr enricher =
      create_component_abstract_type<CMeshTransformer>("CF.Mesh.Actions.CBubbleEnrich","enricher");

  BOOST_CHECK_EQUAL(enricher->name(),"enricher");

  CMeshTransformer::Ptr remover =
      create_component_abstract_type<CMeshTransformer>("CF.Mesh.Actions.CBubbleRemove","remover");

  BOOST_CHECK_EQUAL(remover->name(),"remover");

  Core::instance().root().add_component(enricher);
  Core::instance().root().add_component(remover);

  // create a domain
  Core::instance().root().create_component_ptr<CDomain>("Domain");
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( read_mesh )
{
//  // create meshreader
//  CMeshReader::Ptr meshreader = create_component_abstract_type<CMeshReader>("CF.Mesh.Gmsh.CReader","meshreader");

//  BOOST_CHECK( true );

//  // load mesh
//  boost::filesystem::path file ("rectangle-tg-p1.msh");
//  CMesh::Ptr mesh = meshreader->create_mesh_from(file);

//  Core::instance().root().get_child_ptr("Domain")->add_component(mesh);

//	// Write the fields to file.
//  CMeshWriter::Ptr meshwriter = create_component_abstract_type<CMeshWriter>("CF.Mesh.Gmsh.CWriter","meshwriter");
//	boost::filesystem::path fp_source_out("source.msh");
//	boost::filesystem::path fp_interpolated("interpolated.msh");

//	BOOST_CHECK(true);

//	meshwriter->write_from_to(source,fp_source_out);


  // create the xml parameters for the signal


  SignalFrame frame("Target", "//Root", "//Root");
  SignalOptions options( frame );

  BOOST_CHECK(true);

  std::vector<URI> files;
  files.push_back( "file:rectangle-tg-p2.msh" );

  options.add("location", Core::instance().root().get_child("Domain").full_path() );
  options.add<std::string>("name", std::string("Mesh") );
  options.add("files", files, " ; ");

  // get the generic mesh loader from the Tools

  BOOST_CHECK(true);

  LoadMesh::Ptr load_mesh = Core::instance().root().get_child("Tools").get_child("LoadMesh").as_ptr_checked<LoadMesh>();
  cf_assert( is_not_null(load_mesh) );

  BOOST_CHECK(true);

  load_mesh->signal_load_mesh( frame );

  BOOST_CHECK(true);

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( enricher )
{
  BOOST_CHECK(true);

  CBubbleEnrich::Ptr enricher =
      Core::instance().root().get_child("enricher").as_ptr<CBubbleEnrich>();

  cf_assert( is_not_null(enricher) );

  Component::Ptr domain = Core::instance().root().get_child_ptr("Domain");

  CMesh::Ptr mesh = find_component_ptr<CMesh>(*domain);


//  CFinfo << "---------------------------------------------------" << CFendl;
//  CFinfo << Core::instance().root().tree() << CFendl;
//  CFinfo << "---------------------------------------------------" << CFendl;
  BOOST_CHECK(true);

  enricher->transform( mesh );

//  CFinfo << "---------------------------------------------------" << CFendl;
//  CFinfo << Core::instance().root().tree() << CFendl;
//  CFinfo << "---------------------------------------------------" << CFendl;

  // print connectivity
//  CMesh::Ptr mesh = find_component_ptr<CMesh>(domain);
//  boost_foreach(CElements& elements, find_components_recursively<CCells>(*mesh))
//  {
//    CFinfo << "---------------------------------------------------" << CFendl;
//    CFinfo << elements.node_connectivity().full_path().string()  << CFendl;    // loop on the elements
//    for ( Uint elem = 0; elem != elements.node_connectivity().size(); ++elem )
//    {
//      for ( Uint n = 0; n != elements.node_connectivity().row_size(); ++n )
//        CFinfo << " " << elements.node_connectivity().array()[elem][n];
//      CFinfo << CFendl;
//    }
//  }
  BOOST_CHECK(true);

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( remover )
{
  CDomain& domain = find_component_recursively<CDomain>(Core::instance().root());

  CBubbleRemove::Ptr remover =
      Core::instance().root().get_child_ptr("remover")->as_ptr<CBubbleRemove>();

  cf_assert( is_not_null(remover) );

  CMesh::Ptr mesh = find_component_ptr<CMesh>(domain);

  // print connectivity
//  CMesh::Ptr mesh = find_component_ptr<CMesh>(domain);
//  boost_foreach(CElements& elements, find_components_recursively<CCells>(*mesh))
//  {
//    CFinfo << "---------------------------------------------------" << CFendl;
//    CFinfo << elements.node_connectivity().full_path().string()  << CFendl;    // loop on the elements
//    for ( Uint elem = 0; elem != elements.node_connectivity().size(); ++elem )
//    {
//      for ( Uint n = 0; n != elements.node_connectivity().row_size(); ++n )
//        CFinfo << " " << elements.node_connectivity().array()[elem][n];
//      CFinfo << CFendl;
//    }
//  }

//  CFinfo << "---------------------------------------------------" << CFendl;
//  CFinfo << Core::instance().root().tree() << CFendl;
//  CFinfo << "---------------------------------------------------" << CFendl;
  BOOST_CHECK(true);

  remover->transform( mesh );
  BOOST_CHECK(true);

//  CFinfo << "---------------------------------------------------" << CFendl;
//  CFinfo << Core::instance().root().tree() << CFendl;
//  CFinfo << "---------------------------------------------------" << CFendl;


//  boost_foreach(CElements& elements, find_components_recursively<CCells>(*mesh))
//  {
//    CFinfo << "---------------------------------------------------" << CFendl;
//    CFinfo << elements.node_connectivity().full_path().string()  << CFendl;    // loop on the elements
//    for ( Uint elem = 0; elem != elements.node_connectivity().size(); ++elem )
//    {
//      for ( Uint n = 0; n != elements.node_connectivity().row_size(); ++n )
//        CFinfo << " " << elements.node_connectivity().array()[elem][n];
//      CFinfo << CFendl;
//    }
//  }

  Core::instance().terminate();

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

