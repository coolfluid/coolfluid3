// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::RDM::ScalarAdvection"

#include <boost/test/unit_test.hpp>

#include "Common/BoostFilesystem.hpp"

#include "Common/CreateComponent.hpp"
#include "Common/FindComponents.hpp"
#include "Common/Log.hpp"
#include "Common/CLink.hpp"
#include "Common/Foreach.hpp"

#include "Common/XML/SignalOptions.hpp"

#include "Solver/CSolver.hpp"
#include "Solver/CModel.hpp"
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

#include "RDM/Core/RKRD.hpp"
#include "RDM/Core/DomainTerm.hpp"
#include "RDM/Core/ScalarAdvection.hpp"

using namespace CF;
using namespace CF::Common;
using namespace CF::Common::XML;
using namespace CF::Mesh;
using namespace CF::Solver;
using namespace CF::Solver::Actions;
using namespace CF::RDM;

//#define BUBBLE

struct linearadv2d_global_fixture
{
  linearadv2d_global_fixture()
  {
    Core::instance().initiate(boost::unit_test::framework::master_test_suite().argc,
                              boost::unit_test::framework::master_test_suite().argv);

    linearadv2d_wizard = allocate_component<ScalarAdvection>("mymodel");

    SignalFrame frame;
    SignalOptions options( frame );

    options.add<std::string>("ModelName","mymodel");
    options.add<std::string>("PhysicalModel","LinearAdv2D");

    linearadv2d_wizard->signal_create_model(frame);
  }

  ~linearadv2d_global_fixture()
  {
    linearadv2d_wizard.reset();

    Core::instance().terminate();
  }

  ScalarAdvection::Ptr linearadv2d_wizard;

};

struct linearadv2d_local_fixture
{
  linearadv2d_local_fixture() :
    model  ( * Core::instance().root().get_child_ptr("mymodel")->as_ptr<CModel>() ),
    domain ( find_component_recursively<CDomain>(model)  ),
    solver ( find_component_recursively<CSolver>(model) )
  {}

  CModel& model;
  CDomain& domain;
  CSolver& solver;
};


//////////////////////////////////////////////////////////////////////////////

BOOST_GLOBAL_FIXTURE( linearadv2d_global_fixture )

BOOST_AUTO_TEST_SUITE( linearadv2d_test_suite )

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( test_check_tree , linearadv2d_local_fixture )
{
  BOOST_CHECK(true);

  SignalFrame frame;
  SignalOptions options( frame );

  Core::instance().root().signal_list_tree(frame);

//  CFinfo << model.tree() << CFendl;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( test_read_mesh , linearadv2d_local_fixture )
{
  BOOST_CHECK(true);

  // create the xml parameters for the read mesh signal

  SignalFrame frame;
  SignalOptions options( frame );

  BOOST_CHECK(true);

  std::vector<URI> files;

  URI file( "file:rectangle2x1-tg-p1-953.msh");
//  URI file( "file:rectangle2x1-tg-p2-3689.msh");

//  URI file( "file:rectangle2x1-qd-p1-861.msh");
//  URI file( "file:rectangle2x1-qd-p2-3321.msh");

  options.add("file", file );
  options.add<std::string>("name", std::string("Mesh") );

  domain.signal_load_mesh( frame );

  BOOST_CHECK_NE( domain.count_children(), (Uint) 0);

#ifdef BUBBLE // enrich the mesh with bubble functions
  CMeshTransformer::Ptr enricher =
      create_component_abstract_type<CMeshTransformer>("CF.Mesh.Actions.CBubbleEnrich","enricher");

  domain.add_component( enricher );

  CMesh::Ptr mesh = find_component_ptr<CMesh>(domain);

  enricher->transform( mesh );
#endif
}

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( test_setup_iterative_solver , linearadv2d_local_fixture )
{
  BOOST_CHECK(true);

  solver.configure_property("Domain",URI("cpath:../Domain"));
  solver.get_child("time_stepping").configure_property("CFL", 0.5);;
  solver.get_child("time_stepping").configure_property("MaxIter", 250u);;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( test_create_boundary_term , linearadv2d_local_fixture )
{
  BOOST_CHECK(true);

//  SignalFrame frame;
//  SignalOptions options( frame );

//  std::vector<URI> regions;
//  boost_foreach( const CRegion& region, find_components_recursively_with_name<CRegion>(domain,"bottom"))
//    regions.push_back( region.full_path() );
//  boost_foreach( const CRegion& region, find_components_recursively_with_name<CRegion>(domain,"left"))
//    regions.push_back( region.full_path() );

//  BOOST_CHECK_EQUAL( regions.size() , 2u);

//  std::string name ("INLET");

//  options.add<std::string>("Name",name);
//  options.add<std::string>("Type","CF.RDM.Core.BcDirichlet");
//  options.add("Regions", regions, " ; ");

//  solver.as_ptr<RKRD>()->signal_create_boundary_term(frame);

//  Component::Ptr inletbc = find_component_ptr_recursively_with_name( solver, name );
//  cf_assert( is_not_null(inletbc) );

//  std::vector<std::string> fns;
//  fns.push_back("cos(2*3.141592*(x+y))");

//  inletbc->configure_property("Functions", fns);

  BOOST_CHECK(true);
}

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( test_create_boundary_weak_term , linearadv2d_local_fixture )
{
  BOOST_CHECK(true);

  SignalFrame frame;
  SignalOptions options( frame );

  std::vector<URI> regions;
  boost_foreach( const CRegion& region, find_components_recursively_with_name<CRegion>(domain,"bottom"))
    regions.push_back( region.full_path() );
  boost_foreach( const CRegion& region, find_components_recursively_with_name<CRegion>(domain,"left"))
    regions.push_back( region.full_path() );

  BOOST_CHECK_EQUAL( regions.size() , 2u);

  std::string name ("WEAK_INLET");

  options.add<std::string>("Name",name);
  options.add<std::string>("Type","CF.RDM.Core.WeakDirichlet");
  options.add("Regions", regions, " ; ");

  solver.as_ptr<RKRD>()->signal_create_boundary_term(frame);

  Component::Ptr inletbc = find_component_ptr_recursively_with_name( solver, name );
  cf_assert( is_not_null(inletbc) );

  std::vector<std::string> fns;
  fns.push_back("cos(2*3.141592*(x+y))");

  inletbc->configure_property("Functions", fns);

  BOOST_CHECK(true);
}

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( signal_initialize_solution , linearadv2d_local_fixture )
{
  BOOST_CHECK(true);

  SignalFrame frame;
  SignalOptions options( frame );

  std::vector<std::string> functions(1);
  functions[0] = "0.";
  options.add<std::string>("Functions", functions, " ; ");

  solver.as_type<RKRD>().signal_initialize_solution( frame );
}

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( solve_lda , linearadv2d_local_fixture )
{
  BOOST_CHECK(true);

  CFinfo << "solving with LDA scheme" << CFendl;

  // delete previous domain terms
  Component& domain_terms = solver.get_child("compute_domain_terms");
  boost_foreach( RDM::DomainTerm& term, find_components_recursively<RDM::DomainTerm>( domain_terms ))
  {
    const std::string name = term.name();
    domain_terms.remove_component( name );
  }

  BOOST_CHECK( domain_terms.count_children() == 0 );

  CMesh::Ptr mesh = find_component_ptr<CMesh>(domain);

  SignalFrame frame;
  SignalOptions options( frame );

  std::vector<URI> regions;
  boost_foreach( const CRegion& region, find_components_recursively_with_name<CRegion>(*mesh,"topology"))
    regions.push_back( region.full_path() );

  BOOST_CHECK_EQUAL( regions.size() , 1u);

  options.add<std::string>("Name","INTERNAL");
  options.add<std::string>("Type","CF.RDM.Schemes.CSysLDA");
  options.add("Regions", regions, " ; ");

  solver.as_ptr<RKRD>()->signal_create_domain_term(frame);

  BOOST_CHECK(true);

  solver.solve();

  BOOST_CHECK(true);

}

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( solve_lf , linearadv2d_local_fixture )
{
  BOOST_CHECK(true);

//  CFinfo << "solving with LF scheme" << CFendl;

//  // delete previous domain terms
//  Component& domain_terms = solver.get_child("compute_domain_terms");
//  boost_foreach( RDM::DomainTerm& term, find_components_recursively<RDM::DomainTerm>( domain_terms ))
//  {
//    const std::string name = term.name();
//    domain_terms.remove_component( name );
//  }

//  BOOST_CHECK( domain_terms.count_children() == 0 );

//  CMesh::Ptr mesh = find_component_ptr<CMesh>(domain);

//  SignalFrame frame;
//  SignalOptions options( frame );

//  std::vector<URI> regions;
//  boost_foreach( const CRegion& region, find_components_recursively_with_name<CRegion>(*mesh,"topology"))
//    regions.push_back( region.full_path() );

//  BOOST_CHECK_EQUAL( regions.size() , 1u);

//  options.add<std::string>("Name","INTERNAL");
//  options.add<std::string>("Type","CF.RDM.Schemes.CSysLF");
//  options.add("Regions", regions, " ; ");

//  solver.as_ptr<RKRD>()->signal_create_domain_term(frame);

//  BOOST_CHECK(true);

//  solver.solve();

  BOOST_CHECK(true);

}

//////////////////////////////////////////////////////////////////////////////

//BOOST_FIXTURE_TEST_CASE( solve_blended , linearadv2d_local_fixture )
//{
//  BOOST_CHECK(true);

//  CFinfo << "solving with Blended scheme" << CFendl;

//  // delete previous domain terms
//  Component& domain_terms = solver.get_child("compute_domain_terms");
//  boost_foreach( RDM::DomainTerm& term, find_components_recursively<RDM::DomainTerm>( domain_terms ))
//  {
//    const std::string name = term.name();
//    domain_terms.remove_component( name );
//  }

//  BOOST_CHECK( domain_terms.count_children() == 0 );

//  CMesh::Ptr mesh = find_component_ptr<CMesh>(domain);

//  SignalFrame frame;
//  SignalOptions options( frame );

//  std::vector<URI> regions;
//  boost_foreach( const CRegion& region, find_components_recursively_with_name<CRegion>(*mesh,"topology"))
//    regions.push_back( region.full_path() );

//  BOOST_CHECK_EQUAL( regions.size() , 1u);

//  options.add<std::string>("Name","INTERNAL");
//  options.add<std::string>("Type","CF.RDM.Blended");
//  options.add("Regions", regions, " ; ");

//  solver.as_ptr<RKRD>()->signal_create_domain_term(frame);

//  BOOST_CHECK(true);

//  solver.solve();

//  BOOST_CHECK(true);

//}

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( test_output , linearadv2d_local_fixture )
{
  BOOST_CHECK(true);

  CMesh::Ptr mesh = find_component_ptr<CMesh>(domain);

#ifdef BUBBLE // remove the bubble functions from the mesh
  CMeshTransformer::Ptr remover =
      create_component_abstract_type<CMeshTransformer>("CF.Mesh.Actions.CBubbleRemove","remover");

  domain.add_component( remover );
  remover->transform( mesh );
#endif

  BOOST_CHECK(true);

  CMeshWriter::Ptr mesh_writer = create_component_abstract_type<CMeshWriter> ( "CF.Mesh.Gmsh.CWriter", "GmshWriter" );
  model.add_component(mesh_writer);

  std::vector<URI> fields;
  boost_foreach(const CField& field, find_components_recursively<CField>(*mesh))
    fields.push_back(field.full_path());

  mesh_writer->configure_property("fields",fields);
  mesh_writer->configure_property("file",URI(model.name()+".msh"));
  mesh_writer->configure_property("mesh",mesh->full_path());

  mesh_writer->execute();
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

