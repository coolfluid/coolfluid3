// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::RDM::Euler2D"

#include <boost/test/unit_test.hpp>
#include <boost/assign/list_of.hpp>

#include "Common/BoostFilesystem.hpp"


#include "Common/FindComponents.hpp"
#include "Common/Log.hpp"
#include "Common/Core.hpp"
#include "Common/CRoot.hpp"
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

struct sinusbump_global_fixture
{
  sinusbump_global_fixture()
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
    loader.load_library("coolfluid_mesh_tecplot");
    loader.load_library("coolfluid_mesh_vtklegacy");

    sinusbump_wizard = allocate_component<SteadyExplicit>("mymodel");

    SignalFrame frame;
    SignalOptions options( frame );

    options.add<std::string>("ModelName","mymodel");
    options.add<std::string>("PhysicalModel","Euler2D");

    sinusbump_wizard->signal_create_model(frame);
  }

//  ~sinusbump_global_fixture() { Core::instance().terminate(); }


  SteadyExplicit::Ptr sinusbump_wizard;

};

struct sinusbump_local_fixture
{
  sinusbump_local_fixture() :
    model  ( * Core::instance().root().get_child_ptr("mymodel")->as_ptr<CModel>() ),
    domain ( find_component_recursively<CDomain>(model)  ),
    solver ( find_component_recursively<CSolver>(model) )
  {}

  CModel& model;
  CDomain& domain;
  CSolver& solver;
};


//////////////////////////////////////////////////////////////////////////////

BOOST_GLOBAL_FIXTURE( sinusbump_global_fixture )

BOOST_AUTO_TEST_SUITE( sinusbump_test_suite )

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( test_check_tree , sinusbump_local_fixture )
{
  BOOST_CHECK(true);

  SignalFrame frame;
  SignalOptions options( frame );

  Core::instance().root().signal_list_tree(frame);
}

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( test_read_mesh , sinusbump_local_fixture )
{
  BOOST_CHECK(true);

//  CFinfo << Core::instance().root().tree() << CFendl;

  // create the xml parameters for the read mesh signal

  SignalFrame frame;
  SignalOptions options( frame );

  BOOST_CHECK(true);

  std::vector<URI> files;

  URI file( "file:sinusbump-tg-p1-90.msh" );

  std::vector<URI::Scheme::Type> schemes(1);
  schemes[0] = URI::Scheme::FILE;

  options.add("file", file );
  options.add<std::string>("name", std::string("Mesh") );

  std::cout << "opening file: " << file.string() << std::endl;

  domain.signal_load_mesh( frame );

  BOOST_CHECK_NE( domain.count_children(), (Uint) 0);
}

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( test_setup_iterative_solver , sinusbump_local_fixture )
{
  BOOST_CHECK(true);

  solver.configure_property("Domain",URI("cpath:../Domain"));
  solver.get_child("time_stepping").configure_property("cfl", 0.1);
  solver.get_child("time_stepping").configure_property("MaxIter", 50u);
}

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( signal_create_boundary_term_inlet , sinusbump_local_fixture )
{
  BOOST_CHECK(true);

  SignalFrame frame;
  SignalOptions options( frame );

  std::vector<URI> regions;
  boost_foreach( const CRegion& region, find_components_recursively_with_name<CRegion>(domain,"SubInlet"))
    regions.push_back( region.full_path() );

  BOOST_CHECK_EQUAL( regions.size() , 1u);

  std::string name ("INLET");

  options.add<std::string>("Name",name);
  options.add<std::string>("Type","CF.RDM.Core.SubsonicInFlowWeakBc");
  options.add("Regions", regions, " ; ");

  solver.as_ptr<RKRD>()->signal_create_boundary_term(frame);

  Component::Ptr inletbc = find_component_ptr_recursively_with_name( solver, name );
  BOOST_CHECK( is_not_null(inletbc) );

  std::string bc_density = "1.0";
//  std::string bc_density = "1.20399";
  inletbc->configure_property("rho_in", bc_density);

  std::vector<std::string> bc_velocity(2);
  bc_velocity[XX] = "100.0";
//  bc_velocity[XX] = "172.0197011603";
  bc_velocity[YY] = "0.0";
  inletbc->configure_property("vel_in", bc_velocity);

  BOOST_CHECK(true);
}

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( signal_create_boundary_term_outlet , sinusbump_local_fixture )
{
  BOOST_CHECK(true);

  SignalFrame frame;
  SignalOptions options( frame );

  std::vector<URI> regions;
  boost_foreach( const CRegion& region, find_components_recursively_with_name<CRegion>(domain,"SubOutlet"))
    regions.push_back( region.full_path() );

  BOOST_CHECK_EQUAL( regions.size() , 1u);

  std::string name ("OUTLET");

  options.add<std::string>("Name",name);
  options.add<std::string>("Type","CF.RDM.Core.SubsonicOutFlowWeakBc");
  options.add("Regions", regions, " ; ");

  solver.as_ptr<RKRD>()->signal_create_boundary_term(frame);

  Component::Ptr outletbc = find_component_ptr_recursively_with_name( solver, name );
  BOOST_CHECK( is_not_null(outletbc) );

  std::string bc_pressure = "1000.0";
//  std::string bc_pressure = "101325.216916";
  outletbc->configure_property("p_out", bc_pressure);

  BOOST_CHECK(true);
}

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( signal_create_boundary_term_wall , sinusbump_local_fixture )
{
  BOOST_CHECK(true);

  SignalFrame frame;
  SignalOptions options( frame );

  std::vector<URI> regions;
  boost_foreach( const CRegion& region, find_components_recursively_with_name<CRegion>(domain,"LowerWall"))
    regions.push_back( region.full_path() );
  boost_foreach( const CRegion& region, find_components_recursively_with_name<CRegion>(domain,"UpperWall"))
    regions.push_back( region.full_path() );

  BOOST_CHECK_EQUAL( regions.size() , 2u);

  std::string name ("WALL");

  options.add<std::string>("Name",name);
  options.add<std::string>("Type","CF.RDM.Core.WallWeakBc");
  options.add("Regions", regions, " ; ");

  solver.as_ptr<RKRD>()->signal_create_boundary_term(frame);

  Component::Ptr wallbc = find_component_ptr_recursively_with_name( solver, name );
  BOOST_CHECK( is_not_null(wallbc) );

  BOOST_CHECK(true);
}

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( signal_initialize_solution , sinusbump_local_fixture )
{
  BOOST_CHECK(true);

  SignalFrame frame;
  SignalOptions options( frame );

  std::vector<std::string> fns(4);

  fns[0] = "1.5";
  fns[1] = "100.0";
  fns[2] = "0.0";
  fns[3] = "7500.0";

//  fns[0] = "1.20399";
//  fns[1] = "172.0197011603";
//  fns[2] = "0.0";
//  fns[3] = "265601.6732534";

  options.add<std::string>("Functions", fns, " ; ");

  solver.as_type<RKRD>().signal_initialize_solution( frame );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( test_init_output , sinusbump_local_fixture )
{
  BOOST_CHECK(true);

  CMesh::Ptr mesh = find_component_ptr<CMesh>(domain);

  CMeshWriter::Ptr gmsh_writer = build_component_abstract_type<CMeshWriter> ( "CF.Mesh.Gmsh.CWriter", "GmshWriter" );
  model.add_component(gmsh_writer);

  std::vector<URI> fields;
  boost_foreach(const CField& field, find_components_recursively<CField>(*mesh))
    fields.push_back(field.full_path());

  gmsh_writer->configure_property("fields",fields);
  gmsh_writer->configure_property("file",URI(model.name()+"_init.msh"));
  gmsh_writer->configure_property("mesh",mesh->full_path());

  gmsh_writer->execute();

  model.remove_component("GmshWriter");
}

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( solve_lda, sinusbump_local_fixture )
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

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( test_output , sinusbump_local_fixture )
{
  BOOST_CHECK(true);

  CMesh::Ptr mesh = find_component_ptr<CMesh>(domain);

  std::vector<URI> fields;
  boost_foreach(const CField& field, find_components_recursively<CField>(*mesh))
    fields.push_back(field.full_path());

  // gmsh writer

  CMeshWriter::Ptr gmsh_writer = build_component_abstract_type<CMeshWriter> ( "CF.Mesh.Gmsh.CWriter", "GmshWriter" );
  model.add_component(gmsh_writer);

  gmsh_writer->configure_property("fields",fields);
  gmsh_writer->configure_property("file",URI(model.name()+".msh"));
  gmsh_writer->configure_property("mesh",mesh->full_path());

  gmsh_writer->execute();

  // tecplot writer

  CMeshWriter::Ptr tec_writer = build_component_abstract_type<CMeshWriter>("CF.Mesh.Tecplot.CWriter","TecWriter");
  model.add_component(tec_writer);

  tec_writer->configure_property("fields",fields);
  tec_writer->configure_property("file",URI(model.name()+".plt"));
  tec_writer->configure_property("mesh",mesh->full_path());

  tec_writer->execute();

}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

