// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for cf3::sdm"

#define run_solver 1

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

#include "sdm/scalar/Diffusion1D.hpp"

#include "Tools/Gnuplot/Gnuplot.hpp"

//#include "mesh/Mesh.hpp"
//#include "mesh/CField.hpp"
//#include "mesh/Entities.hpp"
//#include "mesh/ElementType.hpp"
//#include "mesh/MeshWriter.hpp"
//#include "mesh/Domain.hpp"
//#include "mesh/actions/InitFieldFunction.hpp"
//#include "mesh/actions/CreateSpaceP0.hpp"
//#include "solver/Model.hpp"
//#include "solver/Solver.hpp"
//#include "solver/CPhysicalModel.hpp"
//#include "mesh/actions/BuildFaces.hpp"
//#include "mesh/actions/BuildVolume.hpp"
//#include "mesh/actions/CreateSpaceP0.hpp"
//#include "sdm/CreateSpace.hpp"

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

std::map<Real,Real> xy(const Field& field)
{
  std::map<Real,Real> map;
  for (Uint i=0; i<field.size(); ++i)
    map[field.coordinates()[i][0]] = field[i][0];
  return map;
}

struct sdm_MPITests_Fixture
{
  /// common setup for each test case
  sdm_MPITests_Fixture()
  {
    m_argc = boost::unit_test::framework::master_test_suite().argc;
    m_argv = boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~sdm_MPITests_Fixture()
  {
  }
  /// possibly common functions used on the tests below


  /// common values accessed by all tests goes here
  int    m_argc;
  char** m_argv;

};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( sdm_MPITests_TestSuite, sdm_MPITests_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( init_mpi )
{
  PE::Comm::instance().init(m_argc,m_argv);
  Core::instance().environment().options().set("log_level", (Uint)INFO);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test_diffusion1d )
{

  boost::shared_ptr<Diffusion1D> diffusion1d = allocate_component<Diffusion1D>("diffusion1d");
  Diffusion1D::PhysData phys_data;
  phys_data.solution_gradient[0] = 1.;
  Diffusion1D::RealVectorNDIM unit_normal; unit_normal << 1.;
  Diffusion1D::RealVectorNEQS flux;
  Real wave_speed;
  diffusion1d->compute_flux(phys_data,unit_normal,flux,wave_speed);
  CFinfo << "normal = " << unit_normal.transpose() << CFendl;
  CFinfo << "flux = " << flux.transpose() << CFendl;
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test_diffusion1d_solve)
{

  Uint dim=1;

  Model& model   = *Core::instance().root().create_component<Model>("diffusion1d");
  model.setup("cf3.sdm.SDSolver","cf3.physics.Scalar.Scalar1D");
  PhysModel& physics = model.physics();
  SDSolver& solver   = *model.solver().handle<SDSolver>();
  Domain&   domain   = model.domain();


  Uint DOF = 10;
  Uint order = 5;

  Uint res = 30;//DOF/order;

  Uint sol_order = order;
  Uint time_order = 4;

  Real cfl_matteo = 1./(2.2*(sol_order-1.)+1.);
  Real cfl = cfl_matteo;

  // CFL number for definition  CFL = dt*mu/dx^2
  boost::multi_array<Real,2> cfl_array;
  cfl_array.resize(boost::extents[6][5]);

  // RK1 (Forward Euler)
  cfl_array[1][1] = 1.;
  cfl_array[2][1] = 0.32;
  cfl_array[3][1] = 0.1;
  cfl_array[4][1] = 0.042;
  cfl_array[5][1] = 0.018;

  // RK2
  cfl_array[1][2] = 1.;
  cfl_array[2][2] = 0.32;
  cfl_array[3][2] = 0.1;
  cfl_array[4][2] = 0.042;
  cfl_array[5][2] = 0.018;

  // RK4
  cfl_array[1][4] = 1.44;
  cfl_array[2][4] = 0.46;
  cfl_array[3][4] = 0.156;
  cfl_array[4][4] = 0.056;
  cfl_array[5][4] = 0.02550;


  cfl = cfl_array[sol_order][time_order];

  physics.options().set("v",1.);

  //////////////////////////////////////////////////////////////////////////////
  // create and configure mesh

  // Create a 2D rectangular mesh
  Mesh& mesh = *domain.create_component<Mesh>("mesh");


  std::vector<Uint> nb_cells = list_of( res  );
  std::vector<Real> lengths  = list_of( 1. );
  std::vector<Real> offsets  = list_of( 0.  );

  SimpleMeshGenerator& generate_mesh = *domain.create_component<SimpleMeshGenerator>("generate_mesh");
  generate_mesh.options().set("mesh",mesh.uri());
  generate_mesh.options().set("nb_cells",nb_cells);
  generate_mesh.options().set("lengths",lengths);
  generate_mesh.options().set("offsets",offsets);
  generate_mesh.options().set("bdry",true);

  generate_mesh.execute();

  mesh.write_mesh("generated_line.msh");

  build_component_abstract_type<MeshTransformer>("cf3.mesh.actions.LoadBalance","load_balance")->transform(mesh);
  solver.options().set(sdm::Tags::mesh(),mesh.handle<Mesh>());


  mesh.write_mesh("load_balanced_line.msh");
  //////////////////////////////////////////////////////////////////////////////
  // Prepare the mesh

  solver.options().set(sdm::Tags::solution_vars(),std::string("cf3.physics.Scalar.LinearAdv1D"));
  solver.options().set(sdm::Tags::solution_order(),sol_order);
  solver.iterative_solver().options().set("nb_stages",time_order);
  solver.prepare_mesh().execute();


  //////////////////////////////////////////////////////////////////////////////
  // Configure simulation


  // Initial condition
  solver::Action& init_gauss = solver.initial_conditions().create_initial_condition("gaussian");
  std::vector<std::string> functions;
  // Gaussian wave
  functions.push_back("sigma:="+to_str(lengths[XX]/12.5)+";mu:="+to_str(lengths[XX]/2.)+";exp(-(x-mu)^2/(2*sigma^2))");
//  functions.push_back("sin(pi/"+to_str(lengths[XX])+"*x)");
//  functions.push_back("1+x/"+to_str(lengths[XX]));
//  functions.push_back("1+2*x^2");
//  functions.push_back("0");
  init_gauss.options().set("functions",functions);
  solver.initial_conditions().execute();

  Field& solution_field = *Handle<Field>( follow_link( solver.field_manager().get_child(sdm::Tags::solution()) ) );

  // Discretization
  Term& diffusion = solver.domain_discretization().create_term("cf3.sdm.scalar.Diffusion1D","diffusion",std::vector<URI>(1,mesh.topology().uri()));


  // ---------> alpha
  diffusion.options().set("alpha",1./order);

//  convection.options().set("advection_speed",std::vector<Real>(1,2.));
//  // Boundary condition
  std::vector<URI> bc_regions(1);
  bc_regions[0]=mesh.topology().uri()/"xneg";
  BC& left_bc = solver.boundary_conditions().create_boundary_condition("cf3.sdm.BCConstant<1,1>","left",bc_regions);
  left_bc.options().set("constants",std::vector<Real>(1,0.));
  bc_regions[0]=mesh.topology().uri()/"xpos";
  BC& right_bc = solver.boundary_conditions().create_boundary_condition("cf3.sdm.BCConstant<1,1>","right",bc_regions);
  right_bc.options().set("constants",std::vector<Real>(1,0.));
//  Term& dirichlet = solver.domain_discretization().create_term("cf3.sdm.BCDirichlet","dirichlet",bc_regions);
//  std::vector<std::string> dirichlet_functions;
//  dirichlet_functions.push_back("0.1");
//  dirichlet.set("functions",dirichlet_functions);


  // Time stepping
  solver.time().options().set("time_step",100.);
  solver.time().options().set("end_time",1.0); // instead of 0.3
  solver.time_stepping().options().set("max_iteration" , 200u );
  solver.time_stepping().options().set("cfl" , common::to_str(cfl) );

  //////////////////////////////////////////////////////////////////////////////
  // Run simulation

  Field& residual_field = *follow_link(solver.field_manager().get_child(sdm::Tags::residual()))->handle<Field>();
  Field& wave_speed_field = *follow_link(solver.field_manager().get_child(sdm::Tags::wave_speed()))->handle<Field>();
  Field& diffusion_field = *solution_field.parent()->get_child("diffusion")->handle<Field>();
  Field& diffusion_wavespeed = *solution_field.parent()->get_child("diffusion_wavespeed")->handle<Field>();
  Field& jacobian_determinant_field = *solution_field.parent()->get_child("jacobian_determinant")->handle<Field>();

  std::vector<std::pair<Real,Real> >exact_diffusion(100);
  for (Uint i=0; i<exact_diffusion.size(); ++i)
  {
    exact_diffusion[i].first=i*static_cast<Real>(lengths[XX])/(exact_diffusion.size()-1);
    exact_diffusion[i].second = - std::sin(Consts::pi()*exact_diffusion[i].first/lengths[XX])*Consts::pi()/lengths[XX]*Consts::pi()/lengths[XX];
  }
#ifdef GNUPLOT_FOUND
  Gnuplot gp(std::string(GNUPLOT_COMMAND));
  gp << "set terminal png\n";
  gp << "set output 'diffusion1d_P"<<Comm::instance().rank()<<".png'\n";
//  gp << "set yrange [-1.2:1.2]\n";
  gp << "set grid\n";
  gp << "set xlabel 'x'\n";
  gp << "set ylabel 'U'\n";
  gp << "set title 'Rank "<<PE::Comm::instance().rank()<<" , P"<<sol_order-1<<"  RK"<<time_order<<"  DOF="<<solution_field.size()<<"   CFL="<<cfl<<"'\n";
  gp << "plot ";
  gp << "'-' with points title 'initial solution'"    << ", ";
  gp << "'-' with linespoints title 'final solution'"      << "\n ";
//  gp << "'-' with linespoints title 'diffusion'"           << ",";
//  gp << "'-' with linespoints title '1st_derivative'"           << ",";
//  gp << "'-' with lines title 'exact_diffusion'"           << ",";
//  gp << "'-' with points title 'jacobian_determinant'";
//  gp << "'-' with linespoints title 'residual'"            << "\n";
//  gp << "'-' with points title 'wave_speed'"          << "\n";
//  gp << "\n";
  gp.send( solution_field.coordinates().array() , solution_field.array() );
#endif

  model.simulate();

  CFinfo << "solution = \n" << solution_field << CFendl;
  CFinfo << "diffusion = \n" << diffusion_field << CFendl;

#ifdef GNUPLOT_FOUND

  gp.send( solution_field.coordinates().array() , solution_field.array() );
//  gp.send( diffusion_field.coordinates().array() , diffusion_field.array() );
//  gp.send( diffusion_wavespeed.coordinates().array() , diffusion_wavespeed.array() );
//  gp.send( exact_diffusion );
//  gp.send( jacobian_determinant_field.coordinates().array() , jacobian_determinant_field.array() );
//  gp.send( residual_field.coordinates().array() , residual_field.array() );
//  gp.send( wave_speed_field.coordinates().array() , wave_speed_field.array() );
#endif

  CFinfo << "memory: " << OSystem::instance().layer()->memory_usage_str() << CFendl;

  //////////////////////////////////////////////////////////////////////////////
  // Output

  std::vector<URI> fields;
  Field& rank = solution_field.dict().create_field("rankfield");
  Field& rank_sync = solution_field.dict().create_field("rank_sync");
  for (Uint r=0; r<rank.size(); ++r)
  {
    rank[r][0] = rank.rank()[r];
    rank_sync[r][0] = PE::Comm::instance().rank();
  }
  rank_sync.parallelize();
  rank_sync.synchronize();

  fields.push_back(solution_field.uri());
  fields.push_back(solution_field.dict().field("residual").uri());
  fields.push_back(solution_field.dict().field("solution_backup").uri());
  fields.push_back(solution_field.dict().field("wave_speed").uri());
  fields.push_back(solution_field.dict().field("update_coefficient").uri());
  fields.push_back(solution_field.dict().field("diffusion").uri());
  fields.push_back(solution_field.dict().field("diffusion_wavespeed").uri());
  mesh.write_mesh("diffusion1d.plt",fields);

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
}

////////////////////////////////////////////////////////////////////////////////
#if 1
BOOST_AUTO_TEST_CASE( test_linearadvection1d_solve)
{

  //////////////////////////////////////////////////////////////////////////////
  Uint dim=1;

  Model& model   = *Core::instance().root().create_component<Model>("model");
  model.setup("cf3.sdm.SDSolver","cf3.physics.Scalar.Scalar1D");
  PhysModel& physics = model.physics();
  SDSolver& solver   = *model.solver().handle<SDSolver>();
  Domain&   domain   = model.domain();


  Uint DOF = 10;
  Uint order = 4;

  Uint res = 20;//DOF/order;

  Uint sol_order = order;
  Uint time_order = 4;

  physics.options().set("v",1.);

  //////////////////////////////////////////////////////////////////////////////
  // create and configure mesh

  // Create a 2D rectangular mesh
  Mesh& mesh = *domain.create_component<Mesh>("mesh");


  std::vector<Uint> nb_cells = list_of( res  );
  std::vector<Real> lengths  = list_of( 10.  );
  std::vector<Real> offsets  = list_of( 0.  );

  SimpleMeshGenerator& generate_mesh = *domain.create_component<SimpleMeshGenerator>("generate_mesh");
  generate_mesh.options().set("mesh",mesh.uri());
  generate_mesh.options().set("nb_cells",nb_cells);
  generate_mesh.options().set("lengths",lengths);
  generate_mesh.options().set("offsets",offsets);
  generate_mesh.options().set("bdry",true);

  generate_mesh.execute();

  mesh.write_mesh("generated_line.msh");

  build_component_abstract_type<MeshTransformer>("cf3.mesh.actions.LoadBalance","load_balance")->transform(mesh);
  solver.options().set(sdm::Tags::mesh(),mesh.handle<Mesh>());


  mesh.write_mesh("load_balanced_line.msh");
  //////////////////////////////////////////////////////////////////////////////
  // Prepare the mesh

  solver.options().set(sdm::Tags::solution_vars(),std::string("cf3.physics.Scalar.LinearAdv1D"));
  solver.options().set(sdm::Tags::solution_order(),sol_order);
  solver.iterative_solver().options().set("nb_stages",time_order);
  solver.prepare_mesh().execute();


  //////////////////////////////////////////////////////////////////////////////
  // Configure simulation


  // Initial condition
  solver::Action& init_gauss = solver.initial_conditions().create_initial_condition("gaussian");
  std::vector<std::string> functions;
  // Gaussian wave
  functions.push_back("sigma:="+to_str(lengths[XX]/20.)+";mu:="+to_str(lengths[XX]/2.)+";exp(-(x-mu)^2/(2*sigma^2))");
  init_gauss.options().set("functions",functions);
  solver.initial_conditions().execute();

  Field& solution_field = *Handle<Field>( follow_link( solver.field_manager().get_child(sdm::Tags::solution()) ) );

  // Discretization
  Term& convection = solver.domain_discretization().create_term("cf3.sdm.scalar.LinearAdvection1D","convection",std::vector<URI>(1,mesh.topology().uri()));
  convection.options().set("advection_speed",std::vector<Real>(1,2.));
//  // Boundary condition
//  std::vector<URI> bc_regions;
//  bc_regions.push_back(mesh.topology().uri()/"xneg");
//  bc_regions.push_back(mesh.topology().uri()/"xpos");
//  Term& dirichlet = solver.domain_discretization().create_term("cf3.sdm.BCDirichlet","dirichlet",bc_regions);
//  std::vector<std::string> dirichlet_functions;
//  dirichlet_functions.push_back("0.1");
//  dirichlet.set("functions",dirichlet_functions);


  std::vector<Real> cfl(5);
  cfl[1] = 1.;
  cfl[2] = 0.5;
  cfl[3] = 0.3;
  cfl[4] = 0.2254;

  Real cfl_matteo = 1./(2.2*(sol_order-1.)+1.);

  // Time stepping
  solver.time().options().set("time_step",100.);
  solver.time().options().set("end_time", 2.); // instead of 0.3
  solver.time_stepping().options().set("cfl" , common::to_str(cfl_matteo) );

  //////////////////////////////////////////////////////////////////////////////
  // Run simulation

  Field& residual_field = *follow_link(solver.field_manager().get_child(sdm::Tags::residual()))->handle<Field>();
  Field& wave_speed_field = *follow_link(solver.field_manager().get_child(sdm::Tags::wave_speed()))->handle<Field>();

#ifdef GNUPLOT_FOUND
  Gnuplot gp(std::string(GNUPLOT_COMMAND));
  gp << "set terminal png\n";
  gp << "set output 'linearadv1d_P"<<Comm::instance().rank()<<".png'\n";
  gp << "set yrange [-1.2:1.2]\n";
  gp << "set grid\n";
  gp << "set xlabel 'x'\n";
  gp << "set ylabel 'U'\n";
  gp << "set title 'Rank "<<PE::Comm::instance().rank()<<" , P"<<sol_order-1<<"  RK"<<time_order<<"  DOF="<<solution_field.size()<<"   CFL="<<1./(2.2*(sol_order-1.)+1.)<<"'\n";
  gp << "plot ";
  gp << "'-' with points title 'initial solution'"    << ", ";
  gp << "'-' with points title 'final solution'"      << ", ";
  gp << "'-' with points title 'residual'"          << ", ";
  gp << "'-' with points title 'wave_speed'"            << "\n";
  gp.send( solution_field.coordinates().array() , solution_field.array() );
#endif

  model.simulate();

#ifdef GNUPLOT_FOUND
  gp.send( solution_field.coordinates().array() , solution_field.array() );
  gp.send( residual_field.coordinates().array() , residual_field.array() );
  gp.send( wave_speed_field.coordinates().array() , wave_speed_field.array() );
#endif

  CFinfo << "memory: " << OSystem::instance().layer()->memory_usage_str() << CFendl;

  //////////////////////////////////////////////////////////////////////////////
  // Output

  std::vector<URI> fields;
  Field& rank = solution_field.dict().create_field("rankfield");
  Field& rank_sync = solution_field.dict().create_field("rank_sync");
  for (Uint r=0; r<rank.size(); ++r)
  {
    rank[r][0] = rank.rank()[r];
    rank_sync[r][0] = PE::Comm::instance().rank();
  }
  rank_sync.parallelize();
  rank_sync.synchronize();

  fields.push_back(solution_field.uri());
  fields.push_back(solution_field.dict().field("residual").uri());
  fields.push_back(solution_field.dict().field("solution_backup").uri());
  fields.push_back(solution_field.dict().field("wave_speed").uri());
  fields.push_back(solution_field.dict().field("update_coefficient").uri());
  fields.push_back(solution_field.dict().field("convection").uri());
  fields.push_back(solution_field.dict().field("convection_wavespeed").uri());
  mesh.write_mesh("linearadv1d.plt",fields);

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
}
//#endif
//#if 1
BOOST_AUTO_TEST_CASE( solver2d_test )
{
  //////////////////////////////////////////////////////////////////////////////
  // create and configure SFD - Linear advection 2D model
  Uint dim=1;

  Model& model   = *Core::instance().root().create_component<Model>("model2d");
  model.setup("cf3.sdm.SDSolver","cf3.physics.Scalar.Scalar2D");
  PhysModel& physics = model.physics();
  SDSolver& solver  = *model.solver().handle<SDSolver>();
  Domain&   domain  = model.domain();

//  physics.set("v",1.);

  //////////////////////////////////////////////////////////////////////////////
  // create and configure mesh

  // Create a 2D rectangular mesh
  Mesh& mesh = *domain.create_component<Mesh>("mesh");

  Uint DOF = 5;
  Uint order = 3;

  Uint res = 20;//DOF/order;

  Uint sol_order = order;
  Uint time_order = 3;

  std::vector<Uint> nb_cells = list_of( res )( res );
  std::vector<Real> lengths  = list_of( 10. )( 10. );
  std::vector<Real> offsets  = list_of( 0.  )( 0.  );

  SimpleMeshGenerator& generate_mesh = *domain.create_component<SimpleMeshGenerator>("generate_mesh");
  generate_mesh.options().set("mesh",mesh.uri());
  generate_mesh.options().set("nb_cells",nb_cells);
  generate_mesh.options().set("lengths",lengths);
  generate_mesh.options().set("offsets",offsets);
  generate_mesh.options().set("bdry",true);
  generate_mesh.execute();
  build_component_abstract_type<MeshTransformer>("cf3.mesh.actions.LoadBalance","load_balance")->transform(mesh);
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
  solver::Action& init_gauss = solver.initial_conditions().create_initial_condition("gaussian");
  std::vector<std::string> functions;
  // Gaussian wave
  functions.push_back("sigma:="+to_str(lengths[XX]/20.)+";mu:="+to_str(lengths[XX]/2.)+";exp(-((x-mu)^2+(y-mu)^2)/(2*sigma^2))");
  init_gauss.options().set("functions",functions);
  solver.initial_conditions().execute();

  Field& solution_field = *follow_link(solver.field_manager().get_child(sdm::Tags::solution()))->handle<Field>();

  // Discretization
  Term& convection = solver.domain_discretization().create_term("cf3.sdm.scalar.LinearAdvection2D","convection",std::vector<URI>(1,mesh.topology().uri()));
  std::vector<Real> advection_speed(2,0.);
  advection_speed[XX]=1;
  convection.options().set("advection_speed",advection_speed);
  // BC& bc = solver.boundary_conditions().create_boundary_condition("cf3.sdm.BCConstant<1,2>","inlet",std::vector<URI>(1,mesh.topology().access_component("left")->uri()));
  // bc.options().set("constants",std::vector<Real>(1,.5));
  BC& bc = solver.boundary_conditions().create_boundary_condition("cf3.sdm.BCFunction<1,2>","inlet",std::vector<URI>(1,mesh.topology().access_component("left")->uri()));
  bc.options().set("functions",std::vector<std::string>(1,"sin(y*2*pi/10)"));

//  // Boundary condition
//  std::vector<URI> bc_regions;
//  bc_regions.push_back(mesh.topology().uri()/"xneg");
//  bc_regions.push_back(mesh.topology().uri()/"xpos");
//  Term& dirichlet = solver.domain_discretization().create_term("cf3.sdm.BCDirichlet","dirichlet",bc_regions);
//  std::vector<std::string> dirichlet_functions;
//  dirichlet_functions.push_back("0.1");
//  dirichlet.set("functions",dirichlet_functions);


  std::vector<Real> cfl(5);
  cfl[1] = 1.;
  cfl[2] = 0.5;
  cfl[3] = 0.3;
  cfl[4] = 0.2254;

  Real cfl_1d = 1./(2.2*(sol_order-1.)+1.);
  Real cfl_matteo = 1./(std::pow(2.,1./(sol_order))) * cfl_1d;

  // Time stepping
  solver.time().options().set("time_step",100.);
  solver.time().options().set("end_time" , lengths[XX]/3.); // instead of 0.3
  solver.time_stepping().options().set("cfl" , common::to_str(cfl_matteo) );

  //////////////////////////////////////////////////////////////////////////////
  // Run simulation

  std::vector<URI> fields;
  fields.push_back(solution_field.uri());
  fields.push_back(solution_field.dict().field("residual").uri());

  mesh.write_mesh("initial2d.msh",fields);


  Field& residual_field = *follow_link(solver.field_manager().get_child(sdm::Tags::residual()))->handle<Field>();
  Field& wave_speed_field = *follow_link(solver.field_manager().get_child(sdm::Tags::wave_speed()))->handle<Field>();


//#ifdef GNUPLOT_FOUND
//  Gnuplot gp(std::string(GNUPLOT_COMMAND));
//  gp << "set terminal png\n";
//  gp << "set output 'linearadv1d_P"<<Comm::instance().rank()<<".png'\n";
////  gp << "set yrange [-1.2:1.2]\n";
//  gp << "set grid\n";
//  gp << "set xlabel 'x'\n";
//  gp << "set ylabel 'U'\n";
//  gp << "set title 'Rank "<<PE::Comm::instance().rank()<<" , P"<<sol_order-1<<"  RK"<<time_order<<"  DOF="<<solution_field.size()<<"   CFL="<<1./(2.2*(sol_order-1.)+1.)<<"'\n";
//  gp << "plot ";
//  gp << "'-' with linespoints title 'initial solution'"    << ", ";
//  gp << "'-' with linespoints title 'final solution'"      << ", ";
//  gp << "'-' with linespoints title 'residual'"          << ", ";
//  gp << "'-' with linespoints title 'wave_speed'"            << "\n";
//  gp.send( solution_field.coordinates().array() , solution_field.array() );
//#endif

  CFinfo << "cfl = " << cfl_matteo << CFendl;

  model.simulate();

//#ifdef GNUPLOT_FOUND
//  gp.send( solution_field.coordinates().array() , solution_field.array() );
//  gp.send( residual_field.coordinates().array() , residual_field.array() );
//  gp.send( wave_speed_field.coordinates().array() , wave_speed_field.array() );
//#endif

  CFinfo << "memory: " << OSystem::instance().layer()->memory_usage_str() << CFendl;
  CFinfo << "cfl = " << cfl_matteo << CFendl;
  //////////////////////////////////////////////////////////////////////////////
  // Output

  Field& rank = solution_field.dict().create_field("elem_rank");
  Field& rank_sync = solution_field.dict().create_field("rank_sync");
  for (Uint r=0; r<rank.size(); ++r)
  {
    rank[r][0] = rank.rank()[r];
    rank_sync[r][0] = PE::Comm::instance().rank();
  }
  rank_sync.parallelize();
  rank_sync.synchronize();

//  fields.push_back(solution_field.dict().field("solution_backup").uri());
  fields.push_back(solution_field.dict().field("elem_rank").uri());
  fields.push_back(solution_field.dict().field("wave_speed").uri());
  fields.push_back(solution_field.dict().field("update_coefficient").uri());
  fields.push_back(solution_field.dict().field("convection").uri());
  fields.push_back(solution_field.dict().field("convection_wavespeed").uri());

  mesh.write_mesh("linearadv2d.msh",fields);

  mesh.write_mesh("linearadv2d.plt",fields);

  mesh.write_mesh("linearadv2d.pvtu",fields);

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

}


#endif
////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( finalize_mpi )
{
  PE::Comm::instance().finalize();
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
