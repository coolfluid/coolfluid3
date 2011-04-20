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

#include "RDM/RKRD.hpp"
#include "RDM/DomainTerm.hpp"
#include "RDM/ScalarAdvection.hpp"

using namespace CF;
using namespace CF::Common;
using namespace CF::Common::XML;
using namespace CF::Mesh;
using namespace CF::Solver;
using namespace CF::Solver::Actions;
using namespace CF::RDM;

struct translationadv2d_global_fixture
{
  translationadv2d_global_fixture()
  {
    Core::instance().initiate(boost::unit_test::framework::master_test_suite().argc,
                              boost::unit_test::framework::master_test_suite().argv);

    translationadv2d_wizard = allocate_component<ScalarAdvection>("mymodel");

    SignalFrame frame("", "", "");
    SignalFrame& options = frame.map( Protocol::Tags::key_options() );

    options.set_option<std::string>("ModelName","mymodel");
    options.set_option<std::string>("PhysicalModel","LinearAdv2D");

    translationadv2d_wizard->signal_create_model(frame);
  }

  ~translationadv2d_global_fixture() { Core::instance().terminate(); }

  ScalarAdvection::Ptr translationadv2d_wizard;

};

struct translationadv2d_local_fixture
{
  translationadv2d_local_fixture() :
    model  ( * Core::instance().root().get_child_ptr("mymodel")->as_ptr<CModel>() ),
    domain ( find_component_recursively<CDomain>(model)  ),
    solver ( find_component_recursively<CSolver>(model) )
  {}

  CModel& model;
  CDomain& domain;
  CSolver& solver;
};


//////////////////////////////////////////////////////////////////////////////

BOOST_GLOBAL_FIXTURE( translationadv2d_global_fixture )

BOOST_AUTO_TEST_SUITE( translationadv2d_test_suite )

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( check_tree , translationadv2d_local_fixture )
{
  BOOST_CHECK(true);

  SignalFrame frame("", "", "");

  Core::instance().root().signal_list_tree(frame);

//  CFinfo << model.tree() << CFendl;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( read_mesh , translationadv2d_local_fixture )
{
  BOOST_CHECK(true);

  // create the xml parameters for the read mesh signal

  SignalFrame frame("", "", "");
  SignalFrame& options = frame.map( Protocol::Tags::key_options() );

  BOOST_CHECK(true);

  std::vector<URI> files;

  URI file ( "file:translation-tg.msh" );
  //  URI file ( "file:rotation-qd-p1.neu" );

  options.set_option<URI>("File", file );

  domain.signal_load_mesh( frame );

  BOOST_CHECK_NE( domain.count_children(), (Uint) 0);

}

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( setup_iterative_solver , translationadv2d_local_fixture )
{
  BOOST_CHECK(true);

  solver.configure_property("Domain",URI("cpath:../Domain"));
  solver.get_child("time_stepping").configure_property("CFL", 1.);
  solver.get_child("time_stepping").configure_property("MaxIter", 1000u);
}

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( signal_create_boundary_term , translationadv2d_local_fixture )
{
  BOOST_CHECK(true);

  SignalFrame frame("", "", "");
  SignalFrame& options = frame.map( Protocol::Tags::key_options() );

  std::vector<URI> regions;
  boost_foreach( const CRegion& region, find_components_recursively_with_name<CRegion>(domain,"inlet"))
    regions.push_back( region.full_path() );
  boost_foreach( const CRegion& region, find_components_recursively_with_name<CRegion>(domain,"farfield"))
    regions.push_back( region.full_path() );


  //regions.push_back( regions.full_path() );

  BOOST_CHECK_EQUAL( regions.size() , 2u);

  std::string name ("INLET");

  options.set_option<std::string>("Name",name);
  options.set_option<std::string>("Type","CF.RDM.BcDirichlet");
  options.set_array("Regions", regions, " ; ");

  solver.as_ptr<RKRD>()->signal_create_boundary_term(frame);

  Component::Ptr inletbc = find_component_ptr_recursively_with_name( solver, name );
  cf_assert( is_not_null(inletbc) );

  inletbc->
        configure_property("Function", std::string("-4*(y-0.5)*(y-0.5)+1") );

  BOOST_CHECK(true);
}

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( signal_initialize_solution , translationadv2d_local_fixture )
{
  BOOST_CHECK(true);

  SignalFrame frame("", "", "");
  SignalFrame& options = frame.map( Protocol::Tags::key_options() );

  std::vector<std::string> functions(1);
  functions[0] = "0.0";
  options.set_array("Functions", functions, " ; ");

  solver.as_type<RKRD>().signal_initialize_solution( frame );
}

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( solve_ldagpu , translationadv2d_local_fixture )
{
  BOOST_CHECK(true);

  CFinfo << "solving with LDAGPU scheme" << CFendl;

  // delete previous domain terms
  Component& domain_terms = solver.get_child("compute_domain_terms");
  boost_foreach( RDM::DomainTerm& term, find_components_recursively<RDM::DomainTerm>( domain_terms ))
  {
    const std::string name = term.name();
    domain_terms.remove_component( name );
  }

  BOOST_CHECK( domain_terms.count_children() == 0 );

  CMesh::Ptr mesh = find_component_ptr<CMesh>(domain);

  SignalFrame frame("", "", "");
  SignalFrame& options = frame.map( Protocol::Tags::key_options() );

  std::vector<URI> regions;
  boost_foreach( const CRegion& region, find_components_recursively_with_name<CRegion>(*mesh,"topology"))
    regions.push_back( region.full_path() );

  BOOST_CHECK_EQUAL( regions.size() , 1u);

  options.set_option<std::string>("Name","INTERNAL");
  options.set_option<std::string>("Type","CF.RDM.LDAGPU");

  options.set_array("Regions", regions, " ; ");

  solver.as_ptr<RKRD>()->signal_create_domain_term(frame);

  //std::cout << solver.tree() << std::endl;

  BOOST_CHECK(true);

  solver.solve();

  BOOST_CHECK(true);

}

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( output , translationadv2d_local_fixture )
{
  BOOST_CHECK(true);

  CMesh::Ptr mesh = find_component_ptr<CMesh>(domain);

  BOOST_CHECK(true);

  CMeshWriter::Ptr mesh_writer = create_component_abstract_type<CMeshWriter> ( "CF.Mesh.Gmsh.CWriter", "GmshWriter" );
  model.add_component(mesh_writer);

  std::vector<URI> fields;
  boost_foreach(const CField& field, find_components_recursively<CField>(*mesh))
    fields.push_back(field.full_path());

  mesh_writer->configure_property("Fields",fields);
  mesh_writer->configure_property("File",model.name()+".msh");
  mesh_writer->configure_property("Mesh",mesh->full_path());

  mesh_writer->write();

}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

