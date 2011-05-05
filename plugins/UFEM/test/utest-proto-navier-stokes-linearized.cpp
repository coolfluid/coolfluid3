// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for heat-conduction related proto operations"

#include <iomanip>

#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <boost/test/unit_test.hpp>

#include "Solver/Actions/Proto/ElementLooper.hpp"
#include "Solver/Actions/Proto/Functions.hpp"
#include "Solver/Actions/Proto/NodeLooper.hpp"
#include "Solver/Actions/Proto/Terminals.hpp"

#include "Common/Core.hpp"
#include "Common/CreateComponent.hpp"
#include "Common/CRoot.hpp"
#include "Common/Log.hpp"
#include "Common/LibLoader.hpp"
#include "Common/OSystem.hpp"
#include "Common/Timer.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CMeshReader.hpp"
#include "Mesh/ElementData.hpp"

#include "Mesh/CMeshWriter.hpp"
#include "Mesh/SF/Types.hpp"

#include "Solver/CEigenLSS.hpp"

#include "UFEM/src/NavierStokesOps.hpp"

using namespace CF;
using namespace CF::Solver;
using namespace CF::Solver::Actions;
using namespace CF::Solver::Actions::Proto;
using namespace CF::Common;
using namespace CF::Math::MathConsts;
using namespace CF::Mesh;
using namespace CF::UFEM;

using namespace boost;

typedef std::vector<std::string> StringsT;
typedef std::vector<Uint> SizesT;

/// Probe based on a coordinate value
void probe(const Real coord, const Real val, Real& result)
{
  if(coord > -0.1 && coord < 0.1)
    result = val;
}

static boost::proto::terminal< void(*)(Real, Real, Real&) >::type const _probe = {&probe};

BOOST_AUTO_TEST_SUITE( ProtoSystemSuite )

// Solve the Navier-Stokes equations with SUPG and the bulk viscosity term
BOOST_AUTO_TEST_CASE( ProtoNavierStokesBULK )
{
  int    argc = boost::unit_test::framework::master_test_suite().argc;
  char** argv = boost::unit_test::framework::master_test_suite().argv;

  std::ofstream outfile("navier-stokes-lin-stats.txt");
  outfile << "# Time(s) Velocity magnitude(m_s)" << std::endl;
  
  const Real start_time = 0.;
  const Real end_time = boost::lexical_cast<Real>(argv[1]);
  const Real dt = boost::lexical_cast<Real>(argv[2]);
  Real t = start_time;
  const Uint write_interval = 5;
  const Real invdt = 1. / dt;
  
  const Real mu = 0.01;
  const Real rho = 1.;
  
  const RealVector2 u_inf(10., 0.);
  const RealVector2 u_wall(0., 0.);
  const Real p_out = 0.;
  
  SUPGState state;
  state.u_ref = u_inf[XX];
  state.nu = mu / rho;
  state.rho = rho;
  
  // Load the required libraries (we assume the working dir is the binary path)
  LibLoader& loader = *OSystem::instance().lib_loader();
  
  const std::vector< boost::filesystem::path > lib_paths = boost::assign::list_of("../../../dso")("../../../src/Mesh/VTKLegacy");
  loader.set_search_paths(lib_paths);
  
  loader.load_library("coolfluid_mesh_vtklegacy");
  
  // Setup document structure and mesh
  CRoot& root = Core::instance().root();
  
  boost::filesystem::path input_file = boost::filesystem::path(argv[4]);
  
  CMeshReader::Ptr mesh_reader = create_component_abstract_type<CMeshReader>( "CF.Mesh.Neu.CReader", "NeutralReader" );
  root.add_component(mesh_reader);
  
  CMesh::Ptr mesh = root.create_component_ptr<CMesh>("mesh");
  mesh_reader->read_from_to(input_file, mesh);
  
  // Linear system
  CEigenLSS& lss = *root.create_component_ptr<CEigenLSS>("LSS");
  lss.set_config_file(argv[3]);
  
  // Create output fields
  CField& u_fld = mesh->create_field( "Velocity", CField::Basis::POINT_BASED, std::vector<std::string>(1, "u"), std::vector<CField::VarType>(1, CField::VECTOR_2D) );
  CField& p_fld = mesh->create_scalar_field("Pressure", "p", CF::Mesh::CField::Basis::POINT_BASED);
  
  // Used in increment step
  const StringsT fields = boost::assign::list_of("Velocity")("Pressure");
  const StringsT vars = boost::assign::list_of("u")("p");
  const SizesT dims = boost::assign::list_of(2)(1);
  
  lss.resize(u_fld.data().size() * 2 + p_fld.size());
  
  // Setup a mesh writer
  CMeshWriter::Ptr writer = create_component_abstract_type<CMeshWriter>("CF.Mesh.VTKLegacy.CWriter","meshwriter");
  root.add_component(writer);
  const std::vector<URI> out_fields = boost::assign::list_of(u_fld.full_path())(p_fld.full_path());
  writer->configure_property( "Fields", out_fields );
  
  // Regions
  CRegion& in = find_component_recursively_with_name<CRegion>(*mesh, "in");
  CRegion& out = find_component_recursively_with_name<CRegion>(*mesh, "out");
  CRegion& symm = find_component_recursively_with_name<CRegion>(*mesh, "symm");
  CRegion& wall = find_component_recursively_with_name<CRegion>(*mesh, "wall");

  // Expression variables
  MeshTerm<0, VectorField> u("Velocity", "u");
  MeshTerm<1, ScalarField> p("Pressure", "p");
  
  // Set up a physical model (normally handled automatically if using the Component wrappers)
  PhysicalModel physical_model;
  physical_model.nb_dofs = 3;
  physical_model.variable_offsets["u"] = 0;
  physical_model.variable_offsets["p"] = 2;
  
  // Set initial conditions
  for_each_node(mesh->topology(), p = 0.);
  for_each_node(mesh->topology(), u = u_inf);
  
  // Timings
  Real assemblytime, bctime, increment_time, advect_time, update_advect_time;
  
  // Set up fields for velocity extrapolation
  const std::vector<std::string> advection_vars = boost::assign::list_of("u_adv")("u1")("u2")("u3");
  CField& u_adv_fld = mesh->create_field( "AdvectionVelocity", CField::Basis::POINT_BASED, advection_vars, std::vector<CField::VarType>(4, CField::VECTOR_2D) );
  
  // Variables associated with the advection velocity
  MeshTerm<2, VectorField> u_adv("AdvectionVelocity", "u_adv"); // The extrapolated advection velocity (n+1/2)
  MeshTerm<3, VectorField> u1("AdvectionVelocity", "u1");  // Two timesteps ago (n-1)
  MeshTerm<4, VectorField> u2("AdvectionVelocity", "u2"); // n-2
  MeshTerm<5, VectorField> u3("AdvectionVelocity", "u3"); // n-3
  
  // initialize
  for_each_node(mesh->topology(), u1 = u);
  for_each_node(mesh->topology(), u2 = u);
  for_each_node(mesh->topology(), u3 = u);
  
  while(t < end_time)
  {
    Timer timer;
    
    const Uint tstep = static_cast<Uint>(t / dt);
    
    // Extrapolate velocity
    for_each_node(mesh->topology(), u_adv = 2.1875*u - 2.1875*u1 + 1.3125*u2 - 0.3125*u3);
    
    advect_time = timer.elapsed(); timer.restart();
    
    // Fill the system matrix
    lss.set_zero();
    
    for_each_element< boost::mpl::vector1<SF::Triag2DLagrangeP1> >
    (
      mesh->topology(),
      group(state) <<                             // Note we pass the state here, to calculate and share tau_...
      (
        set_tau(u_adv),                               // Calculate the stabilization coefficients
        _A(p, p) = continuity_p_a(u_adv),             // Continuity equation, p terms (PSPG)
        _A(p, u) = continuity_u_a(u_adv),             // Continuity equation, u terms (Standard + PSPG)
        _A(u, p) = momentum_p_a(u_adv),               // Momentum equation, p terms (Standard + SUPG)
        _A(u, u) = momentum_u_a(u_adv),               // Momentum equation, u terms (Standard + SUPG + bulk viscosity)
        //_cout << "momentum, no skew:\n" << momentum_u_a(u_adv) << "\nmomentum skew:\n" << momentum_u_a_skew(u_adv) << "\ndiff:\n" << momentum_u_a(u_adv) - momentum_u_a_skew(u_adv) << "\n",
        _T(p, u) = continuity_t(u_adv),               // Time, PSPG
        _T(u, u) = momentum_t(u_adv),                 // Time, standard and SUPG
        system_matrix(lss) += invdt * _T + 1.0 * _A,
        system_rhs(lss) -= _A * _b
      )
    );
    
    assemblytime = timer.elapsed(); timer.restart();
    
    // Set boundary conditions
    for_each_node(in,   dirichlet(lss, u) = u_inf , physical_model);
    for_each_node(out,  dirichlet(lss, p) = p_out , physical_model);
    for_each_node(symm, dirichlet(lss, u) = u_inf , physical_model);
    for_each_node(wall, dirichlet(lss, u) = u_wall, physical_model);
    
    bctime = timer.elapsed(); 
    
    std::cout << "Solving for time " << t << std::endl;
    
    // Solve the system!
    lss.solve();
    
    timer.restart();
    
    // Save previous velocities for exrapolation
    for_each_node(mesh->topology(), u3 = u2);
    for_each_node(mesh->topology(), u2 = u1);
    for_each_node(mesh->topology(), u1 = u);
    update_advect_time = timer.elapsed(); timer.restart();
    
    increment_solution(lss.solution(), fields, vars, dims, *mesh);
    increment_time = timer.elapsed();
    
    const Real total_time = assemblytime + bctime + increment_time + lss.time_matrix_construction + lss.time_matrix_fill + lss.time_residual + lss.time_solve + lss.time_solver_setup + advect_time + update_advect_time;
    std::cout << "  assembly     : " << assemblytime << " (" << assemblytime/total_time*100. << "%)\n"
              << "  bc           : " << bctime << " (" << bctime/total_time*100. << "%)\n"
              << "  matrix build : " << lss.time_matrix_construction << " (" << lss.time_matrix_construction/total_time*100. << "%)\n"
              << "  matrix fill  : " << lss.time_matrix_fill << " (" << lss.time_matrix_fill/total_time*100. << "%)\n"
              << "  solver setup : " << lss.time_solver_setup << " (" << lss.time_solver_setup/total_time*100. << "%)\n"
              << "  solve        : " << lss.time_solve << " (" << lss.time_solve/total_time*100. << "%)\n"
              << "  residual     : " << lss.time_residual << " (" << lss.time_residual/total_time*100. << "%)\n"
              << "  write field  : " << increment_time << " (" << increment_time/total_time*100. << "%)\n"
              << "  extrapolate  : " << advect_time << " (" << advect_time/total_time*100. << "%)\n"
              << "  save tsteps  : " << update_advect_time << " (" << update_advect_time/total_time*100. << "%)\n"
              << "  total        : " << total_time << std::endl;
    
    t += dt;
    
    Real probeval = 0;
    for_each_node(out, _probe(coordinates[1], _sqrt(u[0]*u[0] + u[1]*u[1]), boost::proto::lit(probeval)));
    
    outfile << t << " " << probeval << std::endl;
    
    // Output solution
    if(t > 0. && (tstep % write_interval == 0 || t >= end_time))
    {
      std::stringstream outname;
      outname << "navier-stokes-lin-";
      outname << std::setfill('0') << std::setw(5) << static_cast<Uint>(t / dt);
      boost::filesystem::path output_file(outname.str() + ".vtk");
      writer->write_from_to(mesh, output_file);
    }
  }
  
  
}

BOOST_AUTO_TEST_SUITE_END()
