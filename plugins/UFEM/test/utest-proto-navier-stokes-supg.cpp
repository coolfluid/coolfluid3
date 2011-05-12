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

#include "Tools/MeshGeneration/MeshGeneration.hpp"


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

typedef Eigen::Matrix<Real, 12, 12> ElementMatrixT;

/// Check close, for testing purposes
inline void check_close(const ElementMatrixT& a, const ElementMatrixT& b, const Real threshold)
{
  if(std::abs(a.norm() - b.norm()) > 1e-10)
    std::cout << "Matrix differs:\n" << a-b << "\n------------\nReference:\n" << a << "\n------------\nCalculated:\n" << b << std::endl;
}

static boost::proto::terminal< void(*)(const ElementMatrixT&, const ElementMatrixT&, Real) >::type const _check_close = {&check_close};

inline void 
check_close2(const Real a, const Real b, const Real threshold)
{
  BOOST_CHECK_SMALL(a - b, threshold);
}

static boost::proto::terminal< void(*)(Real, Real, Real) >::type const _check_close2 = {&check_close2};

BOOST_AUTO_TEST_SUITE( ProtoSUPGSuite )

/// Stores the coefficients for the SUPG model and shares them inside a proto expression through the state
struct SUPGCoeffs
{ 
  /// Reference velocity magnitude
  Real u_ref;
  
  /// Kinematic viscosity
  Real nu;
  
  /// Density
  Real rho;
  
  /// Model coefficients
  Real ps, su, bulk;
};

struct ComputeTau
{ 
  /// Dummy result
  typedef void result_type;
  
  template<typename UT>
  void operator()(const UT& u, SUPGCoeffs& coeffs) const
  {
    const Real he=sqrt(4./3.141592654*u.support().volume());
    const Real ree=coeffs.u_ref*he/(2.*coeffs.nu);
    const Real xi=std::max(0.,std::min(ree/3.,1.));
    coeffs.ps = he*xi/(2.*coeffs.u_ref);
    coeffs.bulk = he*coeffs.u_ref/xi;
    
    // Average cell velocity
    const RealVector2 u_avg = u.value().colwise().mean();
    const Real umag = u_avg.norm();
    coeffs.su = 0.;
    if(umag > 1e-10)
    {
      const Real h = 2. * u.support().volume() / (u.support().nodes() * (u_avg / umag)).array().abs().sum();
      Real ree=umag*h/(2.*coeffs.nu);
      Real xi=std::max(0.,std::min(ree/3.,1.));
      coeffs.su = h*xi/(2.*umag);
    }
  }
};

/// Placeholder for the compute_tau operation
static MakeSFOp<ComputeTau>::type const compute_tau = {};

// Solve the Navier-Stokes equations with SUPG
BOOST_AUTO_TEST_CASE( ProtoNavierStokesSUPG )
{
  int    argc = boost::unit_test::framework::master_test_suite().argc;
  char** argv = boost::unit_test::framework::master_test_suite().argv;

  const Real length = 5.;
  const Real height = 2.;
  const Uint x_segments = 25;
  const Uint y_segments = 10;
  
  const Real start_time = 0.;
  const Real end_time = 50.;
  const Real dt = 5.;
  Real t = start_time;
  const Uint write_interval = 200;
  const Real invdt = 1. / dt;
  
  const Real mu = 0.1;
  const Real rho = 100.;
  
  const RealVector2 u_direction(1., 0.);
  const RealVector2 u_wall(0., 0.);
  const Real p0 = 5.;
  const Real p1 = 0.;
  const Real c = 0.5 * (p0 - p1) / (rho * mu * length);
  
  SUPGCoeffs tau;
  tau.u_ref = c;
  tau.nu = mu / rho;
  tau.rho = rho;
  
  // Mapped coordinates for the centriod
  const RealVector2 centroid(0., 0.);
  
  // Setup document structure and mesh
  CRoot& root = Core::instance().root();
  
  CMesh::Ptr mesh = root.create_component_ptr<CMesh>("mesh");
  Tools::MeshGeneration::create_rectangle(*mesh, length, height, x_segments, y_segments);
  
  // Linear system
  CEigenLSS& lss = *root.create_component_ptr<CEigenLSS>("LSS");
  lss.set_config_file(argv[1]);
  
  // Regions
  CRegion& left = find_component_recursively_with_name<CRegion>(*mesh, "left");
  CRegion& right = find_component_recursively_with_name<CRegion>(*mesh, "right");
  CRegion& bottom = find_component_recursively_with_name<CRegion>(*mesh, "bottom");
  CRegion& top = find_component_recursively_with_name<CRegion>(*mesh, "top");

  // Expression variables
  MeshTerm<0, VectorField> u("Velocity", "u");
  MeshTerm<1, ScalarField> p("Pressure", "p");
  
  // Set up a physical model (normally handled automatically if using the Component wrappers)
  PhysicalModel physical_model;
  physical_model.register_variable(u, true);
  physical_model.register_variable(p, true);
  physical_model.create_fields(*mesh);
  lss.resize(physical_model.nb_dofs() * mesh->nodes().size());
  
  // Setup a mesh writer
  CMeshWriter::Ptr writer = create_component_abstract_type<CMeshWriter>("CF.Mesh.Gmsh.CWriter","meshwriter");
  root.add_component(writer);
  const std::vector<URI> out_fields = boost::assign::list_of(mesh->get_child("Velocity").full_path())(mesh->get_child("Pressure").full_path());
  writer->configure_property( "Fields", out_fields );
  
  // Set initial conditions
  for_each_node(mesh->topology(), p = 0.);
  for_each_node(mesh->topology(), u = c * coordinates[1] * (height - coordinates[1]) * u_direction);
  
  while(t < end_time)
  {
    // Fill the system matrix
    lss.set_zero();
    
    for_each_element< boost::mpl::vector1<SF::Quad2DLagrangeP1> >
    (
      mesh->topology(),
      group <<                             // Note we pass the state here, to calculate and share tau_...
      (
        _A = _0, _T = _0,
        compute_tau(u, tau),
        element_quadrature <<
        (
          _A(p    , u[_i]) +=          transpose(N(p))         * nabla(u)[_i] + tau.ps * transpose(nabla(p)[_i]) * advection(u), // Standard continuity + PSPG for advection
          _A(p    , p)     += tau.ps * transpose(nabla(p))     * nabla(p),     // Continuity, PSPG
          _A(u[_i], u[_i]) += mu     * transpose(nabla(u))     * nabla(u)     + transpose(N(u) + tau.su*advection(u)) * advection(u),     // Diffusion + advection
          _A(u[_i], p)     += 1./rho * transpose(N(u) + tau.su*advection(u)) * nabla(p)[_i], // Pressure gradient (standard and SUPG)
          _T(p    , u[_i]) += tau.ps * transpose(nabla(p)[_i]) * N(u),         // Time, PSPG
          _T(u[_i], u[_i]) += invdt  * transpose(N(u) + tau.su*advection(u))         * N(u)          // Time, standard
        ),
        system_matrix(lss) += invdt * _T + 1.0 * _A,
        system_rhs(lss) -= _A * _b
      )
    );
    
    // Set boundary conditions
    for_each_node(left,   dirichlet(lss, p) = p0                                                           , physical_model);
    for_each_node(right,  dirichlet(lss, p) = p1                                                           , physical_model);
    for_each_node(left,   dirichlet(lss, u) = c * coordinates[1] * (height - coordinates[1]) * u_direction , physical_model);
    for_each_node(right,  dirichlet(lss, u) = c * coordinates[1] * (height - coordinates[1]) * u_direction , physical_model);
    for_each_node(top,    dirichlet(lss, u) = u_wall                                                       , physical_model);
    for_each_node(bottom, dirichlet(lss, u) = u_wall                                                       , physical_model);
    
    std::cout << "Solving for time " << t << std::endl;
    
    // Solve the system!
    lss.solve();
    const StringsT fields = boost::assign::list_of("Velocity")("Pressure");
    const StringsT vars = boost::assign::list_of("u")("p");
    const SizesT dims = boost::assign::list_of(2)(1);
    increment_solution(lss.solution(), fields, vars, dims, *mesh);
    
    t += dt;
    
    // Output solution
    if(t > 0. && (static_cast<Uint>(t / dt) % write_interval == 0 || t >= end_time))
    {
      std::stringstream outname;
      outname << "navier-stokes-supg-";
      outname << std::setfill('0') << std::setw(5) << static_cast<Uint>(t / dt);
      boost::filesystem::path output_file(outname.str() + ".vtk");
      writer->write_from_to(mesh, output_file);
    }
  }
  
  // Check analytical solution
  for_each_node(mesh->topology(), _check_close2(p, p0 * (length - coordinates[0]) / length + p1 * coordinates[1] / length, 0.1));
  for_each_node(mesh->topology(), _check_close2(u[0], c * coordinates[1] * (height - coordinates[1]), 1e-2));
  for_each_node(mesh->topology(), _check_close2(u[1], 0., 1e-2));
  
}

BOOST_AUTO_TEST_SUITE_END()
