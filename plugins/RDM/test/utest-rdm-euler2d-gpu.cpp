uri()// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::RDM::Euler2D"

#include <boost/test/unit_test.hpp>
#include <boost/assign/list_of.hpp>

#include "Common/BoostFilesystem.hpp"

#include "Common/Core.hpp"
#include "Common/CRoot.hpp"
#include "Common/FindComponents.hpp"
#include "Common/Log.hpp"
#include "Common/CLink.hpp"
#include "Common/Foreach.hpp"
#include "Common/LibLoader.hpp"
#include "Common/OSystem.hpp"

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
#include "RDM/Core/SteadyExplicit.hpp"

using namespace CF;
using namespace CF::Common;
using namespace CF::Common::XML;
using namespace CF::Mesh;
using namespace CF::Solver;
using namespace CF::Solver::Actions;
using namespace CF::RDM;

struct euler2d_global_fixture
{
  euler2d_global_fixture()
  {
    Core::instance().initiate(boost::unit_test::framework::master_test_suite().argc,
                              boost::unit_test::framework::master_test_suite().argv);

    // Load the required libraries (we assume the working dir is the binary path)
    LibLoader& loader = *OSystem::instance().lib_loader();

    const std::vector< boost::filesystem::path > lib_paths = boost::assign::list_of
                                                             ("../../../dso");
    loader.set_search_paths(lib_paths);

    loader.load_library("coolfluid_mesh_neu");
    loader.load_library("coolfluid_mesh_gmsh");
    //loader.load_library("coolfluid_mesh_tecplot");
    //loader.load_library("coolfluid_mesh_vtklegacy");

    euler2d_wizard = allocate_component<SteadyExplicit>("mymodel");

    SignalFrame frame;
    SignalOptions options( frame );

    options.add<std::string>("ModelName","mymodel");
    options.add<std::string>("PhysicalModel","Euler2D");

    euler2d_wizard->signal_create_model(frame);
  }

//  ~euler2d_global_fixture() { Core::instance().terminate(); }


  SteadyExplicit::Ptr euler2d_wizard;

};

struct euler2d_local_fixture
{
  euler2d_local_fixture() :
    model  ( * Core::instance().root().get_child_ptr("mymodel")->as_ptr<CModel>() ),
    domain ( find_component_recursively<CDomain>(model)  ),
    solver ( find_component_recursively<CSolver>(model) )
  {}

  CModel& model;
  CDomain& domain;
  CSolver& solver;
};


//////////////////////////////////////////////////////////////////////////////

BOOST_GLOBAL_FIXTURE( euler2d_global_fixture )

BOOST_AUTO_TEST_SUITE( euler2d_test_suite )

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( test_check_tree , euler2d_local_fixture )
{
  BOOST_CHECK(true);

  SignalFrame frame;
  SignalOptions options( frame );

  Core::instance().root().signal_list_tree(frame);
}

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( test_read_mesh , euler2d_local_fixture )
{
  BOOST_CHECK(true);

//  CFinfo << Core::instance().root().tree() << CFendl;

  // create the xml parameters for the read mesh signal

  SignalFrame frame;
  SignalOptions options( frame );

  BOOST_CHECK(true);

  std::vector<URI> files;

  URI file( "file:euler2d-tg.msh" );     // works


  std::vector<URI::Scheme::Type> schemes(1);
  schemes[0] = URI::Scheme::FILE;

  options.add("file", file );
  options.add<std::string>("name", std::string("Mesh") );

  std::cout << "opening file: " << file.string() << std::endl;

  domain.signal_load_mesh( frame );

  BOOST_CHECK_NE( domain.count_children(), (Uint) 0);
}

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( test_setup_iterative_solver , euler2d_local_fixture )
{
  BOOST_CHECK(true);

  solver.configure_property("Domain",URI("cpath:../Domain"));
  solver.get_child("time_stepping").configure_property("cfl", 0.25);;
  solver.get_child("time_stepping").configure_property("MaxIter", 250u);;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( test_create_boundary_term , euler2d_local_fixture )
{
  BOOST_CHECK(true);

  SignalFrame frame;
  SignalOptions options( frame );

  std::vector<URI> regions;
  boost_foreach( const CRegion& region, find_components_recursively_with_name<CRegion>(domain,"bottom"))
    regions.push_back( region.uri() );
  boost_foreach( const CRegion& region, find_components_recursively_with_name<CRegion>(domain,"left"))
    regions.push_back( region.uri() );
  boost_foreach( const CRegion& region, find_components_recursively_with_name<CRegion>(domain,"right"))
    regions.push_back( region.uri() );

  BOOST_CHECK_EQUAL( regions.size() , 3u);

  std::string name ("INLET");

  options.add<std::string>("Name",name);
  options.add<std::string>("Type","CF.RDM.Core.WeakDirichlet");
  options.add("Regions", regions, " ; ");

  solver.as_ptr<RKRD>()->signal_create_boundary_term(frame);

  Component::Ptr inletbc = find_component_ptr_recursively_with_name( solver, name );
  cf_assert( is_not_null(inletbc) );

  std::vector<std::string> fns(4);

  fns[0] = "if(x>0.5,0.5,1.)";
  fns[1] = "0.0";
  fns[2] = "if(x>0.5,1.67332,2.83972)";
  fns[3] = "if(x>0.5,3.425,6.532)";

//  fns[0] = "0.5";
//  fns[1] = "0.0";
//  fns[2] = "1.67332";
//  fns[3] = "3.425";

  inletbc->configure_property("Functions", fns);

  BOOST_CHECK(true);
}

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( signal_initialize_solution , euler2d_local_fixture )
{
  BOOST_CHECK(true);

  SignalFrame frame;
  SignalOptions options( frame );

  std::vector<std::string> fns(4);


  fns[0] = "if(x>0.5,0.5,1.)";
  fns[1] = "0.0";
  fns[2] = "if(x>0.5,1.67332,2.83972)";
  fns[3] = "if(x>0.5,3.425,6.532)";

//  fns[0] = "0.5";
//  fns[1] = "0.0";
//  fns[2] = "1.67332";
//  fns[3] = "3.425";

  options.add<std::string>("Functions", fns, " ; ");

  solver.as_type<RKRD>().signal_initialize_solution( frame );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( test_init_output , euler2d_local_fixture )
{
  BOOST_CHECK(true);

  CMesh::Ptr mesh = find_component_ptr<CMesh>(domain);

  CMeshWriter::Ptr gmsh_writer = build_component_abstract_type<CMeshWriter> ( "CF.Mesh.Gmsh.CWriter", "GmshWriter" );
  model.add_component(gmsh_writer);

  std::vector<URI> fields;
  boost_foreach(const CField& field, find_components_recursively<CField>(*mesh))
    fields.push_back(field.uri());

  gmsh_writer->configure_property("fields",fields);
  gmsh_writer->configure_property("file",URI(model.name()+"_init.msh"));
  gmsh_writer->configure_property("mesh",mesh->uri());

  gmsh_writer->execute();

  model.remove_component("GmshWriter");
}

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( solve_CSysLDAGPU, euler2d_local_fixture )
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
    regions.push_back( region.uri() );

  BOOST_CHECK_EQUAL( regions.size() , 1u);

  options.add<std::string>("Name","INTERNAL");
  options.add<std::string>("Type","CF.RDM.Schemes.CSysLDAGPU");
  options.add("Regions", regions, " ; ");

  solver.as_ptr<RKRD>()->signal_create_domain_term(frame);

  BOOST_CHECK(true);

  solver.solve();

  BOOST_CHECK(true);

}

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( test_output , euler2d_local_fixture )
{
  BOOST_CHECK(true);

  CMesh::Ptr mesh = find_component_ptr<CMesh>(domain);

  std::vector<URI> fields;
  boost_foreach(const CField& field, find_components_recursively<CField>(*mesh))
    fields.push_back(field.uri());

  // gmsh writer

  CMeshWriter::Ptr gmsh_writer = build_component_abstract_type<CMeshWriter> ( "CF.Mesh.Gmsh.CWriter", "GmshWriter" );
  model.add_component(gmsh_writer);

  gmsh_writer->configure_property("fields",fields);
  gmsh_writer->configure_property("file",URI(model.name()+".msh"));
  gmsh_writer->configure_property("mesh",mesh->uri());

  gmsh_writer->execute();
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

