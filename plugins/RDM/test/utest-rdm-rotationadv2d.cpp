// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::RDM::ScalarAdvection"

#include <boost/test/unit_test.hpp>
#include <boost/filesystem/path.hpp>

#include "Common/CreateComponent.hpp"
#include "Common/ComponentPredicates.hpp"
#include "Common/Log.hpp"
#include "Common/CLink.hpp"
#include "Common/Foreach.hpp"

#include "Solver/CIterativeSolver.hpp"
#include "Solver/CModel.hpp"
#include "Solver/CDiscretization.hpp"
#include "Solver/CPhysicalModel.hpp"
#include "Solver/Actions/CLoop.hpp"

#include "Mesh/LoadMesh.hpp"
#include "Mesh/CCells.hpp"
#include "Mesh/CMeshReader.hpp"
#include "Mesh/CMeshWriter.hpp"
#include "Mesh/CDomain.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/Actions/CBubbleEnrich.hpp"
#include "Mesh/Actions/CBubbleRemove.hpp"

#include "RDM/ScalarAdvection.hpp"
#include "RDM/ResidualDistribution.hpp"

using namespace CF;
using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Solver;
using namespace CF::Solver::Actions;
using namespace CF::RDM;

//#define BUBBLE

struct rotationadv2d_global_fixture
{
  rotationadv2d_global_fixture()
  {
    rotationadv2d_wizard = allocate_component<ScalarAdvection>("mymodel");

    boost::shared_ptr<XmlDoc> doc = XmlOps::create_doc();
    XmlNode& node  = *XmlOps::goto_doc_node(*doc.get());
    XmlParams p(node);
    p.add_option<std::string>("Model name","mymodel");

    rotationadv2d_wizard->create_model(node);
  }

  ScalarAdvection::Ptr rotationadv2d_wizard;

};

struct rotationadv2d_local_fixture
{
  rotationadv2d_local_fixture() :
    model  ( * Core::instance().root()->get_child("mymodel")->as_type<CModel>() ),
    domain ( find_component_recursively<CDomain>(model)  ),
    solver ( find_component_recursively<CIterativeSolver>(model) ),
    discretization( find_component_recursively<CDiscretization>(solver) )

  {}

  CModel& model;
  CDomain& domain;
  CIterativeSolver& solver;
  CDiscretization& discretization;
};


//////////////////////////////////////////////////////////////////////////////

BOOST_GLOBAL_FIXTURE( rotationadv2d_global_fixture )

BOOST_AUTO_TEST_SUITE( rotationadv2d_test_suite )

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( check_tree , rotationadv2d_local_fixture )
{
  BOOST_CHECK(true);

  boost::shared_ptr<XmlDoc> doc = XmlOps::create_doc();
  XmlNode& tree_node  = *XmlOps::goto_doc_node(*doc.get());

  Core::instance().root()->list_tree(tree_node);

//  CFinfo << model.tree() << CFendl;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( read_mesh , rotationadv2d_local_fixture )
{
  BOOST_CHECK(true);

  // create the xml parameters for the read mesh signal

  boost::shared_ptr<XmlDoc> doc = XmlOps::create_doc();
  XmlNode& node  = *XmlOps::goto_doc_node(*doc.get());
  XmlParams xmlp (node);

  BOOST_CHECK(true);

  std::vector<URI> files;

  files.push_back( "file:rotation-tg-p1.neu" );
//  files.push_back( "file:rotation-qd-p1.neu" );

  xmlp.add_option<URI>("Parent Component", URI( domain.full_path().string()) );
  xmlp.add_array("Files", files);

  // get the generic mesh loader from the Tools

  LoadMesh::Ptr load_mesh = Core::instance().root()->get_child("Tools")->get_child<LoadMesh>("LoadMesh");
  cf_assert( is_not_null(load_mesh) );

  load_mesh->signal_load_mesh( node );

  BOOST_CHECK_NE( domain.get_child_count(), (Uint) 0);

#ifdef BUBBLE // enrich the mesh with bubble functions
  CMeshTransformer::Ptr enricher =
      create_component_abstract_type<CMeshTransformer>("CF.Mesh.Actions.CBubbleEnrich","enricher");

  domain.add_component( enricher );

  CMesh::Ptr mesh = find_component_ptr<CMesh>(domain);

  enricher->transform( mesh );
#endif
}

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( setup_iterative_solver , rotationadv2d_local_fixture )
{
  BOOST_CHECK(true);

  solver.configure_property("Domain",URI("cpath:../Domain"));
  solver.configure_property("Number of Iterations", 1500u);
}

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( create_boundary_term , rotationadv2d_local_fixture )
{
  BOOST_CHECK(true);

  boost::shared_ptr<XmlDoc> doc = XmlOps::create_doc();
  XmlNode& node  = *XmlOps::goto_doc_node(*doc.get());
  XmlParams p(node);

  std::vector<URI> bc_regions;
  boost_foreach( const CRegion& region, find_components_recursively_with_name<CRegion>(domain,"inlet"))
    bc_regions.push_back( region.full_path() );

  BOOST_CHECK_EQUAL( bc_regions.size() , 1u);

  std::string name ("INLET");

  p.add_option<std::string>("Name",name);
  p.add_option<std::string>("Type","CF.RDM.BcDirichlet");
  p.add_array("Regions", bc_regions);

  discretization.as_type<ResidualDistribution>()->create_boundary_term(node);

  Component::Ptr inletbc = find_component_ptr_recursively_with_name( discretization, name );
  cf_assert( is_not_null(inletbc) );

  inletbc->get_child("action")->
      configure_property("Function", std::string("if(x>=-1.4,if(x<=-0.6,0.5*(cos(3.141592*(x+1.0)/0.4)+1.0),0.),0.)") );

//  CFinfo << find_component_recursively<CModel>(*Core::instance().root()).tree() << CFendl;

  BOOST_CHECK(true);
}

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( create_domain_term , rotationadv2d_local_fixture )
{
  BOOST_CHECK(true);

  CMesh::Ptr mesh = find_component_ptr<CMesh>(domain);

  boost::shared_ptr<XmlDoc> doc = XmlOps::create_doc();
  XmlNode& node  = *XmlOps::goto_doc_node(*doc.get());
  XmlParams p(node);

  std::vector<URI> bc_regions;
  boost_foreach( const CRegion& region, find_components_recursively_with_name<CRegion>(*mesh,"topology"))
    bc_regions.push_back( region.full_path() );

  BOOST_CHECK_EQUAL( bc_regions.size() , 1u);

  p.add_option<std::string>("Name","INTERNAL");
  p.add_option<std::string>("Type","CF.RDM.CLDA<RotationAdv2D>");
  p.add_array("Regions", bc_regions);

  discretization.as_type<ResidualDistribution>()->create_domain_term(node);

//  CFinfo << find_component_recursively<CModel>(*Core::instance().root()).tree() << CFendl;

  BOOST_CHECK(true);
}

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( solve , rotationadv2d_local_fixture )
{
  BOOST_CHECK(true);

//  CFinfo << model.tree() << CFendl;

  solver.solve();
}

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( output , rotationadv2d_local_fixture )
{
  BOOST_CHECK(true);

  CMesh::Ptr mesh = find_component_ptr<CMesh>(domain);

#ifdef BUBBLE // remove the bubble functions from the mesh
  CMeshTransformer::Ptr remover =
      create_component_abstract_type<CMeshTransformer>("CF.Mesh.Actions.CBubbleRemove","remover");

  domain.add_component( remover );

  std::vector<std::string> args;
  remover->transform( mesh );
#endif

  BOOST_CHECK(true);

//  CFinfo << model.tree() << CFendl;

  ////////////////////////////////////////////////////////////////////////////////
  // Writer
  ////////////////////////////////////////////////////////////////////////////////

  CMeshWriter::Ptr mesh_writer = create_component_abstract_type<CMeshWriter> ( "CF.Mesh.Gmsh.CWriter", "GmshWriter" );
  model.add_component(mesh_writer);

  std::vector<URI> fields;
  boost_foreach(const CField2& field, find_components_recursively<CField2>(*mesh))
    fields.push_back(field.full_path());

  mesh_writer->configure_property("Fields",fields);
  mesh_writer->configure_property("File",model.name()+".msh");
  mesh_writer->configure_property("Mesh",mesh->full_path());

  mesh_writer->write();

}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

