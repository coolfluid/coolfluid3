// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the ElementCaches of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for cf3::sdm"

#include <boost/flyweight.hpp>

#include <boost/test/unit_test.hpp>
#include <boost/assign/list_of.hpp>
#include "common/Log.hpp"
#include "common/Core.hpp"
#include "common/Environment.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"
#include "common/OSystem.hpp"
#include "common/OSystemLayer.hpp"
#include "common/List.hpp"
#include "common/Group.hpp"

#include "common/PE/Comm.hpp"

#include "math/Consts.hpp"
#include "math/VariablesDescriptor.hpp"
#include "math/VectorialFunction.hpp"

#include "solver/Model.hpp"
#include "solver/Tags.hpp"

#include "physics/PhysModel.hpp"
#include "physics/Variables.hpp"

#include "mesh/Domain.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Field.hpp"
#include "mesh/FieldManager.hpp"
#include "mesh/SimpleMeshGenerator.hpp"
#include "mesh/MeshTransformer.hpp"
#include "mesh/Region.hpp"
#include "mesh/Space.hpp"
#include "mesh/Cells.hpp"
#include "mesh/ElementConnectivity.hpp"
#include "mesh/FaceCellConnectivity.hpp"
#include "mesh/actions/BuildFaces.hpp"

#include "sdm/ElementCaching.hpp"
#include "sdm/Reconstructions.hpp"
#include "sdm/SDSolver.hpp"
#include "sdm/Term.hpp"
#include "sdm/Tags.hpp"
#include "sdm/ShapeFunction.hpp"
#include "sdm/Operations.hpp"

#include "Tools/Gnuplot/Gnuplot.hpp"
#include <common/Link.hpp>

using namespace boost::assign;
using namespace cf3;
using namespace cf3::math;
using namespace cf3::common;
using namespace cf3::common::PE;
using namespace cf3::mesh;
using namespace cf3::physics;
using namespace cf3::solver;
using namespace cf3::sdm;

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

namespace sandbox
{


template <typename PHYSDATA>
class ConvectiveTerm : public Component
{
public:
  enum {NEQS=PHYSDATA::_neqs};
  enum {NDIM=PHYSDATA::_ndim};

  static std::string type_name() { return "ConvectiveTerm"; }
  ConvectiveTerm(const std::string& name) : Component(name)
  {
    flux_pt_data.resize(1);
  }

  ~ConvectiveTerm() {}
//  virtual void compute_face_data() = 0;

  template<typename VAR>
  void reconstruct_to_flx_pt(const Handle<Field>& field, VAR& var)
  {
    std::cout << "reconstructing field to var " << std::endl;
  }

  std::vector<PHYSDATA> flux_pt_data;
};


class PhysData
{
public:
  enum {_neqs=4};
  enum {_ndim=2};
public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  RealVector4 solution;
  RealVector2 coords;
};

class ConcreteTerm: public ConvectiveTerm<PhysData>
{
public:
  static std::string type_name() { return "ConcreteTerm"; }
  ConcreteTerm(const std::string& name) : ConvectiveTerm<PhysData>(name) {}
  ~ConcreteTerm() {}

  virtual void compute_flx_pt_data(const SFDElement& elem, const Uint flx_pt, PhysData& phys_data)
  {
//    elem.reconstruct_solution_space_to_flux_points[flx_pt](solution->view(elem.idx), phys_data.solution);
//    elem.reconstruct_solution_space_to_flux_points[flx_pt](coords->view(elem.idx)  , phys_data.coords  );
  }

//  virtual void compute_face_data()
//  {
//    //reconstruct_to_flx_pt()
//    phys_data.solution.setZero();
//    phys_data.coords.setZero();
//  }

  Handle<Field> solution;
  Handle<Field> coords;
};

} // namespace sandbox
using namespace sandbox;
BOOST_AUTO_TEST_CASE( sandbox_convection )
{
  boost::shared_ptr<ConcreteTerm> term = allocate_component<ConcreteTerm>("term");
//  term->compute_face_data();

//  BOOST_CHECK_EQUAL(term->properties.rho     , 1.);
//  BOOST_CHECK_EQUAL(term->properties.rhoU[XX], 1.);
//  BOOST_CHECK_EQUAL(term->properties.rhoU[YY], 0.);
//  BOOST_CHECK_EQUAL(term->properties.rhoE    , 2.);
}

# if 0
BOOST_AUTO_TEST_CASE( test_P0 )
{

  //////////////////////////////////////////////////////////////////////////////
  Uint dim=1;

  Model& model   = *Core::instance().root().create_component<Model>("test_P0");
  model.setup("cf3.sdm.SDSolver","cf3.physics.Scalar.Scalar1D");
  PhysModel& physics = model.physics();
  SDSolver& solver  = *model.solver().handle<SDSolver>();
  Domain&   domain  = model.domain();

  physics.options().set("v",1.);

  //////////////////////////////////////////////////////////////////////////////
  // create and configure mesh

  // Create a 2D rectangular mesh
  Mesh& mesh = *domain.create_component<Mesh>("mesh");

  Uint res        = 4;
  Uint sol_order  = 1;
  Uint time_order = 1;

  std::vector<Uint> nb_cells = list_of( res  );
  std::vector<Real> lengths  = list_of(  8.  );
  std::vector<Real> offsets  = list_of(  -3.  );

  SimpleMeshGenerator& generate_mesh = *domain.create_component<SimpleMeshGenerator>("generate_mesh");
  generate_mesh.options().set("mesh",mesh.uri());
  generate_mesh.options().set("nb_cells",nb_cells);
  generate_mesh.options().set("lengths",lengths);
  generate_mesh.options().set("offsets",offsets);
  generate_mesh.options().set("bdry",false);
  generate_mesh.execute();
  build_component_abstract_type<MeshTransformer>("cf3.mesh.actions.LoadBalance","load_balance")->transform(mesh);
  solver.options().set(sdm::Tags::mesh(),mesh.handle<Mesh>());

  //////////////////////////////////////////////////////////////////////////////
  // Prepare the mesh

  solver.options().set(sdm::Tags::solution_vars(),std::string("cf3.physics.Scalar.LinearAdv1D"));
  solver.options().set(sdm::Tags::solution_order(),sol_order);
  solver.iterative_solver().options().set("nb_stages",time_order);
  solver.prepare_mesh().execute();

  //////////////////////////////////////////////////////////////////////////////
  // Configure simulation


  // Initial condition
  solver::Action& init = solver.initial_conditions().create_initial_condition("gaussian");
  std::vector<std::string> functions;
  // Gaussian wave
  functions.push_back("3-abs(x)");
  init.options().set("functions",functions);
  solver.initial_conditions().execute();

  Field& solution_field = *follow_link(solver.field_manager().get_child(sdm::Tags::solution()))->handle<Field>();

  // Discretization
  solver.domain_discretization().create_term("cf3.sdm.scalar.LinearAdvection1D","convection",std::vector<URI>(1,mesh.topology().uri()));

  // Time stepping
  solver.time_stepping().time().options().set("time_step",100.);
  solver.time_stepping().time().options().set("end_time" , 2.); // instead of 0.3
  solver.time_stepping().configure_option_recursively("cfl" ,std::string("1."));

  //////////////////////////////////////////////////////////////////////////////
  // Run simulation

  Field& residual_field = *follow_link(solver.field_manager().get_child(sdm::Tags::residual()))->handle<Field>();


#ifdef GNUPLOT_FOUND
  Gnuplot gp(std::string(GNUPLOT_COMMAND));
  gp << "set terminal png\n";
  gp << "set output 'test_P0.png'\n";
//  gp << "set yrange [-1.2:1.2]\n";
  gp << "set grid\n";
  gp << "set xlabel 'x'\n";
  gp << "set ylabel 'U'\n";
  gp << "set title 'Rank "<<PE::Comm::instance().rank()<<" , P"<<sol_order-1<<"  RK"<<time_order<<"'\n";
  gp << "plot ";
  gp << "'-' with linespoints title 'initial solution'"    << ", ";
  gp << "'-' with linespoints title 'final solution'"      << ", ";
  gp << "'-' with linespoints title 'residual'"            << "\n";
  gp.send( xy(solution_field) );
#endif

  model.simulate();

#ifdef GNUPLOT_FOUND
  gp.send( xy(solution_field) );
  gp.send( xy(residual_field) );
#endif

  CFinfo << "memory: " << OSystem::instance().layer()->memory_usage_str() << CFendl;

  std::cout << mesh.tree() << std::endl;

  /// CHECKS
  BOOST_CHECK_EQUAL(solver.time_stepping().properties().value<Uint>("iteration") , 2u);
  BOOST_CHECK_EQUAL(solver.time_stepping().time().dt() , 2.);

  Connectivity& node = *mesh.access_component("topology/interior/cells/Line/spaces/solution_space/connectivity")->handle<Connectivity>();

  BOOST_CHECK_EQUAL(residual_field[node[0][0]][0] ,  0.);

  BOOST_CHECK_EQUAL(residual_field[node[1][0]][0] , -1.);

  BOOST_CHECK_EQUAL(residual_field[node[2][0]][0] , 1.);

  BOOST_CHECK_EQUAL(residual_field[node[3][0]][0] , 1.);


  //////////////////////////////////////////////////////////////////////////////
  // Output

  std::vector<URI> fields;
  Field& rank = solution_field.dict().create_field("rank");
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

  //std::cout << "solution_field.max = " << max.transpose() << std::endl;
  //std::cout << "solution_field.min = " << min.transpose() << std::endl;

}

BOOST_AUTO_TEST_CASE( test_P1 )
{
  //////////////////////////////////////////////////////////////////////////////
  // create and configure SFD - LinEuler 2D model
  Uint dim=1;

  Model& model   = *Core::instance().root().create_component<Model>("test_P1");
  model.setup("cf3.sdm.SDSolver","cf3.physics.Scalar.Scalar1D");
  PhysModel& physics = model.physics();
  SDSolver& solver  = *model.solver().handle<SDSolver>();
  Domain&   domain  = model.domain();

  physics.options().set("v",1.);

  //////////////////////////////////////////////////////////////////////////////
  // create and configure mesh

  // Create a 1D line mesh
  Mesh& mesh = *domain.create_component<Mesh>("mesh");

  Uint res        = 4;
  Uint sol_order  = 2;
  Uint time_order = 1;

  std::vector<Uint> nb_cells = list_of( res  );
  std::vector<Real> lengths  = list_of(  8.  );
  std::vector<Real> offsets  = list_of(  -3.  );

  SimpleMeshGenerator& generate_mesh = *domain.create_component<SimpleMeshGenerator>("generate_mesh");
  generate_mesh.options().set("mesh",mesh.uri());
  generate_mesh.options().set("nb_cells",nb_cells);
  generate_mesh.options().set("lengths",lengths);
  generate_mesh.options().set("offsets",offsets);
  generate_mesh.options().set("bdry",false);
  generate_mesh.execute();
  build_component_abstract_type<MeshTransformer>("cf3.mesh.actions.LoadBalance","load_balance")->transform(mesh);
  solver.options().set(sdm::Tags::mesh(),mesh.handle<Mesh>());

  //////////////////////////////////////////////////////////////////////////////
  // Prepare the mesh

  solver.options().set(sdm::Tags::solution_vars(),std::string("cf3.physics.Scalar.LinearAdv1D"));
  solver.options().set(sdm::Tags::solution_order(),sol_order);
  solver.iterative_solver().options().set("nb_stages",time_order);
  solver.prepare_mesh().execute();

  //////////////////////////////////////////////////////////////////////////////
  // Configure simulation


  // Initial condition
  solver::Action& init = solver.initial_conditions().create_initial_condition("gaussian");
  std::vector<std::string> functions;
  // Gaussian wave
  functions.push_back("3-abs(x)");
  init.options().set("functions",functions);
  solver.initial_conditions().execute();

  Field& solution_field = *follow_link(solver.field_manager().get_child(sdm::Tags::solution()))->handle<Field>();

  // Discretization
  solver.domain_discretization().create_term("cf3.sdm.scalar.LinearAdvection1D","convection",std::vector<URI>(1,mesh.topology().uri()));

//  // Boundary condition
//  std::vector<URI> bc_regions;
//  bc_regions.push_back(mesh.topology().uri()/"xneg");
//  bc_regions.push_back(mesh.topology().uri()/"xpos");
//  ElementCache& dirichlet = solver.domain_discretization().create_ElementCache("cf3.sdm.BCDirichlet","dirichlet",bc_regions);
//  std::vector<std::string> dirichlet_functions;
//  dirichlet_functions.push_back("0");
//  dirichlet.set("functions",dirichlet_functions);

  // Time stepping
  solver.time_stepping().time().options().set("time_step",100.);
  solver.time_stepping().time().options().set("end_time" , 2.); // instead of 0.3
  solver.time_stepping().options().set("cfl" , std::string("1."));

  //////////////////////////////////////////////////////////////////////////////
  // Run simulation

  Field& residual_field = *follow_link(solver.field_manager().get_child(sdm::Tags::residual()))->handle<Field>();


#ifdef GNUPLOT_FOUND
  Gnuplot gp(std::string(GNUPLOT_COMMAND));
  gp << "set terminal png\n";
  gp << "set output 'test_P1.png'\n";
//  gp << "set yrange [-1.2:1.2]\n";
  gp << "set grid\n";
  gp << "set xlabel 'x'\n";
  gp << "set ylabel 'U'\n";
  gp << "set title 'Rank "<<PE::Comm::instance().rank()<<" , P"<<sol_order-1<<"  RK"<<time_order<<"'\n";
  gp << "plot ";
  gp << "'-' with linespoints title 'initial solution'"    << ", ";
  gp << "'-' with linespoints title 'final solution'"      << ", ";
  gp << "'-' with linespoints title 'residual'"            << "\n";
  gp.send( solution_field.coordinates().array() , solution_field.array() );
#endif

  model.simulate();

#ifdef GNUPLOT_FOUND
  gp.send( solution_field.coordinates().array() , solution_field.array() );
  gp.send( residual_field.coordinates().array() , residual_field.array() );
#endif

  CFinfo << "memory: " << OSystem::instance().layer()->memory_usage_str() << CFendl;

  /// CHECKS
  BOOST_CHECK_EQUAL(solver.time_stepping().properties().value<Uint>("iteration") , 2u);
  BOOST_CHECK_EQUAL(solver.time_stepping().time().dt() , 2.);

  BOOST_CHECK_EQUAL(residual_field.size()     , 10u);
  BOOST_CHECK_EQUAL(residual_field.row_size() , 1u);

  // cells
  Connectivity& node = *mesh.access_component("topology/interior/cells/Line/spaces/solution_space/connectivity")->handle<Connectivity>();
  BOOST_CHECK_EQUAL(residual_field[node[0][0]][0] , -1.);
  BOOST_CHECK_EQUAL(residual_field[node[0][1]][0] , -1.);

  BOOST_CHECK_EQUAL(residual_field[node[1][0]][0] , 0.);
  BOOST_CHECK_EQUAL(residual_field[node[1][1]][0] , 0.);

  BOOST_CHECK_EQUAL(residual_field[node[2][0]][0] , 1.);
  BOOST_CHECK_EQUAL(residual_field[node[2][1]][0] , 1.);

  BOOST_CHECK_EQUAL(residual_field[node[3][0]][0] , 1.);
  BOOST_CHECK_EQUAL(residual_field[node[3][1]][0] , 1.);

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

  //std::cout << "solution_field.max = " << max.transpose() << std::endl;
  //std::cout << "solution_field.min = " << min.transpose() << std::endl;

}

BOOST_AUTO_TEST_CASE( test_P2 )
{

  //////////////////////////////////////////////////////////////////////////////
  // create and configure SFD - LinEuler 2D model
  Uint dim=1;

  Model& model   = *Core::instance().root().create_component<Model>("test_P2");
  model.setup("cf3.sdm.SDSolver","cf3.physics.Scalar.Scalar1D");
  PhysModel& physics = model.physics();
  SDSolver& solver  = *model.solver().handle<SDSolver>();
  Domain&   domain  = model.domain();

  physics.options().set("v",1.);

  //////////////////////////////////////////////////////////////////////////////
  // create and configure mesh

  // Create a 2D rectangular mesh
  Mesh& mesh = *domain.create_component<Mesh>("mesh");

  Uint res        = 4;
  Uint sol_order  = 3;
  Uint time_order = 1;

  std::vector<Uint> nb_cells = list_of( res  );
  std::vector<Real> lengths  = list_of(  8.  );
  std::vector<Real> offsets  = list_of(  -3.  );

  SimpleMeshGenerator& generate_mesh = *domain.create_component<SimpleMeshGenerator>("generate_mesh");
  generate_mesh.options().set("mesh",mesh.uri());
  generate_mesh.options().set("nb_cells",nb_cells);
  generate_mesh.options().set("lengths",lengths);
  generate_mesh.options().set("offsets",offsets);
  generate_mesh.options().set("bdry",false);
  generate_mesh.execute();
  build_component_abstract_type<MeshTransformer>("cf3.mesh.actions.LoadBalance","load_balance")->transform(mesh);
  solver.options().set(sdm::Tags::mesh(),mesh.handle<Mesh>());

  //////////////////////////////////////////////////////////////////////////////
  // Prepare the mesh

  solver.options().set(sdm::Tags::solution_vars(),std::string("cf3.physics.Scalar.LinearAdv1D"));
  solver.options().set(sdm::Tags::solution_order(),sol_order);
  solver.iterative_solver().options().set("nb_stages",time_order);
  solver.prepare_mesh().execute();

  //////////////////////////////////////////////////////////////////////////////
  // Configure simulation


  // Initial condition
  solver::Action& init = solver.initial_conditions().create_initial_condition("gaussian");
  std::vector<std::string> functions;
  // Gaussian wave
  functions.push_back("3-abs(x)");
  init.options().set("functions",functions);
  solver.initial_conditions().execute();

  Field& solution_field = *follow_link(solver.field_manager().get_child(sdm::Tags::solution()))->handle<Field>();

  // Discretization
  solver.domain_discretization().create_term("cf3.sdm.scalar.LinearAdvection1D","convection",std::vector<URI>(1,mesh.topology().uri()));

//  // Boundary condition
//  std::vector<URI> bc_regions;
//  bc_regions.push_back(mesh.topology().uri()/"xneg");
//  bc_regions.push_back(mesh.topology().uri()/"xpos");
//  ElementCache& dirichlet = solver.domain_discretization().create_ElementCache("cf3.sdm.BCDirichlet","dirichlet",bc_regions);
//  std::vector<std::string> dirichlet_functions;
//  dirichlet_functions.push_back("0");
//  dirichlet.set("functions",dirichlet_functions);

  // Time stepping
  solver.time_stepping().time().options().set("time_step",100.);
  solver.time_stepping().time().options().set("end_time" , 2.); // instead of 0.3
  solver.time_stepping().options().set("cfl" , std::string("1."));

  //////////////////////////////////////////////////////////////////////////////
  // Run simulation

  Field& residual_field = *follow_link(solver.field_manager().get_child(sdm::Tags::residual()))->handle<Field>();


#ifdef GNUPLOT_FOUND
  Gnuplot gp(std::string(GNUPLOT_COMMAND));
  gp << "set terminal png\n";
  gp << "set output 'test_P2.png'\n";
//  gp << "set yrange [-1.2:1.2]\n";
  gp << "set grid\n";
  gp << "set xlabel 'x'\n";
  gp << "set ylabel 'U'\n";
  gp << "set title 'Rank "<<PE::Comm::instance().rank()<<" , P"<<sol_order-1<<"  RK"<<time_order<<"'\n";
  gp << "plot ";
  gp << "'-' with linespoints title 'initial solution'"    << ", ";
  gp << "'-' with linespoints title 'final solution'"      << ", ";
  gp << "'-' with linespoints title 'residual'"            << "\n";
  gp.send( solution_field.coordinates().array() , solution_field.array() );
#endif

  model.simulate();

#ifdef GNUPLOT_FOUND
  gp.send( solution_field.coordinates().array() , solution_field.array() );
  gp.send( residual_field.coordinates().array() , residual_field.array() );
#endif

  CFinfo << "memory: " << OSystem::instance().layer()->memory_usage_str() << CFendl;

  /// CHECKS
  Real fraction = 100*math::Consts::eps();
  BOOST_CHECK_EQUAL(solver.time_stepping().properties().value<Uint>("iteration") , 2u);
  BOOST_CHECK_EQUAL(solver.time_stepping().time().dt() , 2.);

  BOOST_CHECK_EQUAL(residual_field.size()     , 14u);
  BOOST_CHECK_EQUAL(residual_field.row_size() , 1u);

  // interior
  Connectivity& node = *mesh.access_component("topology/interior/cells/Line/spaces/solution_space/connectivity")->handle<Connectivity>();
  BOOST_CHECK_CLOSE_FRACTION(residual_field[node[0][0]][0] , -1.  , fraction);
  BOOST_CHECK_CLOSE_FRACTION(residual_field[node[0][1]][0] , -1.  , fraction);
  BOOST_CHECK_CLOSE_FRACTION(residual_field[node[0][2]][0] , -1.  , fraction);

  BOOST_CHECK_CLOSE_FRACTION(residual_field[node[1][0]][0] , -2.  , fraction);
  BOOST_CHECK_SMALL(residual_field[node[1][1]][0]                 , fraction);
  BOOST_CHECK_CLOSE_FRACTION(residual_field[node[1][2]][0] ,  2.  , fraction);

  BOOST_CHECK_CLOSE_FRACTION(residual_field[node[2][0]][0] ,  1.  , fraction);
  BOOST_CHECK_CLOSE_FRACTION(residual_field[node[2][1]][0] ,  1.  , fraction);
  BOOST_CHECK_CLOSE_FRACTION(residual_field[node[2][2]][0] ,  1.  , fraction);

  BOOST_CHECK_CLOSE_FRACTION(residual_field[node[3][0]][0] ,  1.  , fraction);
  BOOST_CHECK_CLOSE_FRACTION(residual_field[node[3][1]][0],  1.  , fraction);
  BOOST_CHECK_CLOSE_FRACTION(residual_field[node[3][2]][0],  1.  , fraction);

  //////////////////////////////////////////////////////////////////////////////
  // Output

  std::vector<URI> fields;
  Field& rank = solution_field.dict().create_field("rank");
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

  //std::cout << "solution_field.max = " << max.transpose() << std::endl;
  //std::cout << "solution_field.min = " << min.transpose() << std::endl;

}

BOOST_AUTO_TEST_CASE( test_P3 )
{

  //////////////////////////////////////////////////////////////////////////////
  Uint dim=1;

  Model& model   = *Core::instance().root().create_component<Model>("test_P3");
  model.setup("cf3.sdm.SDSolver","cf3.physics.Scalar.Scalar1D");
  PhysModel& physics = model.physics();
  SDSolver& solver  = *model.solver().handle<SDSolver>();
  Domain&   domain  = model.domain();

  physics.options().set("v",1.);

  //////////////////////////////////////////////////////////////////////////////
  // create and configure mesh

  // Create a 2D rectangular mesh
  Mesh& mesh = *domain.create_component<Mesh>("mesh");

  Uint res        = 4;
  Uint sol_order  = 4;
  Uint time_order = 1;

  std::vector<Uint> nb_cells = list_of( res  );
  std::vector<Real> lengths  = list_of(  8.  );
  std::vector<Real> offsets  = list_of(  -3.  );

  SimpleMeshGenerator& generate_mesh = *domain.create_component<SimpleMeshGenerator>("generate_mesh");
  generate_mesh.options().set("mesh",mesh.uri());
  generate_mesh.options().set("nb_cells",nb_cells);
  generate_mesh.options().set("lengths",lengths);
  generate_mesh.options().set("offsets",offsets);
  generate_mesh.options().set("bdry",false);
  generate_mesh.execute();
  build_component_abstract_type<MeshTransformer>("cf3.mesh.actions.LoadBalance","load_balance")->transform(mesh);
  solver.options().set(sdm::Tags::mesh(),mesh.handle<Mesh>());

  //////////////////////////////////////////////////////////////////////////////
  // Prepare the mesh

  solver.options().set(sdm::Tags::solution_vars(),std::string("cf3.physics.Scalar.LinearAdv1D"));
  solver.options().set(sdm::Tags::solution_order(),sol_order);
  solver.iterative_solver().options().set("nb_stages",time_order);
  solver.prepare_mesh().execute();

  //////////////////////////////////////////////////////////////////////////////
  // Configure simulation


  // Initial condition
  solver::Action& init = solver.initial_conditions().create_initial_condition("gaussian");
  std::vector<std::string> functions;
  // Gaussian wave
  functions.push_back("3-abs(x)");
  init.options().set("functions",functions);
  solver.initial_conditions().execute();

  Field& solution_field = *follow_link(solver.field_manager().get_child(sdm::Tags::solution()))->handle<Field>();

  // Discretization
  solver.domain_discretization().create_term("cf3.sdm.scalar.LinearAdvection1D","convection",std::vector<URI>(1,mesh.topology().uri()));

  // Boundary condition
  std::vector<URI> bc_regions;
//  bc_regions.push_back(mesh.topology().uri()/"xneg");
//  bc_regions.push_back(mesh.topology().uri()/"xpos");
//  ElementCache& dirichlet = solver.domain_discretization().create_ElementCache("cf3.sdm.BCDirichlet","dirichlet",bc_regions);
//  std::vector<std::string> dirichlet_functions;
//  dirichlet_functions.push_back("0");
//  dirichlet.set("functions",dirichlet_functions);

  // Time stepping
  solver.time_stepping().time().options().set("time_step",100.);
  solver.time_stepping().time().options().set("end_time" , 2.); // instead of 0.3
  solver.time_stepping().options().set("cfl" , std::string("1."));

  //////////////////////////////////////////////////////////////////////////////
  // Run simulation

  Field& residual_field = *follow_link(solver.field_manager().get_child(sdm::Tags::residual()))->handle<Field>();


#ifdef GNUPLOT_FOUND
  Gnuplot gp(std::string(GNUPLOT_COMMAND));
  gp << "set terminal png\n";
  gp << "set output 'test_P3.png'\n";
//  gp << "set yrange [-1.2:1.2]\n";
  gp << "set grid\n";
  gp << "set xlabel 'x'\n";
  gp << "set ylabel 'U'\n";
  gp << "set title 'Rank "<<PE::Comm::instance().rank()<<" , P"<<sol_order-1<<"  RK"<<time_order<<"'\n";
  gp << "plot ";
  gp << "'-' with linespoints title 'initial solution'"    << ", ";
  gp << "'-' with linespoints title 'final solution'"      << ", ";
  gp << "'-' with linespoints title 'residual'"            << "\n";
  gp.send( solution_field.coordinates().array() , solution_field.array() );
#endif

  model.simulate();

#ifdef GNUPLOT_FOUND
  gp.send( solution_field.coordinates().array() , solution_field.array() );
  gp.send( residual_field.coordinates().array() , residual_field.array() );
#endif

  CFinfo << "memory: " << OSystem::instance().layer()->memory_usage_str() << CFendl;

  /// CHECKS
  Real fraction = 100*math::Consts::eps();

  BOOST_CHECK_EQUAL(solver.time_stepping().properties().value<Uint>("iteration") , 2u);
  BOOST_CHECK_EQUAL(solver.time_stepping().time().dt() , 2.);

  BOOST_CHECK_EQUAL(residual_field.size()     , 18u);
  BOOST_CHECK_EQUAL(residual_field.row_size() , 1u);

  Connectivity& node = *mesh.access_component("topology/interior/cells/Line/spaces/solution_space/connectivity")->handle<Connectivity>();
  BOOST_CHECK_CLOSE_FRACTION(residual_field[node[0][0]][0] , -1.  , fraction);
  BOOST_CHECK_CLOSE_FRACTION(residual_field[node[0][1]][0] , -1.  , fraction);
  BOOST_CHECK_CLOSE_FRACTION(residual_field[node[0][2]][0] , -1.  , fraction);
  BOOST_CHECK_CLOSE_FRACTION(residual_field[node[0][3]][0] , -1.  , fraction);

  BOOST_CHECK_CLOSE_FRACTION(residual_field[node[1][0]][0] , -1.1270166537925839  , fraction);
  BOOST_CHECK_CLOSE_FRACTION(residual_field[node[1][1]][0] , -0.87298334620741969 , fraction);
  BOOST_CHECK_CLOSE_FRACTION(residual_field[node[1][2]][0] ,  0.87298334620741969 , fraction);
  BOOST_CHECK_CLOSE_FRACTION(residual_field[node[1][3]][0] ,  1.1270166537925839  , fraction);

  BOOST_CHECK_CLOSE_FRACTION(residual_field[node[2][0]][0] ,  1.  , fraction);
  BOOST_CHECK_CLOSE_FRACTION(residual_field[node[2][1]][0] ,  1.  , fraction);
  BOOST_CHECK_CLOSE_FRACTION(residual_field[node[2][2]][0] , 1.  , fraction);
  BOOST_CHECK_CLOSE_FRACTION(residual_field[node[2][3]][0] , 1.  , fraction);

  BOOST_CHECK_CLOSE_FRACTION(residual_field[node[3][0]][0] , 1.  , fraction);
  BOOST_CHECK_CLOSE_FRACTION(residual_field[node[3][1]][0],  1.  , fraction);
  BOOST_CHECK_CLOSE_FRACTION(residual_field[node[3][2]][0],  1.  , fraction);
  BOOST_CHECK_CLOSE_FRACTION(residual_field[node[3][3]][0],  1.  , fraction);

  //////////////////////////////////////////////////////////////////////////////
  // Output

  std::vector<URI> fields;
  Field& rank = solution_field.dict().create_field("rank");
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

  //std::cout << "solution_field.max = " << max.transpose() << std::endl;
  //std::cout << "solution_field.min = " << min.transpose() << std::endl;

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
