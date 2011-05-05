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
#include "Mesh/Integrators/Gauss.hpp"
#include "Mesh/SF/Types.hpp"

#include "Solver/CEigenLSS.hpp"


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

/// Probe based on a coordinate value
void probe(const Real coord, const Real val, Real& result)
{
  if(coord > -0.1 && coord < 0.1)
    result = val;
}

static boost::proto::terminal< void(*)(Real, Real, Real&) >::type const _probe = {&probe};

BOOST_AUTO_TEST_SUITE( ProtoSystemSuite )

/// Operation to set the different coefficients for PSPG, SUPG and bulk viscosity
template<typename SF, Uint Dim, Uint Offset, Uint MatrixSize>
struct SetTau;

/// 2D specialization
template<typename SF, Uint Offset, Uint MatrixSize>
struct SetTau<SF, 2, Offset, MatrixSize>
{ 
  /// Dummy result
  typedef int result_type;
  
  template<typename SupportT, typename VarDataT, typename StateT>
  result_type operator()(const int, const SupportT& support, const VarDataT& u_data, StateT& state) const
  {
    const Real he=sqrt(4./3.141592654*support.volume());
    const Real ree=state.u_ref*he/(2.*state.nu);
    const Real xi=std::max(0.,std::min(ree/3.,1.));
    state.tau_ps = he*xi/(2.*state.u_ref);
    state.tau_bulk = he*state.u_ref/xi;
    
    // Average cell velocity
    const RealVector2 u = u_data.value().colwise().mean();
    const Real umag = u.norm();
    state.tau_su = 0.;
    if(umag > 1e-10)
    {
      const Real h = 2. * support.volume() / (support.nodes() * (u / umag)).array().abs().sum();
      Real ree=umag*h/(2.*state.nu);
      Real xi=std::max(0.,std::min(ree/3.,1.));
      state.tau_su = h*xi/(2.*umag);
    }
    
    // We need this one every time, so cache it
    state.jacobian_determinant = support.jacobian_determinant(Integrators::GaussMappedCoords<1, SF::shape>::instance().coords.col(0));
    
    return 0;
  }
};

// Placeholder for the SetTau operation
MakeSFOp<SetTau>::type set_tau = {};

/// Pressure contribution to the continuity equation (PSPG of pressure gradient) for matrix A
template<typename SF, Uint Dim, Uint Offset, Uint MatrixSize>
struct ContinuityPressureA
{
  BOOST_MPL_ASSERT_RELATION(Dim, ==, 1); // Pressure is a scalar
  
  typedef Eigen::Matrix<Real, SF::nb_nodes, SF::nb_nodes> MatrixT;
  typedef const MatrixT& result_type;
  
  template<typename SupportT, typename VarDataT, typename StateT>
  result_type operator()(MatrixT& matrix, const SupportT& support, const VarDataT& u_data, StateT& state) const
  {
    // Use first order integration
    typedef Integrators::GaussMappedCoords<1, SF::shape> GaussT;
    const typename SF::MappedGradientT& grad = u_data.gradient(GaussT::instance().coords.col(0), support);
    matrix = state.tau_ps * GaussT::instance().weights[0] * grad.transpose() * grad * state.jacobian_determinant;
    return matrix;
  };
};

MakeSFOp<ContinuityPressureA>::type continuity_p_a = {};

/// Velocity contribution to the continuity equation (divergence + PSPG of convective term) for matrix A
template<typename SF, Uint Dim, Uint Offset, Uint MatrixSize>
struct ContinuityVelocityA
{
  BOOST_MPL_ASSERT_RELATION(Dim, ==, SF::dimension);
  
  typedef Eigen::Matrix<Real, SF::nb_nodes, Dim*SF::nb_nodes> MatrixT;
  typedef const MatrixT& result_type;
  
  template<typename SupportT, typename VarDataT, typename StateT>
  result_type operator()(MatrixT& matrix, const SupportT& support, const VarDataT& u_data, StateT& state) const
  {
    // Use first order integration
    typedef Integrators::GaussMappedCoords<1, SF::shape> GaussT;
    const typename SF::ShapeFunctionsT& sf = u_data.shape_function(GaussT::instance().coords.col(0));
    const typename SF::MappedGradientT& gradient_matrix = u_data.gradient(GaussT::instance().coords.col(0), support);
    
    const typename SF::ShapeFunctionsT pspg_advection = state.tau_ps * sf * u_data.value() * gradient_matrix;
    for(Uint i = 0; i != SF::dimension; ++i)
      matrix.template block<SF::nb_nodes, SF::nb_nodes>(0, i*SF::nb_nodes).noalias() =
        sf.transpose() * gradient_matrix.row(i) // Divergence (standard)
      + gradient_matrix.row(i).transpose() * pspg_advection; // PSPG
    
    // Integration
    matrix *= GaussT::instance().weights[0] * state.jacobian_determinant;
    
    return matrix;
  };
};

MakeSFOp<ContinuityVelocityA>::type continuity_u_a = {};

/// Pressure contribution to the momentum equation (standard + SUPG) for matrix A
template<typename SF, Uint Dim, Uint Offset, Uint MatrixSize>
struct MomentumPressureA
{ 
  typedef Eigen::Matrix<Real, SF::dimension*SF::nb_nodes, SF::nb_nodes> MatrixT;
  typedef const MatrixT& result_type;
  
  // We pass data for u here, to be able to calculate the SUPG terms
  template<typename SupportT, typename VarDataT, typename StateT>
  result_type operator()(MatrixT& matrix, const SupportT& support, const VarDataT& u_data, StateT& state) const
  {
    // Use first order integration
    typedef Integrators::GaussMappedCoords<1, SF::shape> GaussT;
    const typename SF::ShapeFunctionsT& sf = u_data.shape_function(GaussT::instance().coords.col(0));
    const typename SF::MappedGradientT& gradient_matrix = u_data.gradient(GaussT::instance().coords.col(0), support);
    
    // Standard + SUPG
    const typename SF::ShapeFunctionsT supg_advection = state.tau_su * sf * u_data.value() * gradient_matrix;
    for(Uint i = 0; i != SF::dimension; ++i)
      matrix.template block<SF::nb_nodes, SF::nb_nodes>(i*SF::nb_nodes, 0).noalias() = (sf/ state.rho + supg_advection).transpose() * gradient_matrix.row(i);
    
    // Integration
    matrix *= GaussT::instance().weights[0] * state.jacobian_determinant;
    
    return matrix;
  };
};

MakeSFOp<MomentumPressureA>::type momentum_p_a = {};

/// Velocity contribution to the momentum equation (standard + SUPG + bulk viscosity) for matrix A
template<typename SF, Uint Dim, Uint Offset, Uint MatrixSize>
struct MomentumVelocityA
{ 
  typedef Eigen::Matrix<Real, SF::dimension*SF::nb_nodes, SF::dimension*SF::nb_nodes> MatrixT;
  typedef const MatrixT& result_type;
  
  // We pass data for u here, to be able to calculate the SUPG terms
  template<typename SupportT, typename VarDataT, typename StateT>
  result_type operator()(MatrixT& matrix, const SupportT& support, const VarDataT& u_data, StateT& state) const
  {
    // Use first order integration
    typedef Integrators::GaussMappedCoords<1, SF::shape> GaussT;
    const typename SF::ShapeFunctionsT& sf = u_data.shape_function(GaussT::instance().coords.col(0));
    const typename SF::MappedGradientT& gradient_matrix = u_data.gradient(GaussT::instance().coords.col(0), support);
    
    // Bulk viscosity
    Eigen::Matrix<Real, 1, SF::dimension*SF::nb_nodes> divergence_lin; // rowvector with all the components of the gradient
    for(Uint i = 0; i != SF::dimension; ++i)
      divergence_lin.template block<1, SF::nb_nodes>(0, i*SF::nb_nodes).noalias() = gradient_matrix.row(i);
    matrix = state.tau_bulk * divergence_lin.transpose() * divergence_lin;
    
    const typename SF::ShapeFunctionsT advection = sf * u_data.value() * gradient_matrix;
    const Eigen::Matrix<Real, SF::nb_nodes, SF::nb_nodes> diffusion =  state.nu*state.rho * gradient_matrix.transpose() * gradient_matrix;
    for(Uint i = 0; i != SF::dimension; ++i)
    {
      matrix.template block<SF::nb_nodes, SF::nb_nodes>(i*SF::nb_nodes, i*SF::nb_nodes) += diffusion
        + (sf.transpose() + state.tau_su * advection.transpose()) * advection; // Advection + SUPG
    }
    
    // Integration
    matrix *= GaussT::instance().weights[0] * state.jacobian_determinant;
    
    return matrix;
  };
};

MakeSFOp<MomentumVelocityA>::type momentum_u_a = {};

/// Continuity equation time contribution (PSPG)
template<typename SF, Uint Dim, Uint Offset, Uint MatrixSize>
struct ContinuityT
{ 
  typedef Eigen::Matrix<Real, SF::nb_nodes, SF::dimension*SF::nb_nodes> MatrixT;
  typedef const MatrixT& result_type;
  
  // We pass data for u here, to be able to calculate the SUPG terms
  template<typename SupportT, typename VarDataT, typename StateT>
  result_type operator()(MatrixT& matrix, const SupportT& support, const VarDataT& u_data, StateT& state) const
  {
    // Use first order integration
    typedef Integrators::GaussMappedCoords<1, SF::shape> GaussT;
    const typename SF::ShapeFunctionsT& sf = u_data.shape_function(GaussT::instance().coords.col(0));
    const typename SF::MappedGradientT& gradient_matrix = u_data.gradient(GaussT::instance().coords.col(0), support);
    
    const Real f = GaussT::instance().weights[0] * state.jacobian_determinant * state.tau_ps;
    for(Uint i = 0; i != SF::dimension; ++i)
    {
      matrix.template block<SF::nb_nodes, SF::nb_nodes>(0, i*SF::nb_nodes).noalias() = f * sf.transpose() * gradient_matrix.row(i);
    }
    
    return matrix;
  };
};

MakeSFOp<ContinuityT>::type continuity_t = {};

/// Momentum equation time contribution (Standard + SUPG)
template<typename SF, Uint Dim, Uint Offset, Uint MatrixSize>
struct MomentumT
{ 
  typedef Eigen::Matrix<Real, SF::dimension*SF::nb_nodes, SF::dimension*SF::nb_nodes> MatrixT;
  typedef const MatrixT& result_type;
  
  // We pass data for u here, to be able to calculate the SUPG terms
  template<typename SupportT, typename VarDataT, typename StateT>
  result_type operator()(MatrixT& matrix, const SupportT& support, const VarDataT& u_data, StateT& state) const
  {
    matrix.setZero();
    
    // Second order integration for the standard time
    typedef Integrators::GaussMappedCoords<2, SF::shape> GaussT2;
    for(Uint i = 0; i != GaussT2::nb_points; ++i)
    {
      const typename SF::ShapeFunctionsT& sf = u_data.shape_function(GaussT2::instance().coords.col(i));
      Eigen::Matrix<Real, SF::nb_nodes, SF::nb_nodes> m = GaussT2::instance().weights[i] * support.jacobian_determinant(GaussT2::instance().coords.col(i)) * sf.transpose() * sf;
      for(Uint d = 0; d != Dim; ++d)
        matrix.template block<SF::nb_nodes, SF::nb_nodes>(SF::nb_nodes*d, SF::nb_nodes*d) += m;
    }
    
    // Use first order integration for the rest
    typedef Integrators::GaussMappedCoords<1, SF::shape> GaussT;
    
    const typename SF::ShapeFunctionsT& sf = u_data.shape_function(GaussT::instance().coords.col(0));
    const typename SF::MappedGradientT& gradient_matrix = u_data.gradient(GaussT::instance().coords.col(0), support);
    
    // SUPG and integration
    const typename SF::ShapeFunctionsT supg_advection = GaussT::instance().weights[0] * state.jacobian_determinant * state.tau_su * sf * u_data.value() * gradient_matrix;
    for(Uint i = 0; i != SF::dimension; ++i)
    {
      matrix.template block<SF::nb_nodes, SF::nb_nodes>(i*SF::nb_nodes, i*SF::nb_nodes) += supg_advection.transpose() * sf;
    }
    
    return matrix;
  };
};

MakeSFOp<MomentumT>::type momentum_t = {};

/// Stores the coefficients for the SUPG model and shares them inside a proto expression through the state
struct SUPGState
{
  /// Reference velocity magnitude
  Real u_ref;
  
  /// Kinematic viscosity
  Real nu;
  
  /// Density
  Real rho;
  
  /// Model coefficients
  Real tau_ps, tau_su, tau_bulk;
  
  /// Jacobian determinant at the first gauss point
  Real jacobian_determinant;
};

// Solve the Navier-Stokes equations with SUPG and the bulk viscosity term
BOOST_AUTO_TEST_CASE( ProtoNavierStokesBULK )
{
  int    argc = boost::unit_test::framework::master_test_suite().argc;
  char** argv = boost::unit_test::framework::master_test_suite().argv;

  std::ofstream outfile("navier-stokes-bulk-stats.txt");
  outfile << "# Time(s) Velocity magnitude(m_s)" << std::endl;
  
  const Real start_time = 0.;
  const Real end_time = boost::lexical_cast<Real>(argv[1]);
  const Real dt = boost::lexical_cast<Real>(argv[2]);
  Real t = start_time;
  const Uint write_interval = 5;
  const Real invdt = 1. / dt;
  
  const Real mu = 0.01;
  const Real rho = 1.;
  
  const RealVector2 u_inf(16, 0.);
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
  for_each_node(mesh->topology(), u = u_wall);
  
  
  // Timings
  Real assemblytime, bctime, increment_time;
  
  while(t < end_time)
  {
    Timer timer;
    
    // Fill the system matrix
    lss.set_zero();
    
    for_each_element< boost::mpl::vector1<SF::Triag2DLagrangeP1> >
    (
      mesh->topology(),
      group(state) <<                             // Note we pass the state here, to calculate and share tau_...
      (
        set_tau(u),                               // Calculate the stabilization coefficients
        _A(p, p) = continuity_p_a(p),             // Continuity equation, p terms (PSPG)
        _A(p, u) = continuity_u_a(u),             // Continuity equation, u terms (Standard + PSPG)
        _A(u, p) = momentum_p_a(u),               // Momentum equation, p terms (Standard + SUPG)
        _A(u, u) = momentum_u_a(u),               // Momentum equation, u terms (Standard + SUPG + bulk viscosity)
        _T(p, u) = continuity_t(u),               // Time, PSPG
        _T(u, u) = momentum_t(u),                 // Time, standard and SUPG
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
    const StringsT fields = boost::assign::list_of("Velocity")("Pressure");
    const StringsT vars = boost::assign::list_of("u")("p");
    const SizesT dims = boost::assign::list_of(2)(1);
    
    timer.restart();
    increment_solution(lss.solution(), fields, vars, dims, *mesh);
    increment_time = timer.elapsed();
    
    const Real total_time = assemblytime + bctime + increment_time + lss.time_matrix_construction + lss.time_matrix_fill + lss.time_residual + lss.time_solve + lss.time_solver_setup;
    std::cout << "  assembly     : " << assemblytime << " (" << assemblytime/total_time*100. << "%)\n"
              << "  bc           : " << bctime << " (" << bctime/total_time*100. << "%)\n"
              << "  matrix build : " << lss.time_matrix_construction << " (" << lss.time_matrix_construction/total_time*100. << "%)\n"
              << "  matrix fill  : " << lss.time_matrix_fill << " (" << lss.time_matrix_fill/total_time*100. << "%)\n"
              << "  solver setup : " << lss.time_solver_setup << " (" << lss.time_solver_setup/total_time*100. << "%)\n"
              << "  solve        : " << lss.time_solve << " (" << lss.time_solve/total_time*100. << "%)\n"
              << "  residual     : " << lss.time_residual << " (" << lss.time_residual/total_time*100. << "%)\n"
              << "  write field  : " << increment_time << " (" << increment_time/total_time*100. << "%)\n"
              << "  total        : " << total_time << std::endl;
    
    t += dt;
    
    Real probeval = 0;
    for_each_node(out, _probe(coordinates[1], _sqrt(u[0]*u[0] + u[1]*u[1]), boost::proto::lit(probeval)));
    
    outfile << t << " " << probeval << std::endl;
    
    // Output solution
    if(t > 0. && (static_cast<Uint>(t / dt) % write_interval == 0 || t >= end_time))
    {
      std::stringstream outname;
      outname << "navier-stokes-bulk-";
      outname << std::setfill('0') << std::setw(5) << static_cast<Uint>(t / dt);
      boost::filesystem::path output_file(outname.str() + ".vtk");
      writer->write_from_to(mesh, output_file);
    }
  }
  
  
}

BOOST_AUTO_TEST_SUITE_END()
