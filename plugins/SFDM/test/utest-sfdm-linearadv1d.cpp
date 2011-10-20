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
#include "common/CRoot.hpp"
#include "common/CEnv.hpp"
#include "common/OSystem.hpp"
#include "common/OSystemLayer.hpp"

#include "common/PE/Comm.hpp"

#include "Math/VariablesDescriptor.hpp"

#include "Solver/CModel.hpp"
#include "Solver/Tags.hpp"

#include "Physics/PhysModel.hpp"
#include "Physics/Variables.hpp"

#include "Mesh/CDomain.hpp"
#include "Mesh/Geometry.hpp"
#include "Mesh/Field.hpp"
#include "Mesh/FieldManager.hpp"
#include "Mesh/CSimpleMeshGenerator.hpp"
#include "Mesh/CMeshTransformer.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CLinearInterpolator.hpp"

#include "SFDM/SFDSolver.hpp"
#include "SFDM/Term.hpp"
#include "SFDM/Tags.hpp"

#include "Tools/Gnuplot/Gnuplot.hpp"

//#include "Mesh/CMesh.hpp"
//#include "Mesh/CField.hpp"
//#include "Mesh/CEntities.hpp"
//#include "Mesh/ElementType.hpp"
//#include "Mesh/CMeshWriter.hpp"
//#include "Mesh/CDomain.hpp"
//#include "Mesh/Actions/CInitFieldFunction.hpp"
//#include "Mesh/Actions/CreateSpaceP0.hpp"
//#include "Solver/CModelUnsteady.hpp"
//#include "Solver/CSolver.hpp"
//#include "Solver/CPhysicalModel.hpp"
//#include "Mesh/Actions/CBuildFaces.hpp"
//#include "Mesh/Actions/CBuildVolume.hpp"
//#include "Mesh/Actions/CreateSpaceP0.hpp"
//#include "SFDM/CreateSpace.hpp"

using namespace boost::assign;
using namespace cf3;
using namespace cf3::Math;
using namespace cf3::common;
using namespace cf3::common::PE;
using namespace cf3::Mesh;
using namespace cf3::Physics;
using namespace cf3::Solver;
using namespace cf3::SFDM;

std::map<Real,Real> xy(const Field& field)
{
  std::map<Real,Real> map;
  for (Uint i=0; i<field.size(); ++i)
    map[field.coordinates()[i][0]] = field[i][0];
  return map;
}

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

BOOST_FIXTURE_TEST_SUITE( SFDM_MPITests_TestSuite, SFDM_MPITests_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( init_mpi )
{
  PE::Comm::instance().init(m_argc,m_argv);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Solver_test )
{
  Core::instance().environment().configure_option("log_level", (Uint)INFO);

  //////////////////////////////////////////////////////////////////////////////
  // create and configure SFD - LinEuler 2D model
  Uint dim=1;

  CModel& model   = Core::instance().root().create_component<CModel>("model");
  model.setup("CF.SFDM.SFDSolver","CF.Physics.Scalar.Scalar1D");
  PhysModel& physics = model.physics();
  SFDSolver& solver  = model.solver().as_type<SFDSolver>();
  CDomain&   domain  = model.domain();

  physics.configure_option("v",1.);

  //////////////////////////////////////////////////////////////////////////////
  // create and configure mesh

  // Create a 2D rectangular mesh
  CMesh& mesh = domain.create_component<CMesh>("mesh");

  Uint res = 20;
  Uint order = 3;
  std::vector<Uint> nb_cells = list_of( res  );
  std::vector<Real> lengths  = list_of(  10.  );
  std::vector<Real> offsets  = list_of(  0.  );

  CSimpleMeshGenerator& generate_mesh = domain.create_component<CSimpleMeshGenerator>("generate_mesh");
  generate_mesh.configure_option("mesh",mesh.uri());
  generate_mesh.configure_option("nb_cells",nb_cells);
  generate_mesh.configure_option("lengths",lengths);
  generate_mesh.configure_option("offsets",offsets);
  generate_mesh.configure_option("bdry",false);
  generate_mesh.execute();
  build_component_abstract_type<CMeshTransformer>("CF.Mesh.Actions.LoadBalance","load_balance")->transform(mesh);
  solver.configure_option(SFDM::Tags::mesh(),mesh.uri());

  //////////////////////////////////////////////////////////////////////////////
  // Prepare the mesh

  solver.configure_option(SFDM::Tags::solution_vars(),std::string("CF.Physics.Scalar.LinearAdv1D"));
  solver.configure_option(SFDM::Tags::solution_order(),order);
  solver.iterative_solver().configure_option("rk_order",order);
  solver.prepare_mesh().execute();

  //////////////////////////////////////////////////////////////////////////////
  // Configure simulation


  // Initial condition
  Solver::Action& init_gauss = solver.initial_conditions().create_initial_condition("gaussian");
  std::vector<std::string> functions;
  // Gaussian wave
  functions.push_back("sigma:=0.5;mu:=5;exp(-(x-mu)^2/(2*sigma^2))");
  init_gauss.configure_option("functions",functions);
  solver.initial_conditions().execute();

  Field& solution_field = solver.field_manager().get_child(SFDM::Tags::solution()).follow()->as_type<Field>();
  solution_field.field_group().create_coordinates();

  // Discretization
  solver.domain_discretization().create_term("CF.SFDM.Convection","convection",std::vector<URI>(1,mesh.topology().uri()));

  // Time stepping
  solver.time_stepping().time().configure_option("time_step",100.);
  solver.time_stepping().time().configure_option("end_time" ,2.); // instead of 0.3
  solver.time_stepping().configure_option_recursively("cfl" , 0.4);
  solver.time_stepping().configure_option_recursively("milestone_dt" , 100.);

  //////////////////////////////////////////////////////////////////////////////
  // Run simulation

  CFinfo << model.tree() << CFendl;


#ifdef GNUPLOT_FOUND
  Gnuplot gp(std::string(GNUPLOT_COMMAND));
  gp << "set terminal png\n";
  gp << "set output 'linearadv1d_P"<<Comm::instance().rank()<<".png'\n";
  gp << "set yrange [-0.2:1.2]\n";
  gp << "set grid\n";
  gp << "set xlabel 'x'\n";
  gp << "set ylabel 'U'\n";
  gp << "set title 'Rank "<<PE::Comm::instance().rank()<<"'\n";
  gp << "plot ";
  gp << "'-' with linespoints title 'initial solution'"    << ", ";
  gp << "'-' with linespoints title 'final solution'"      << "\n";
  gp.send( xy(solution_field) );
#endif

  model.simulate();

#ifdef GNUPLOT_FOUND
  gp.send( xy(solution_field) );
#endif

  CFinfo << "memory: " << OSystem::instance().layer()->memory_usage_str() << CFendl;

  //////////////////////////////////////////////////////////////////////////////
  // Output

  std::vector<URI> fields;
  Field& rank = solution_field.field_group().create_field("rank");
  Field& rank_sync = solution_field.field_group().create_field("rank_sync");
  for (Uint r=0; r<rank.size(); ++r)
  {
    rank[r][0] = rank.rank()[r];
    rank_sync[r][0] = PE::Comm::instance().rank();
  }
  rank_sync.parallelize();
  rank_sync.synchronize();

  fields.push_back(solution_field.uri());
  fields.push_back(solution_field.field_group().field("residual").uri());
  fields.push_back(solution_field.field_group().field("solution_backup").uri());
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

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( finalize_mpi )
{
  PE::Comm::instance().finalize();
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
