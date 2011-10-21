// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for cf3::SFDM"

#include <boost/test/unit_test.hpp>
#include <boost/assign/list_of.hpp>
#include "common/Log.hpp"
#include "common/Core.hpp"
#include "common/Root.hpp"
#include "common/Environment.hpp"
#include "common/OSystem.hpp"
#include "common/OSystemLayer.hpp"

#include "math/VariablesDescriptor.hpp"

#include "Solver/CModel.hpp"
#include "Solver/Tags.hpp"

#include "Physics/PhysModel.hpp"
#include "Physics/Variables.hpp"

#include "mesh/CDomain.hpp"
#include "mesh/Geometry.hpp"
#include "mesh/Field.hpp"
#include "mesh/FieldManager.hpp"
#include "mesh/CSimpleMeshGenerator.hpp"
#include "mesh/MeshTransformer.hpp"

#include "mesh/CLinearInterpolator.hpp"

#include "SFDM/SFDSolver.hpp"
#include "SFDM/Term.hpp"
#include "SFDM/Tags.hpp"

#include "mesh/CRegion.hpp"
//#include "mesh/Mesh.hpp"
//#include "mesh/CField.hpp"
//#include "mesh/CEntities.hpp"
//#include "mesh/ElementType.hpp"
//#include "mesh/MeshWriter.hpp"
//#include "mesh/CDomain.hpp"
//#include "mesh/Actions/CInitFieldFunction.hpp"
//#include "mesh/Actions/CreateSpaceP0.hpp"
//#include "Solver/CModelUnsteady.hpp"
//#include "Solver/CSolver.hpp"
//#include "Solver/CPhysicalModel.hpp"
//#include "mesh/Actions/CBuildFaces.hpp"
//#include "mesh/Actions/CBuildVolume.hpp"
//#include "mesh/Actions/CreateSpaceP0.hpp"
//#include "SFDM/CreateSpace.hpp"

using namespace boost::assign;
using namespace cf3;
using namespace cf3::math;
using namespace cf3::common;
using namespace cf3::mesh;
using namespace cf3::Physics;
using namespace cf3::Solver;
using namespace cf3::SFDM;

//////////////////////////////////////////////////////////////////////////////

struct SFDM_MPITests_Fixture
{
  /// common setup for each test case
  SFDM_MPITests_Fixture()
  {
    m_argc = boost::unit_test::framework::master_test_suite().argc;
    m_argv = boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~SFDM_MPITests_Fixture()
  {
  }
  /// possibly common functions used on the tests below


  /// common values accessed by all tests goes here
  int    m_argc;
  char** m_argv;

};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( SFDM_Solver_TestSuite, SFDM_MPITests_Fixture )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( init_mpi )
{
  PE::Comm::instance().init(m_argc,m_argv);
}

BOOST_AUTO_TEST_CASE( Solver_test )
{
  Core::instance().environment().configure_option("log_level", (Uint)INFO);

  //////////////////////////////////////////////////////////////////////////////
  // create and configure SFD - LinEuler 2D model
  Uint dim=2;

  CModel& model   = Core::instance().root().create_component<CModel>("model");
  model.setup("CF.SFDM.SFDSolver","CF.Physics.LinEuler.LinEuler2D");
  PhysModel& physics = model.physics();
  SFDSolver& solver  = model.solver().as_type<SFDSolver>();
  CDomain&   domain  = model.domain();

  physics.configure_option("gamma",1.4);
  physics.configure_option("rho0",1.);
  physics.configure_option("U0",std::vector<Real>(2,0.));
  physics.configure_option("P0",1.);

  //////////////////////////////////////////////////////////////////////////////
  // create and configure mesh

  // Create a 2D rectangular mesh
  Mesh& mesh = domain.create_component<Mesh>("mesh");

  Uint res = 20;
  Uint order = 3;
  std::vector<Uint> nb_cells = list_of( res  )( res );
  std::vector<Real> lengths  = list_of(  1.  )(  1. );
  std::vector<Real> offsets  = list_of( -0.5 )( -0.5 );

  CSimpleMeshGenerator& generate_mesh = domain.create_component<CSimpleMeshGenerator>("generate_mesh");
  generate_mesh.configure_option("mesh",mesh.uri());
  generate_mesh.configure_option("nb_cells",nb_cells);
  generate_mesh.configure_option("lengths",lengths);
  generate_mesh.configure_option("offsets",offsets);
  generate_mesh.execute();
  build_component_abstract_type<MeshTransformer>("CF.Mesh.Actions.LoadBalance","load_balance")->transform(mesh);
  solver.configure_option(SFDM::Tags::mesh(),mesh.uri());

  //////////////////////////////////////////////////////////////////////////////
  // Prepare the mesh

  solver.configure_option(SFDM::Tags::solution_vars(),std::string("CF.Physics.LinEuler.Cons2D"));
  solver.configure_option(SFDM::Tags::solution_order(),order);
  solver.iterative_solver().configure_option("rk_order",3u);
  solver.prepare_mesh().execute();

  //////////////////////////////////////////////////////////////////////////////
  // Configure simulation

  // Initial condition
  Solver::Action& shocktube = solver.initial_conditions().create_initial_condition("accoustics");
  std::vector<std::string> functions;

  // Accoustic pulse LinEuler
  std::string rho = "0.001*exp( -( (x-0.0)^2 + (y-0.0)^2 )/(0.05)^2 )";
  std::string c2 = "1.4*1/1";  // c = sqrt(gamma*P0/rho0)
  functions.push_back(rho);
  functions.push_back("0.");
  functions.push_back("0.");
  functions.push_back(c2 + "*" + rho);

  // Accoustic pulse Euler
//  shocktube.configure_option(SFDM::Tags::input_vars(), physics.create_variables("CF.Physics.LinEuler.Prim2D",SFDM::Tags::input_vars())->uri() );
//  std::string rho_ac = "0.001*exp( -( (x-0.5)^2 + (y-0.5)^2 )/(0.05)^2 )";
//  std::string c2 = "1.4*1/1";  // c = sqrt(gamma*P0*rho0)
//  functions.push_back("1*(1+"+rho_ac+")");
//  functions.push_back("0.");
//  functions.push_back("0.");
//  functions.push_back("1 + "+c2 + "*" + rho_ac);


// Tiago's case
//  functions.push_back( "exp((-0.301)/25.*(x*x+y*y))+0.1*exp((-0.301)/25.*((x-67.)*(x-67.)+y*y))" );
//  functions.push_back( "0.04*(y)*exp((-0.301)/25.*((x-67.)*(x-67.)+y*y))" );
//  functions.push_back( "-0.04*(x-67.)*exp((-0.301)/25.*((x-67.)*(x-67.)+y*y))" );
//  functions.push_back( "exp((-0.301)/25.*(x*x+y*y))" );


// Shocktube
//  shocktube.configure_option(SFDM::Tags::input_vars(), physics.create_variables("CF.Physics.NavierStokes.Prim2D",SFDM::Tags::input_vars())->uri() );
//  functions.push_back("if( x<="+to_str(lengths[XX]/2.)+"& y<="+to_str(lengths[YY]/2.)+" , 4.696  , 1.408  )"); // Prim2D[ Rho ]
//  functions.push_back("if( x<="+to_str(lengths[XX]/2.)+"& y<="+to_str(lengths[YY]/2.)+" , 0      , 0      )"); // Prim2D[ U   ]
//  if (dim>1)
//    functions.push_back("if( x<="+to_str(lengths[XX]/2.)+"& y<="+to_str(lengths[YY]/2.)+" , 0      , 0      )"); // Prim2D[ V   ]
//  functions.push_back("if( x<="+to_str(lengths[XX]/2.)+"& y<="+to_str(lengths[YY]/2.)+" , 404400 , 101100 )"); // Prim2D[ P   ]


  shocktube.configure_option("functions",functions);
  solver.initial_conditions().execute();
  domain.write_mesh("sfdm_initial.msh");


  // Discretization
//  physics.create_variables("CF.Physics.LinEuler.Roe2D","roe_vars");
  solver.domain_discretization().create_term("CF.SFDM.Convection","convection",std::vector<URI>(1,mesh.topology().uri()));
//  solver.domain_discretization().create_term("CF.SFDM.DummyTerm","term_2",std::vector<URI>(1,mesh.topology().uri()));
//  solver.domain_discretization().create_term("CF.SFDM.DummyTerm","term_3",std::vector<URI>(1,mesh.topology().uri()));

  // Time stepping
  solver.time_stepping().time().configure_option("time_step",100.);
  solver.time_stepping().time().configure_option("end_time" ,0.01); // instead of 0.3
  solver.time_stepping().configure_option_recursively("cfl" , 0.3);
  solver.time_stepping().configure_option_recursively("milestone_dt" , 0.3);


  //////////////////////////////////////////////////////////////////////////////
  // Run simulation

  CFinfo << model.tree() << CFendl;

  model.simulate();

  CFinfo << "memory: " << OSystem::instance().layer()->memory_usage_str() << CFendl;

  //////////////////////////////////////////////////////////////////////////////
  // Output


  std::vector<URI> fields;
  Field& solution_field = solver.field_manager().get_child(SFDM::Tags::solution()).follow()->as_type<Field>();
  Field& solution_geom = mesh.geometry().create_field("solution_geom",solution_field.descriptor());

  Action& interpolate = mesh.create_component("interpolate","CF.Mesh.Actions.Interpolate").as_type<Action>();
  interpolate.configure_option("source",solution_field.uri());
  interpolate.configure_option("target",solution_geom.uri());
  interpolate.execute();

  fields.push_back(solution_field.uri());
  fields.push_back(solution_geom.uri());
  mesh.write_mesh("sfdm_output.msh",fields);
  mesh.write_mesh("sfdm_output.vtu",fields);
  mesh.write_mesh("sfdm_output.plt",fields);

  RealVector max( solution_field.row_size() ); max.setZero();
  RealVector min( solution_field.row_size() ); min.setZero();
  for (Uint i=0; i<solution_field.size(); ++i)
  {
    for (Uint j=0; j<solution_field.row_size(); ++j)
    {
      max[j] = std::max(max[j],solution_field[i][j]);
      min[j] = std::min(min[j],solution_field[i][j]);
    }
  }

  std::cout << "solution_field.max = " << max.transpose() << std::endl;
  std::cout << "solution_field.min = " << min.transpose() << std::endl;


//  // Create a 1D line mesh
//  Mesh& probe = domain.create_component<Mesh>("probe");

//  std::vector<Uint> nb_cells_probe = list_of( res*order*2 );
//  std::vector<Real> lengths_probe  = list_of( 0.5 );
//  std::vector<Real> offsets_probe  = list_of( 0. );

//  CSimpleMeshGenerator& generate_probe = domain.create_component<CSimpleMeshGenerator>("generate_probe");
//  generate_probe.configure_option("mesh",probe.uri());
//  generate_probe.configure_option("nb_cells",nb_cells_probe);
//  generate_probe.configure_option("lengths",lengths_probe);
//  generate_probe.configure_option("offsets",offsets_probe);
//  generate_probe.execute();

//  Field& solution_probe = probe.geometry().create_field("solution",solution_field.descriptor());

//  std::cout << "solution_field.row_size() = " << solution_field.row_size() << std::endl;
//  std::cout << "solution_probe.row_size() = " << solution_probe.row_size() << std::endl;
//  std::cout << "solution_probe.descriptor.size() = " << solution_probe.descriptor().size() << std::endl;
//  std::cout << "solution_field.descriptor.size() = " << solution_field.descriptor().size() << std::endl;




//  CInterpolator::Ptr interpolator = build_component_abstract_type<CInterpolator>("CF.Mesh.CLinearInterpolator","interpolator");
////  interpolator->configure_option("ApproximateNbElementsPerCell", (Uint) 1 );
////  // Following configuration option has priority over the the previous one.
////  std::vector<Uint> divisions = boost::assign::list_of(3)(2)(2);
////  //interpolator->configure_option("Divisions", divisions );

//  // Create the honeycomb
//  interpolator->construct_internal_storage(mesh);

//  // Interpolate the source field data to the target field. Note it can be in same or different meshes
//  interpolator->interpolate_field_from_to(solution_field,solution_probe);







//  std::vector<URI> probe_fields (1,solution_probe.uri());
//  probe.write_mesh("probe.plt",probe_fields);


//  solver.configure_option_recursively("time",model.time().uri());
//  solver.configure_option_recursively("time_accurate",true);
//  solver.configure_option_recursively("cfl",1.);

//  model.time().configure_option("end_time",2.5);
//  model.time().configure_option("time_step",5.);

//  /// Initialize solution field with the function sin(2*pi*x)
//  Actions::CInitFieldFunction::Ptr init_field = common::Core::instance().root().create_component_ptr<Actions::CInitFieldFunction>("init_field");
//  //init_field->configure_option("functions",std::vector<std::string>(1,"sin(2*pi*x/10)"));

//  std::string gaussian="sigma:=1; mu:=5.; exp(-(x-mu)^2/(2*sigma^2)) / exp(-(mu-mu)^2/(2*sigma^2))";
//  init_field->configure_option("functions",std::vector<std::string>(1,gaussian));
//  init_field->configure_option("field",find_component_with_tag<CField>(mesh,"solution").uri());
//  init_field->transform(mesh);


//  std::vector<CField::Ptr> fields;
//  fields.push_back(find_component_with_tag<CField>(mesh,"solution").as_ptr<CField>());
//  fields.push_back(find_component_with_tag<CField>(mesh,"jacobian_determinant").as_ptr<CField>());
//  fields.push_back(find_component_with_tag<CField>(mesh,"residual").as_ptr<CField>());
//  fields.push_back(find_component_with_tag<CField>(mesh,"wave_speed").as_ptr<CField>());

//  MeshWriter& gmsh_writer = solver.get_child("iterate").create_component("7_gmsh_writer","CF.Mesh.Gmsh.CWriter").as_type<MeshWriter>();
//  gmsh_writer.configure_option("mesh",mesh.uri());
//  gmsh_writer.configure_option("file",URI("line_${iter}.msh"));
//  gmsh_writer.set_fields(fields);

//  gmsh_writer.execute();

//  CFinfo << model.tree() << CFendl;

//  //solver.get_child("iterate").configure_option("MaxIterations",1u);
//  solver.solve();

//  gmsh_writer.configure_option("file",URI("final.msh"));
//  gmsh_writer.execute();

//  /// write gmsh file. note that gmsh gets really confused because of the multistate view
////  gmsh_writer->write_from_to(mesh,"line_"+to_str(model.time().time())+".msh");
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( finalize_mpi )
{
  PE::Comm::instance().finalize();
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
