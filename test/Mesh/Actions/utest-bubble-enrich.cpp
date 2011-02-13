// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Tests mesh interpolation"

#include <boost/test/unit_test.hpp>

#include "Common/ComponentPredicates.hpp"
#include "Common/CreateComponent.hpp"
#include "Common/Core.hpp"
#include "Common/CRoot.hpp"

#include "Mesh/LoadMesh.hpp"
#include "Mesh/CMeshReader.hpp"
#include "Mesh/CDomain.hpp"
#include "Mesh/Actions/CBubbleEnrich.hpp"
#include "Mesh/Actions/CBubbleRemove.hpp"
#include "Mesh/CCells.hpp"

using namespace boost;
using namespace CF;
using namespace CF::Common;
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
  CMeshTransformer::Ptr enricher =
      create_component_abstract_type<CMeshTransformer>("CF.Mesh.Actions.CBubbleEnrich","enricher");

  BOOST_CHECK_EQUAL(enricher->name(),"enricher");

  CMeshTransformer::Ptr remover =
      create_component_abstract_type<CMeshTransformer>("CF.Mesh.Actions.CBubbleRemove","remover");

  BOOST_CHECK_EQUAL(remover->name(),"remover");

  Core::instance().root()->add_component(enricher);
  Core::instance().root()->add_component(remover);

  // create a domain
  Core::instance().root()->create_component<CDomain>("Domain");
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

//  Core::instance().root()->get_child("Domain")->add_component(mesh);

//	// Write the fields to file.
//  CMeshWriter::Ptr meshwriter = create_component_abstract_type<CMeshWriter>("CF.Mesh.Gmsh.CWriter","meshwriter");
//	boost::filesystem::path fp_source_out("source.msh");
//	boost::filesystem::path fp_interpolated("interpolated.msh");

//	BOOST_CHECK(true);

//	meshwriter->write_from_to(source,fp_source_out);


  // create the xml parameters for the signal

  boost::shared_ptr<XmlDoc> doc = XmlOps::create_doc();
  XmlNode& node  = *XmlOps::goto_doc_node(*doc.get());
  XmlParams xmlp (node);

  BOOST_CHECK(true);

  std::vector<URI> files;
  files.push_back( "file:rectangle-tg-p2.msh" );

  xmlp.add_option<URI>("Parent Component", URI( Core::instance().root()->get_child("Domain")->full_path().string()) );
  xmlp.add_array("Files", files);

  // get the generic mesh loader from the Tools

  LoadMesh::Ptr load_mesh = Core::instance().root()->get_child("Tools")->get_child<LoadMesh>("LoadMesh");
  cf_assert( is_not_null(load_mesh) );

  load_mesh->signal_load_mesh( node );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( enricher )
{
  CBubbleEnrich::Ptr enricher =
      Core::instance().root()->get_child<CBubbleEnrich>("enricher");

  cf_assert( is_not_null(enricher) );

  Component::Ptr domain = Core::instance().root()->get_child("Domain");

  CMesh::Ptr mesh = find_component_ptr<CMesh>(*domain);


//  CFinfo << "---------------------------------------------------" << CFendl;
//  CFinfo << Core::instance().root()->tree() << CFendl;
//  CFinfo << "---------------------------------------------------" << CFendl;

  enricher->transform( mesh );

//  CFinfo << "---------------------------------------------------" << CFendl;
//  CFinfo << Core::instance().root()->tree() << CFendl;
//  CFinfo << "---------------------------------------------------" << CFendl;

  // print connectivity
//  CMesh::Ptr mesh = find_component_ptr<CMesh>(domain);
//  boost_foreach(CElements& elements, find_components_recursively<CCells>(*mesh))
//  {
//    CFinfo << "---------------------------------------------------" << CFendl;
//    CFinfo << elements.connectivity_table().full_path().string()  << CFendl;    // loop on the elements
//    for ( Uint elem = 0; elem != elements.connectivity_table().size(); ++elem )
//    {
//      for ( Uint n = 0; n != elements.connectivity_table().row_size(); ++n )
//        CFinfo << " " << elements.connectivity_table().array()[elem][n];
//      CFinfo << CFendl;
//    }
//  }
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( remover )
{
  CDomain& domain = find_component_recursively<CDomain>(*Core::instance().root());

  CBubbleRemove::Ptr remover =
      Core::instance().root()->get_child<CBubbleRemove>("remover");

  cf_assert( is_not_null(remover) );

  CMesh::Ptr mesh = find_component_ptr<CMesh>(domain);

  // print connectivity
//  CMesh::Ptr mesh = find_component_ptr<CMesh>(domain);
//  boost_foreach(CElements& elements, find_components_recursively<CCells>(*mesh))
//  {
//    CFinfo << "---------------------------------------------------" << CFendl;
//    CFinfo << elements.connectivity_table().full_path().string()  << CFendl;    // loop on the elements
//    for ( Uint elem = 0; elem != elements.connectivity_table().size(); ++elem )
//    {
//      for ( Uint n = 0; n != elements.connectivity_table().row_size(); ++n )
//        CFinfo << " " << elements.connectivity_table().array()[elem][n];
//      CFinfo << CFendl;
//    }
//  }

//  CFinfo << "---------------------------------------------------" << CFendl;
//  CFinfo << Core::instance().root()->tree() << CFendl;
//  CFinfo << "---------------------------------------------------" << CFendl;

  remover->transform( mesh );

//  CFinfo << "---------------------------------------------------" << CFendl;
//  CFinfo << Core::instance().root()->tree() << CFendl;
//  CFinfo << "---------------------------------------------------" << CFendl;


//  boost_foreach(CElements& elements, find_components_recursively<CCells>(*mesh))
//  {
//    CFinfo << "---------------------------------------------------" << CFendl;
//    CFinfo << elements.connectivity_table().full_path().string()  << CFendl;    // loop on the elements
//    for ( Uint elem = 0; elem != elements.connectivity_table().size(); ++elem )
//    {
//      for ( Uint n = 0; n != elements.connectivity_table().row_size(); ++n )
//        CFinfo << " " << elements.connectivity_table().array()[elem][n];
//      CFinfo << CFendl;
//    }
//  }

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

