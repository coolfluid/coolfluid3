// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for cf3::sdm"


#include <boost/test/unit_test.hpp>
#include <boost/assign/list_of.hpp>
#include "common/Log.hpp"
#include "common/Core.hpp"
#include "common/Environment.hpp"
#include "common/OSystem.hpp"
#include "common/OSystemLayer.hpp"
#include "common/OptionList.hpp"
#include "common/List.hpp"
#include "common/Link.hpp"

#include "common/PE/Comm.hpp"
#include "common/PE/debug.hpp"

#include "math/Consts.hpp"
#include "math/VariablesDescriptor.hpp"

#include "solver/Model.hpp"
#include "solver/Tags.hpp"

#include "physics/PhysModel.hpp"
#include "physics/Variables.hpp"

#include "mesh/Domain.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Field.hpp"
#include "mesh/FieldManager.hpp"
#include "mesh/SimpleMeshGenerator.hpp"
#include "mesh/MeshTransformer.hpp"
#include "mesh/Region.hpp"

#include "sdm/SDSolver.hpp"
#include "sdm/Term.hpp"
#include "sdm/BC.hpp"
#include "sdm/Tags.hpp"

#include "sdm/scalar/Diffusion2D.hpp"

using namespace boost::assign;
using namespace cf3;
using namespace cf3::math;
using namespace cf3::common;
using namespace cf3::common::PE;
using namespace cf3::mesh;
using namespace cf3::physics;
using namespace cf3::solver;
using namespace cf3::sdm;
using namespace cf3::sdm::scalar;

struct sdm_diffusionTests_Fixture
{
  /// common setup for each test case
  sdm_diffusionTests_Fixture()
  {
    m_argc = boost::unit_test::framework::master_test_suite().argc;
    m_argv = boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~sdm_diffusionTests_Fixture()
  {
  }
  /// possibly common functions used on the tests below


  /// common values accessed by all tests goes here
  int    m_argc;
  char** m_argv;

};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( sdm_diffusionTests_TestSuite, sdm_diffusionTests_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( init_mpi )
{
  PE::Comm::instance().init(m_argc,m_argv);
  Core::instance().environment().options().set("log_level", (Uint)INFO);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( solver2d_test )
{
  //////////////////////////////////////////////////////////////////////////////

  Model& model   = *Core::instance().root().create_component<Model>("model2d");
  model.setup("cf3.sdm.SDSolver","cf3.physics.Scalar.Scalar2D");
  PhysModel& physics = model.physics();
  SDSolver& solver  = *model.solver().handle<SDSolver>();
  Domain&   domain  = model.domain();

  //////////////////////////////////////////////////////////////////////////////
  // create and configure mesh

  // Create a 2D rectangular mesh
  Mesh& mesh = *domain.create_component<Mesh>("mesh");

  Uint DOF = 5;
  Uint order = 3;

  Uint res = 20;//DOF/order;

  Uint sol_order = order;
  Uint time_order = 1;

  std::vector<Uint> nb_cells = list_of( 5 )( 5 );
  std::vector<Real> lengths  = list_of( 10.)( 10.);

//  std::vector<Uint> nb_cells = list_of( 1 )( 1 );
//  std::vector<Real> lengths  = list_of( 2.)( 2.);

  SimpleMeshGenerator& generate_mesh = *domain.create_component<SimpleMeshGenerator>("generate_mesh");
  generate_mesh.options().set("mesh",mesh.uri());
  generate_mesh.options().set("nb_cells",nb_cells);
  generate_mesh.options().set("lengths",lengths);
  generate_mesh.execute();

  boost::shared_ptr<MeshTransformer> rotate = build_component_abstract_type<MeshTransformer>("cf3.mesh.actions.Rotate","rotate");
  rotate->options().set("angle",math::Consts::pi()/4.);
  rotate->transform(mesh);

//  build_component_abstract_type<MeshTransformer>("cf3.mesh.actions.LoadBalance","load_balance")->transform(mesh);


  solver.options().set(sdm::Tags::mesh(),mesh.handle<Mesh>());

  //////////////////////////////////////////////////////////////////////////////
  // Prepare the mesh

  solver.options().set(sdm::Tags::solution_vars(),std::string("cf3.physics.Scalar.LinearAdv2D"));
  solver.options().set(sdm::Tags::solution_order(),sol_order);
  solver.iterative_solver().options().set("nb_stages",time_order);
  solver.prepare_mesh().execute();

  //////////////////////////////////////////////////////////////////////////////
  // Configure simulation

  // Initial condition
  solver::Action& init = solver.initial_conditions().create_initial_condition("constants");
  std::vector<std::string> functions;
  functions.push_back("1.");
  init.options().set("functions",functions);
  init.execute();

  Field& solution_field = *follow_link(solver.field_manager().get_child(sdm::Tags::solution()))->handle<Field>();

  // Discretization
  Term& diffusion = solver.domain_discretization().create_term("cf3.sdm.scalar.Diffusion2D","diffusion");
  diffusion.options().set("mu",1.);
  diffusion.options().set("alpha",1.);

  std::vector<URI> bc_regions;
  bc_regions.push_back(mesh.topology().uri()/"left");
  bc_regions.push_back(mesh.topology().uri()/"right");
  bc_regions.push_back(mesh.topology().uri()/"top");
  bc_regions.push_back(mesh.topology().uri()/"bottom");
  BC& bc = solver.boundary_conditions().create_boundary_condition("cf3.sdm.BCConstant<1,2>","walls",bc_regions);
  bc.options().set("constants",std::vector<Real>(1,0.));

  // Time stepping
  solver.time().options().set("time_step",100.);
  solver.time().options().set("end_time" , 100. );
  solver.time_stepping().options().set("cfl" , std::string("0.05") );
  solver.time_stepping().options().set("max_iteration" , 20 );

  //////////////////////////////////////////////////////////////////////////////
  // Run simulation

  std::vector<URI> fields;
  fields.push_back(solution_field.uri());
  fields.push_back(solution_field.dict().field("diffusion").uri());
  fields.push_back(solution_field.dict().field("diffusion_wavespeed").uri());

  model.simulate();

  mesh.write_mesh("diffusion2d.msh",fields);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( finalize_mpi )
{
  PE::Comm::instance().finalize();
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
