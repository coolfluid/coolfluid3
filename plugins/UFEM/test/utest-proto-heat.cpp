// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for heat-conduction related proto operations"

#include <boost/foreach.hpp>
#include <boost/test/unit_test.hpp>

#include "Solver/Actions/Proto/BlockAccumulator.hpp"
#include "Solver/Actions/Proto/CProtoAction.hpp"
#include "Solver/Actions/Proto/ElementLooper.hpp"
#include "Solver/Actions/Proto/Expression.hpp"
#include "Solver/Actions/Proto/Functions.hpp"
#include "Solver/Actions/Proto/NodeLooper.hpp"
#include "Solver/Actions/Proto/Terminals.hpp"

#include "Common/Core.hpp"
#include "Common/CRoot.hpp"
#include "Common/Log.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CMeshReader.hpp"
#include "Mesh/ElementData.hpp"

#include "Mesh/Integrators/Gauss.hpp"
#include "Mesh/SF/Types.hpp"

#include "Solver/CEigenLSS.hpp"
#include "Solver/CPhysicalModel.hpp"

#include "Tools/MeshGeneration/MeshGeneration.hpp"

#include "UFEM/LinearProblem.hpp"
#include "UFEM/SteadyModel.hpp"


using namespace CF;
using namespace CF::Solver;
using namespace CF::Solver::Actions;
using namespace CF::Solver::Actions::Proto;
using namespace CF::Common;
using namespace CF::Math::MathConsts;
using namespace CF::Mesh;

using namespace boost;


typedef std::vector<std::string> StringsT;
typedef std::vector<Uint> SizesT;

/// Check close, for testing purposes
inline void
check_close(const Real a, const Real b, const Real threshold)
{
  BOOST_CHECK_CLOSE(a, b, threshold);
}

static boost::proto::terminal< void(*)(Real, Real, Real) >::type const _check_close = {&check_close};

struct ProtoHeatFixture
{
  ProtoHeatFixture() :
    root( Core::instance().root() )
  {
    solver_config = boost::unit_test::framework::master_test_suite().argv[1];
  }

  CRoot& root;
  std::string solver_config;

};

BOOST_FIXTURE_TEST_SUITE( ProtoHeatSuite, ProtoHeatFixture )

//////////////////////////////////////////////////////////////////////////////
/*
BOOST_AUTO_TEST_CASE( Laplacian1D )
{
  const Uint nb_segments = 5;

  CMesh& mesh = root.create_component<CMesh>("mesh");
  Tools::MeshGeneration::create_line(mesh, 5., nb_segments);

  // Linear system
  CEigenLSS& lss = root.create_component<CEigenLSS>("LSS");
  lss.set_config_file(solver_config);

  // Physical model
  CPhysicalModel& physical_model = root.create_component<CPhysicalModel>("PhysicalModel");

  MeshTerm<0, ScalarField> temperature("Temperature", "T");
  LSSProxy p(lss, physical_model);
  SystemMatrix M(p);

  Expression::Ptr assemble_matrix = elements_expression
  (
    group <<
    (
      _A(temperature) = integral<1>(transpose(nabla(temperature)) * nabla(temperature) * jacobian_determinant),
      M += _A
    )
  );

  std::vector<LSSProxy*> v;
  CollectLSSProxies()
  (
    group <<
    (
      _A(temperature) = integral<1>(transpose(nabla(temperature)) * nabla(temperature) * jacobian_determinant),
      M += _A
    ),
    v
  );

  BOOST_CHECK_EQUAL(v.size(), 1);
  BOOST_CHECK_EQUAL(v.front()->lss().name(), "LSS");
  
  
  assemble_matrix->register_variables(physical_model);
  physical_model.configure_option("mesh", mesh.uri());
  physical_model.create_fields();
  lss.resize(physical_model.nb_dof() * mesh.topology().nodes().size());
  
  elements_expression
  (
    _cout << "elem result:\n" << integral<1>(transpose(nabla(temperature)) * nabla(temperature) * jacobian_determinant) << "\n"
  )->loop(mesh.topology());

  assemble_matrix->loop(mesh.topology());
  lss.print_matrix();
}

BOOST_AUTO_TEST_CASE( Heat1D )
{
  Real length     =     5.;
  Real temp_start =   100.;
  Real temp_stop  =   500.;

  const Uint nb_segments = 20;

  CMesh& mesh = root.create_component<CMesh>("mesh");
  Tools::MeshGeneration::create_line(mesh, 5., nb_segments);

  // Linear system
  CEigenLSS& lss = root.create_component<CEigenLSS>("LSS");
  lss.set_config_file(solver_config);

  // Physical model
  CPhysicalModel& physical_model = root.create_component<CPhysicalModel>("PhysicalModel");

  MeshTerm<0, ScalarField> temperature("Temperature", "T");
  LSSProxy lss_proxy(lss, physical_model);
  SystemMatrix M(lss_proxy);
  DirichletBC dirichlet(lss_proxy);

  Expression::Ptr assemble_matrix = elements_expression
  (
    group <<
    (
      _A = _0,
      element_quadrature( _A(temperature[_i], temperature[_j]) += transpose(nabla(temperature)) * nabla(temperature) ),
      M += _A
    )
  );

  assemble_matrix->register_variables(physical_model);
  physical_model.configure_option("mesh", mesh.uri());
  physical_model.create_fields();
  lss.resize(physical_model.nb_dof() * mesh.topology().nodes().size());
  
  // Run the assembly
  assemble_matrix->loop(mesh.topology());
  
  // Left boundary at temp_start
  for_each_node
  (
    find_component_recursively_with_name<CRegion>(mesh, "xneg"),
    dirichlet(temperature) = temp_start
  );

  // Right boundary at temp_stop
  for_each_node
  (
    find_component_recursively_with_name<CRegion>(mesh, "xpos"),
    dirichlet(temperature) = temp_stop
  );

  // Solve the system!
  lss.solve();
  increment_solution(lss.solution(), StringsT(1, "Temperature"), StringsT(1, "T"), SizesT(1, 1), mesh);

  // Check solution
  for_each_node
  (
    mesh.topology(),
    _check_close(temperature, temp_start + (coordinates(0,0) / length) * (temp_stop - temp_start), 1e-6)
  );
}

// Heat conduction with Neumann BC
BOOST_AUTO_TEST_CASE( Heat1DNeumannBC )
{
  const Real length     =     5.;
  const Real temp_start =   100.;
  const Real temp_stop  =   500.;
  const Real k = 1.;
  const Real q = k * (temp_stop - temp_start) / length;

  const Uint nb_segments = 5;

  CMesh& mesh = root.create_component<CMesh>("mesh");
  Tools::MeshGeneration::create_line(mesh, 5., nb_segments);

  // Linear system
  CEigenLSS& lss = root.create_component<CEigenLSS>("LSS");
  lss.set_config_file(solver_config);

  // Physical model
  CPhysicalModel& physical_model = root.create_component<CPhysicalModel>("PhysicalModel");

  MeshTerm<0, ScalarField> temperature("Temperature", "T");
  LSSProxy lss_proxy(lss, physical_model);
  SystemMatrix M(lss_proxy);

  Expression::Ptr assemble_matrix = elements_expression
  (
    group <<
    (
      _A = _0,
      element_quadrature( _A(temperature[_i], temperature[_j]) += transpose(nabla(temperature)) * nabla(temperature) ),
      M += _A
    )
  );

  assemble_matrix->register_variables(physical_model);
  physical_model.configure_option("mesh", mesh.uri());
  physical_model.create_fields();
  lss.resize(physical_model.nb_dof() * mesh.topology().nodes().size());
  
  // Run the assembly
  assemble_matrix->loop(mesh.topology());

  
  NeumannBC neumann(lss_proxy);
  DirichletBC dirichlet(lss_proxy);
  
  // Right boundary at constant heat flux
  for_each_node
  (
    find_component_recursively_with_name<CRegion>(mesh, "xpos"),
    neumann(temperature) = q
  );

  // Left boundary at temp_start
  for_each_node
  (
    find_component_recursively_with_name<CRegion>(mesh, "xneg"),
    dirichlet(temperature) = temp_start
  );

  // Solve the system!
  lss.solve();
  increment_solution(lss.solution(), StringsT(1, "Temperature"), StringsT(1, "T"), SizesT(1, 1), mesh);

  // Check solution
  for_each_node
  (
    mesh.topology(),
    _check_close(temp_start + q * coordinates(0,0), temperature, 1e-6)
  );
}
*/
BOOST_AUTO_TEST_CASE( Heat1DComponent )
{
  // Parameters
  Real length            = 5.;
  const Uint nb_segments = 5 ;

  // Setup a UFEM model
  UFEM::Model& model = root.create_component<UFEM::SteadyModel>("Model");
  Tools::MeshGeneration::create_line(model.get_child("Mesh").as_type<CMesh>(), length, nb_segments);

  // Linear system setup (TODO: sane default config for this, so this can be skipped)
  CEigenLSS& lss = model.create_component<CEigenLSS>("LSS");
  lss.set_config_file(solver_config);
  model.problem().solve_action().configure_option("lss", lss.uri());

  // Proto placeholders
  MeshTerm<0, ScalarField> temperature("Temperature", "T");

  // shorthand for the problem and boundary conditions
  UFEM::LinearProblem& p = model.problem();
  UFEM::BoundaryConditions& bc = p.boundary_conditions();

  // add the top-level actions (assembly, BC and solve)
  model <<
  (
    p << p.add_action("Assembly", elements_expression
          (
            group <<
            (
              _A = _0,
              element_quadrature( _A(temperature[_i], temperature[_j]) += transpose(nabla(temperature)) * nabla(temperature) ),
              p.system_matrix += _A
            )
          ))
      << ( bc << bc.add_constant_bc("xneg", "Temperature", 10.) << bc.add_constant_bc("xpos", "Temperature", 35.) )
      << p.solve_action()
      << p.add_action("Increment", nodes_expression(temperature += p.solution(temperature)))
      << p.add_action("Output", nodes_expression(_cout << "T(" << coordinates(0,0) << ") = " << temperature << "\n"))
      << p.add_action("CheckResult", nodes_expression(_check_close(temperature, 10. + 25.*(coordinates(0,0) / length), 1e-6)))
  );
      
  // Run the solver
  model.execute();
}
/*
/// 1D heat conduction with a volume heat source
BOOST_AUTO_TEST_CASE( Heat1DVolumeTerm )
{
  Real length              = 5.;
  Real ambient_temp        = 293.;
  const Uint nb_segments   = 20;
  const Real k             = 100.; // thermal conductivity
  const Real q             = 100.; // Heat production per volume

  CMesh& mesh = root.create_component<CMesh>("mesh");
  Tools::MeshGeneration::create_line(mesh, 5., nb_segments);

  // Linear system
  CEigenLSS& lss = root.create_component<CEigenLSS>("LSS");
  lss.set_config_file(solver_config);

  // Physical model
  CPhysicalModel& physical_model = root.create_component<CPhysicalModel>("PhysicalModel");

  MeshTerm<0, ScalarField> temperature("Temperature", "T");
  MeshTerm<1, ScalarField> heat("heat", "q");

  LSSProxy lss_proxy(lss, physical_model);

  SystemMatrix M(lss_proxy);
  SystemRHS    b(lss_proxy);

  Expression::Ptr assemble_matrix = elements_expression
  (
    group <<
    (
      _A(temperature) = integral<1>( k * transpose(nabla(temperature)) * nabla(temperature) * jacobian_determinant ),
      _T(temperature) = integral<1>( jacobian_determinant * transpose(N(temperature))*N(temperature) ),
      M += _A,
      b += _T * nodal_values(heat)
    )
  );

  assemble_matrix->register_variables(physical_model);
  physical_model.configure_option("mesh", mesh.uri());
  physical_model.create_fields();
  lss.resize(physical_model.nb_dof() * mesh.topology().nodes().size());
  
  // Initialize volume term
  for_each_node(mesh.topology(), heat = q);
  
  // Run the assembly
  assemble_matrix->loop(mesh.topology());

  NeumannBC neumann(lss_proxy);
  DirichletBC dirichlet(lss_proxy);
  
  // Left boundary at ambient temperature
  for_each_node
  (
    find_component_recursively_with_name<CRegion>(mesh.topology(), "xneg"),
    dirichlet(temperature) = ambient_temp
  );

  // Right boundary at ambient temperature
  for_each_node
  (
    find_component_recursively_with_name<CRegion>(mesh.topology(), "xpos"),
    dirichlet(temperature) = ambient_temp
  );

  // Solve the system!
  lss.solve();
  increment_solution(lss.solution(), StringsT(1, "Temperature"), StringsT(1, "T"), SizesT(1, 1), mesh);

  // Check solution
  for(int i = 0; i != lss.solution().rows(); ++i)
  {
    Real x = i * length / static_cast<Real>(nb_segments);
    BOOST_CHECK_CLOSE
    (
      lss.solution()[i],
      -q/(2.*k)*x*x + q*length/(2.*k)*x + ambient_temp, // analytical solution, see "A Heat transfer textbook, section 2.2
      1e-6
    );
  }
  CFinfo << CFendl;
}
*/
////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
