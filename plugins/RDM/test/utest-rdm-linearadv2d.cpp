// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::RDM::LinearAdv2D"

#include <boost/test/unit_test.hpp>

#include "Common/BoostFilesystem.hpp"


#include "Common/FindComponents.hpp"
#include "Common/Log.hpp"
#include "Common/Core.hpp"
#include "Common/CRoot.hpp"
#include "Common/CLink.hpp"
#include "Common/Foreach.hpp"
#include "Common/LibLoader.hpp"
#include "Common/OSystem.hpp"
#include "Common/OptionArray.hpp"
#include "Common/OptionT.hpp"
#include "Common/OptionURI.hpp"

#include "Common/XML/SignalOptions.hpp"

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
#include "Mesh/Actions/CBuildFaces.hpp"

#include "RDM/Core/RKRD.hpp"
#include "RDM/Core/DomainTerm.hpp"
#include "RDM/Core/SteadyExplicit.hpp"

using namespace CF;
using namespace CF::Common;
using namespace CF::Common::XML;
using namespace CF::Mesh;
using namespace CF::Mesh::Actions;
using namespace CF::Solver;
using namespace CF::Solver::Actions;
using namespace CF::RDM;

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
    options.add_option< OptionT<std::string> >("PhysicalModel","LinearAdv2D");

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

BOOST_AUTO_TEST_SUITE( linearadv2d_test_suite )

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( read_mesh , local_fixture )
{
  SignalFrame frame; SignalOptions options;

  URI file( "file:rectangle2x1-tg-p1-953.msh");
//  URI file( "file:rectangle2x1-tg-p2-3689.msh");

//  URI file( "file:rectangle2x1-qd-p1-861.msh");
//  URI file( "file:rectangle2x1-qd-p2-3321.msh");

  options.add_option< OptionURI >("file", file );
  options.add_option< OptionT<std::string> >("name", std::string("Mesh") );

  frame = options.create_frame();

  domain.signal_load_mesh( frame );

  BOOST_CHECK_NE( domain.count_children(), (Uint) 0);

  CMesh& mesh = find_component<CMesh>(domain);

  solver.configure_option("mesh", mesh.uri() );

#if 1
  // create faces to cell connectivity

  CBuildFaces::Ptr facebuilder = allocate_component<CBuildFaces>("facebuilder");
  domain.add_component( facebuilder );
  facebuilder->set_mesh(mesh);
  facebuilder->execute();

  // print the domain tree

  CFinfo << domain.tree() << CFendl;
#endif
}

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( signal_initialize_solution , local_fixture )
{
  SignalFrame frame; SignalOptions options;

  std::vector<std::string> functions(1);
  functions[0] = "7.7777";
  options.add_option< OptionArrayT<std::string> >("functions", functions);

  frame = options.create_frame();

  solver.as_type<RKRD>().signal_initialize_solution( frame );

  BOOST_CHECK(true);

  CMesh& mesh = find_component<CMesh>(domain);

  std::vector<URI> fields;
  boost_foreach(const CField& field, find_components_recursively<CField>(mesh))
    fields.push_back(field.uri());

  writer.configure_option("fields",fields);
  writer.configure_option("file",URI(model.name()+"_init.msh"));
  writer.configure_option("mesh",mesh.uri());

  writer.execute();
}

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( initial_output , local_fixture )
{
  CMesh& mesh = find_component<CMesh>(domain);

  writer.write_mesh(mesh,URI(model.name() + "_init.plt"));
  writer.write_mesh(mesh,URI(model.name() + "_init.msh"));
}

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( signal_create_boundaries , local_fixture )
{
  // inlet bc
  {
    SignalFrame frame; SignalOptions options;

    std::vector<URI> regions;
    boost_foreach( const CRegion& region, find_components_recursively_with_name<CRegion>(domain,"bottom"))
        regions.push_back( region.uri() );
    boost_foreach( const CRegion& region, find_components_recursively_with_name<CRegion>(domain,"left"))
        regions.push_back( region.uri() );

    BOOST_CHECK_EQUAL( regions.size() , 2u);

    std::string name ("WEAK_INLET");

    options.add_option< OptionT<std::string> >("Name",name);
    options.add_option< OptionT<std::string> >("Type","CF.RDM.Core.WeakDirichlet");
    options.add_option< OptionArrayT<URI> >("Regions", regions);

    frame = options.create_frame();

    solver.as_ptr<RKRD>()->signal_create_boundary_term(frame);

    Component::Ptr inletbc = find_component_ptr_recursively_with_name( solver, name );
    cf_assert( is_not_null(inletbc) );

    std::vector<std::string> fns;
    fns.push_back("cos(2*3.141592*(x+y))");
    inletbc->configure_option("functions", fns);
  }

  BOOST_CHECK(true);
}

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( setup_iterative_solver , local_fixture )
{
  solver.get_child("time_stepping").configure_option("cfl", 0.5);
  solver.get_child("time_stepping").configure_option("MaxIter", 50u);
}

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( solve_lda , local_fixture )
{
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

  SignalFrame frame; SignalOptions options;

  std::vector<URI> regions;
  boost_foreach( const CRegion& region, find_components_recursively_with_name<CRegion>(mesh,"topology"))
    regions.push_back( region.uri() );

  BOOST_CHECK_EQUAL( regions.size() , 1u);

  options.add_option< OptionT<std::string> >("Name","INTERNAL");
  options.add_option< OptionT<std::string> >("Type","CF.RDM.Schemes.CSysLDA");
  options.add_option< OptionArrayT<URI> >("Regions", regions);

  frame = options.create_frame();

  solver.as_ptr<RKRD>()->signal_create_domain_term(frame);

  solver.solve();
}

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( output , local_fixture )
{
  CMesh& mesh = find_component<CMesh>(domain);

  writer.write_mesh(mesh,URI(model.name() + ".plt"));
  writer.write_mesh(mesh,URI(model.name() + ".msh"));
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

