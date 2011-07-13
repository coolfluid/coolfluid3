// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::RDM::Burgers2D"

#include <boost/test/unit_test.hpp>

#include "Common/BoostFilesystem.hpp"
#include "Common/XML/SignalOptions.hpp"


#include "Common/FindComponents.hpp"
#include "Common/Log.hpp"
#include "Common/Core.hpp"
#include "Common/CRoot.hpp"
#include "Common/CLink.hpp"
#include "Common/Foreach.hpp"
#include "Common/LibLoader.hpp"
#include "Common/OSystem.hpp"
#include "Common/OptionT.hpp"
#include "Common/OptionURI.hpp"
#include "Common/OptionArray.hpp"

#include "Solver/CSolver.hpp"
#include "Solver/CModel.hpp"
#include "Solver/Actions/CLoop.hpp"

#include "Mesh/WriteMesh.hpp"
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
#include "RDM/Core/SteadyExplicit.hpp"

using namespace CF;
using namespace CF::Common;
using namespace CF::Common::XML;
using namespace CF::Mesh;
using namespace CF::Solver;
using namespace CF::Solver::Actions;
using namespace CF::RDM;

//#define BUBBLE

struct global_fixture
{
  global_fixture()
  {
    Core::instance().initiate(boost::unit_test::framework::master_test_suite().argc,
                              boost::unit_test::framework::master_test_suite().argv);


    mpi::PE::instance().init(boost::unit_test::framework::master_test_suite().argc,
                             boost::unit_test::framework::master_test_suite().argv);

    LibLoader& loader = *OSystem::instance().lib_loader();

    loader.load_library("coolfluid_mesh_neu");
    loader.load_library("coolfluid_mesh_gmsh");
    loader.load_library("coolfluid_mesh_tecplot");

    wizard = allocate_component<SteadyExplicit>("wizard");

    SignalFrame frame;
    SignalOptions options;

    options.add_option< OptionT<std::string> >("ModelName","mymodel");
    options.add_option< OptionT<std::string> >("PhysicalModel","Burgers2D");

    frame = options.create_frame();

    wizard->signal_create_model(frame);

   CModel& model = Core::instance().root().get_child("mymodel").as_type<CModel>();

   CDomain& domain = find_component_recursively<CDomain>(model);
   CSolver& solver = find_component_recursively<CSolver>(model);

   solver.configure_option("domain", domain.uri() );

   model.create_component_ptr<WriteMesh>("writer");
  }

  ~global_fixture()
  {
    wizard.reset();
    mpi::PE::instance().finalize();
    Core::instance().terminate();
  }

  SteadyExplicit::Ptr wizard;

}; // !global_fixture

struct local_fixture
{
    local_fixture() :
    model  ( * Core::instance().root().get_child_ptr("mymodel")->as_ptr<CModel>() ),
    domain ( find_component_recursively<CDomain>(model)   ),
    solver ( find_component_recursively<CSolver>(model)   ),
    writer ( find_component_recursively<WriteMesh>(model) )
  {}

  CModel&     model;
  CDomain&    domain;
  CSolver&    solver;
  WriteMesh&  writer;

};

//////////////////////////////////////////////////////////////////////////////

BOOST_GLOBAL_FIXTURE( global_fixture )

BOOST_AUTO_TEST_SUITE( burgers2d_test_suite )

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( check_tree , local_fixture )
{
  BOOST_CHECK(true);

  SignalFrame frame;

  Core::instance().root().signal_list_tree(frame);

//  CFinfo << model.tree() << CFendl;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( read_mesh , local_fixture )
{
  BOOST_CHECK(true);

  // create the xml parameters for the read mesh signal

  SignalFrame frame;
  SignalOptions options;

  BOOST_CHECK(true);

//  URI file ( "file:square1x1-tg-p1-303n.msh" );  // works
//  URI file ( "file:square1x1-tg-p1-7614.msh" );  // works
  URI file ( "file:square1x1-tg-p2-333n.msh" );    // works
//  URI file ( "file:square1x1-tg-p2-2kn.msh"  );  // works

//  URI file ( "file:square1x1-qd-p1-256n.msh" );  // works
//  URI file ( "file:square1x1-qd-p1-1369.msh" );  // works
//  URI file ( "file:square1x1-qd-p2-289n.msh" );  // works

//  URI file ( "file:square1x1-tgqd-p1-298n.msh" ); // works

  options.add_option( OptionURI::create("file", file, URI::Scheme::FILE) );
  options.add_option< OptionT<std::string> >("name", std::string("Mesh") );

  frame = options.create_frame();

  domain.signal_load_mesh( frame );

  BOOST_CHECK_NE( domain.count_children(), (Uint) 0);

#ifdef BUBBLE // enrich the mesh with bubble functions
  CMeshTransformer::Ptr enricher =
      build_component_abstract_type<CMeshTransformer>("CF.Mesh.Actions.CBubbleEnrich","enricher");

  domain.add_component( enricher );

  CMesh& mesh = find_component<CMesh>(domain);

  enricher->transform( mesh );
#endif

}

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( setup_iterative_solver , local_fixture )
{
  BOOST_CHECK(true);

  solver.configure_option("domain",URI("cpath:../Domain"));
  solver.get_child("time_stepping").configure_option("cfl", 0.5);
  solver.get_child("time_stepping").configure_option("MaxIter", 50u);
}

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( signal_create_boundary_term , local_fixture )
{
  BOOST_CHECK(true);

  SignalFrame frame;
  SignalOptions options;

  std::vector<URI> regions;
  boost_foreach( const CRegion& region, find_components_recursively_with_name<CRegion>(domain,"bottom"))
    regions.push_back( region.uri() );
  boost_foreach( const CRegion& region, find_components_recursively_with_name<CRegion>(domain,"left"))
    regions.push_back( region.uri() );
  boost_foreach( const CRegion& region, find_components_recursively_with_name<CRegion>(domain,"right"))
    regions.push_back( region.uri() );

  BOOST_CHECK_EQUAL( regions.size() , 3u);

  std::string name = "INLET";
  std::vector<URI::Scheme::Type> schemes(1);
  schemes[0] = URI::Scheme::CPATH;

  options.add_option< OptionT<std::string> >("Name", name);
  options.add_option< OptionT<std::string> >("Type", "CF.RDM.Core.BcDirichlet");
  options.add_option< OptionArrayT<URI> >("Regions", regions/*,  schemes*/);

  frame = options.create_frame();

  solver.as_ptr<RKRD>()->signal_create_boundary_term(frame);

  Component::Ptr inletbc = find_component_ptr_recursively_with_name( solver, name );
  cf_assert( is_not_null(inletbc) );

  std::vector<std::string> fns;
  fns.push_back("1.5-2.0*x");
  inletbc->configure_option("functions", fns);

  BOOST_CHECK(true);
}

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( signal_initialize_solution , local_fixture )
{
  BOOST_CHECK(true);

  SignalFrame frame;
  SignalOptions options;

  std::vector<std::string> functions(1);
  functions[0] = "0.";
  options.add_option< OptionArrayT<std::string> >("functions", functions);

  frame = options.create_frame();

  solver.as_type<RKRD>().signal_initialize_solution( frame );
}

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( initial_output , local_fixture )
{
  CMesh& mesh = find_component<CMesh>(domain);

  writer.write_mesh(mesh,URI(model.name() + "_init.plt"));
  writer.write_mesh(mesh,URI(model.name() + "_init.msh"));
}

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( solve_lda , local_fixture )
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

  CMesh& mesh = find_component<CMesh>(domain);

  SignalFrame frame;
  SignalOptions options;

  std::vector<URI> regions;
  boost_foreach( const CRegion& region, find_components_recursively_with_name<CRegion>(mesh,"topology"))
    regions.push_back( region.uri() );

  BOOST_CHECK_EQUAL( regions.size() , 1u);

  std::vector<URI::Scheme::Type> schemes(1);
  schemes[0] = URI::Scheme::CPATH;

  options.add_option< OptionT<std::string> >("Name","INTERNAL");
  options.add_option< OptionT<std::string> >("Type","CF.RDM.Schemes.CSysLDA");
  options.add_option<OptionArrayT<URI> >("Regions", regions/*, schemes*/);

  frame = options.create_frame();

  solver.as_ptr<RKRD>()->signal_create_domain_term(frame);

  BOOST_CHECK(true);

  solver.solve();

  BOOST_CHECK(true);

}

////////////////////////////////////////////////////////////////////////////////

//BOOST_FIXTURE_TEST_CASE( solve_blended , local_fixture )
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

//  CMesh& mesh = find_component<CMesh>(domain);

//  SignalFrame frame;
//  SignalOptions options( frame );

//  std::vector<URI> regions;
//  boost_foreach( const CRegion& region, find_components_recursively_with_name<CRegion>(mesh,"topology"))
//    regions.push_back( region.uri() );

//  BOOST_CHECK_EQUAL( regions.size() , 1u);

//  options.add_option< OptionT<std::string> >("Name","INTERNAL");
//  options.add_option< OptionT<std::string> >("Type","CF.RDM.Blended");
//  options.add("Regions", regions, " ; ");

//  solver.as_ptr<RKRD>()->signal_create_domain_term(frame);

//  BOOST_CHECK(true);

//  solver.solve();

//  BOOST_CHECK(true);

//}

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( output , local_fixture )
{
  CMesh& mesh = find_component<CMesh>(domain);

#ifdef BUBBLE // remove the bubble functions from the mesh
  CMeshTransformer::Ptr remover =
      build_component_abstract_type<CMeshTransformer>("CF.Mesh.Actions.CBubbleRemove","remover");

  domain.add_component( remover );
  remover->transform( mesh );
#endif

  writer.write_mesh(mesh,URI(model.name() + ".plt"));
  writer.write_mesh(mesh,URI(model.name() + ".msh"));

}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

