// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for heat-conduction related proto operations"

#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <boost/test/unit_test.hpp>

#include "Solver/Actions/Proto/ElementLooper.hpp"
#include "Solver/Actions/Proto/Functions.hpp"
#include "Solver/Actions/Proto/NodeLooper.hpp"
#include "Solver/Actions/Proto/Terminals.hpp"

#include "Common/Core.hpp"
 
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

BOOST_AUTO_TEST_SUITE( ProtoStokesPSPGSuite )

/// Check close, for testing purposes
inline void 
check_close(const Real a, const Real b, const Real threshold)
{
  BOOST_CHECK_SMALL(a - b, threshold);
}

static boost::proto::terminal< void(*)(Real, Real, Real) >::type const _check_close = {&check_close};

struct ComputeTau
{ 
  /// Dummy result
  typedef void result_type;
  
  template<typename UT>
  void operator()(const UT& u, const Real u_ref, const Real nu, Real& tau_ps) const
  {
    const Real he=sqrt(4./3.141592654*u.support().volume());
    const Real ree=u_ref*he/(2.*nu);
    const Real xi=std::max(0.,std::min(ree/3.,1.));
    tau_ps = he*xi/(2.*u_ref);
  }
};

/// Placeholder for the compute_tau operation
static MakeSFOp<ComputeTau>::type const compute_tau = {};

// Solve the Stokes equations with PSPG
BOOST_AUTO_TEST_CASE( ProtoStokesPSPG )
{
  const Real length = 5.;
  const Real height = 2.;
  const Uint x_segments = 25;
  const Uint y_segments = 10;
  
  const Real start_time = 0.;
  const Real end_time = 10;
  const Real dt = 0.5;
  Real t = start_time;
  const Uint write_interval = 200;
  const Real invdt = 1. / dt;
  
  const Real mu = 0.1;
  const Real rho = 100.;
  const Real nu = mu / rho;
  
  const RealVector2 u_direction(1., 0.);
  const RealVector2 u_wall(0., 0.);
  const Real p0 = 5.;
  const Real p1 = 0.;
  const Real c = 0.5 * (p0 - p1) / (rho * mu * length);
  
  // c = umax if height = 2
//   const Real he = height / static_cast<Real>(y_segments);
//   const Real ree = 0.5 * he * c * rho / mu;
//   const Real xi = std::max(0.,std::min(ree/3.,1.));
//   const Real tau_ps = he*xi/(2.*c);
//   
//   std::cout << "tau_ps = " << tau_ps << std::endl;
  
  boost::proto::literal<Real> tau_ps;
  
  // Load the required libraries (we assume the working dir is the binary path)
  LibLoader& loader = *OSystem::instance().lib_loader();
  
  const std::vector< boost::filesystem::path > lib_paths = boost::assign::list_of("../../../src/Mesh/Gmsh");
  loader.set_search_paths(lib_paths);
  
  loader.load_library("coolfluid_mesh_gmsh");
  
  // Setup document structure and mesh
  CRoot& root = Core::instance().root();
  
  CMesh::Ptr mesh = root.create_component_ptr<CMesh>("mesh");
  Tools::MeshGeneration::create_rectangle(*mesh, length, height, x_segments, y_segments);
  
  // Linear system
  CEigenLSS& lss = *root.create_component_ptr<CEigenLSS>("LSS");
  lss.set_config_file(boost::unit_test::framework::master_test_suite().argv[1]);
  
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
  CMeshWriter::Ptr writer = build_component_abstract_type<CMeshWriter>("CF.Mesh.Gmsh.CWriter","meshwriter");
  root.add_component(writer);
  const std::vector<URI> out_fields = boost::assign::list_of(mesh->get_child("Velocity").uri())(mesh->get_child("Pressure").uri());
  writer->configure_option( "fields", out_fields );
  
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
      group <<
      (
        _A = _0, _T = _0,
        compute_tau(u, c, nu, tau_ps),
        element_quadrature <<
        (
          _A(p    , u[_i]) +=          transpose(N(p))         * nabla(u)[_i], // Continuity, standard
          _A(p    , p)     += tau_ps * transpose(nabla(p))     * nabla(p),     // Continuity, PSPG
          _A(u[_i], u[_i]) += mu     * transpose(nabla(u))     * nabla(u),     // Diffusion
          _A(u[_i], p)     += 1./rho * transpose(N(u))         * nabla(p)[_i], // Pressure gradient
          _T(p    , u[_i]) += tau_ps * transpose(nabla(p)[_i]) * N(u),         // Time, PSPG
          _T(u[_i], u[_i]) += transpose(N(u))         * N(u)          // Time, standard
        ),
        system_matrix(lss) += invdt * _T + 0.5 * _A,
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
    
    // Solve the system!
    lss.solve();
    const StringsT fields = boost::assign::list_of("Velocity")("Pressure");
    const StringsT vars = boost::assign::list_of("u")("p");
    const SizesT dims = boost::assign::list_of(2)(1);
    physical_model.update_fields(*mesh, lss.solution());

    t += dt;
        
    // Output using Gmsh
    if(t > 0. && (static_cast<Uint>(t / dt) % write_interval == 0 || t >= end_time))
    {
      URI output_file("stokes-pspg-" + boost::lexical_cast<std::string>(static_cast<Uint>(t / dt)) + ".msh");
      writer->write_from_to(*mesh, output_file);
    }
  }
  // Check analytical solution
  for_each_node(mesh->topology(), _check_close(p, p0 * (length - coordinates[0]) / length + p1 * coordinates[1] / length, 1e-3));
  for_each_node(mesh->topology(), _check_close(u[0], c * coordinates[1] * (height - coordinates[1]), 1e-3));
  for_each_node(mesh->topology(), _check_close(u[1], 0., 1e-3));
}

BOOST_AUTO_TEST_SUITE_END()
