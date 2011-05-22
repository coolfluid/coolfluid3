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

#include "Tools/MeshGeneration/MeshGeneration.hpp"

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

BOOST_AUTO_TEST_SUITE( ProtoSystemSuite )

Real max(const Real a, const Real b)
{
  return a > b ? a : b;
}

static boost::proto::terminal< Real(*)(const Real, const Real) >::type const _max = {&max};

// Solve the Navier-Stokes equations for lid driven cavity flow
BOOST_AUTO_TEST_CASE( ProtoNavierStokesLid )
{
  int    argc = boost::unit_test::framework::master_test_suite().argc;
  char** argv = boost::unit_test::framework::master_test_suite().argv;

  std::vector<Real> ux;
  std::vector<Real> uy;
  std::vector<Real> pressures;
  Real assemblytime = 0.;
  Real total_time = 0.;
  
  for(Uint segments = 8; segments != 128; segments *= 2)
  {
    std::cout << "Running for " << segments << " segments" << std::endl;
    const Real length = 1.;
    const Real height = 1.;
    const Uint x_segments = segments;
    const Uint y_segments = segments;
    
    const Real mu = 1.;
    const Real rho = 1.;
    
    const RealVector2 u_lid(100., 0.);
    const RealVector2 u_wall(0., 0.);
    
    SUPGCoeffs tau;
    tau.u_ref = u_lid[XX];
    tau.nu = mu / rho;
    tau.rho = rho;
    
    // Setup document structure and mesh
    CRoot& root = Core::instance().root();
    
    CMesh::Ptr mesh = root.create_component_ptr<CMesh>("mesh");
    Tools::MeshGeneration::create_rectangle_tris(*mesh, length, height, x_segments, y_segments);
    
    // Linear system
    CEigenLSS& lss = *root.create_component_ptr<CEigenLSS>("LSS");
    lss.set_config_file(argv[1]);

    // Regions
    CRegion& left = find_component_recursively_with_name<CRegion>(*mesh, "left");
    CRegion& right = find_component_recursively_with_name<CRegion>(*mesh, "right");
    CRegion& bottom = find_component_recursively_with_name<CRegion>(*mesh, "bottom");
    CRegion& top = find_component_recursively_with_name<CRegion>(*mesh, "top");
    CRegion& corner = find_component_recursively_with_name<CRegion>(*mesh, "corner");
    CRegion& center_line = find_component_recursively_with_name<CRegion>(*mesh, "center_line");
    CRegion& center_point = find_component_recursively_with_name<CRegion>(*mesh, "center_point");

    // Expression variables
    MeshTerm<0, VectorField> u("Velocity", "u");
    MeshTerm<1, ScalarField> p("Pressure", "p");
  
    // Set up a physical model (normally handled automatically if using the Component wrappers)
    PhysicalModel physical_model;
    physical_model.register_variable(u, true);
    physical_model.register_variable(p, true);
    physical_model.create_fields(*mesh);
    lss.resize(physical_model.nb_dofs() * mesh->nodes().size());
    
    // Set up fields for velocity extrapolation
    const std::vector<std::string> advection_vars = boost::assign::list_of("u_adv")("u1")("u2")("u3");
    CField& u_adv_fld = mesh->create_field( "AdvectionVelocity", CField::Basis::POINT_BASED, advection_vars, std::vector<CField::VarType>(4, CField::VECTOR_2D) );
    
    // Variables associated with the advection velocity
    MeshTerm<2, VectorField> u_adv("AdvectionVelocity", "u_adv"); // The extrapolated advection velocity (n+1/2)
    MeshTerm<3, VectorField> u1("AdvectionVelocity", "u1");  // Two timesteps ago (n-1)
    MeshTerm<4, VectorField> u2("AdvectionVelocity", "u2"); // n-2
    MeshTerm<5, VectorField> u3("AdvectionVelocity", "u3"); // n-3
    
    // Setup a mesh writer
    CMeshWriter::Ptr writer = create_component_abstract_type<CMeshWriter>("CF.Mesh.VTKLegacy.CWriter","meshwriter");
    root.add_component(writer);
    const std::vector<URI> out_fields = boost::assign::list_of(mesh->get_child("Velocity").full_path())(mesh->get_child("Pressure").full_path());
    writer->configure_property( "fields", out_fields );
    
    Timer total_timer;
    
    // Set initial conditions
    for_each_node(mesh->topology(), p = 0.);
    for_each_node(mesh->topology(), u = u_wall);

    // initialize
    for_each_node(mesh->topology(), u1 = u);
    for_each_node(mesh->topology(), u2 = u);
    for_each_node(mesh->topology(), u3 = u);
    
    for(Uint iter = 0; iter != 1000; ++iter)
    {
      // Extrapolate velocity
      for_each_node(mesh->topology(), u_adv = 2.1875*u - 2.1875*u1 + 1.3125*u2 - 0.3125*u3);
      
      // Fill the system matrix
      lss.set_zero();
      
      Timer timer;
      
      for_each_element< boost::mpl::vector1<SF::Triag2DLagrangeP1> >
      (
        mesh->topology(),
        group <<                             // Note we pass the state here, to calculate and share tau_...
        (
          _A = _0, _T = _0,
          compute_tau(u, tau),
          element_quadrature <<
          (
            _A(p    , u[_i]) +=          transpose(N(p))         * nabla(u)[_i] + tau.ps * transpose(nabla(p)[_i]) * u_adv*nabla(u), // Standard continuity + PSPG for advection
            _A(p    , p)     += tau.ps * transpose(nabla(p))     * nabla(p),     // Continuity, PSPG
            _A(u[_i], u[_i]) += tau.nu * transpose(nabla(u))     * nabla(u)     + transpose(N(u) + tau.su*u_adv*nabla(u)) * u_adv*nabla(u),     // Diffusion + advection
            _A(u[_i], p)     += 1./rho * transpose(N(u) + tau.su*u_adv*nabla(u)) * nabla(p)[_i], // Pressure gradient (standard and SUPG)
            _A(u[_i], u[_j]) += tau.bulk * transpose(nabla(u)[_i]) * nabla(u)[_j], // Bulk viscosity
            _T(p    , u[_i]) += tau.ps * transpose(nabla(p)[_i]) * N(u),         // Time, PSPG
            _T(u[_i], u[_i]) += transpose(N(u) + tau.su*u_adv*nabla(u))         * N(u)          // Time, standard
          ),
          system_matrix(lss) += 100. * _T + 1.0 * _A,
          system_rhs(lss) -= _A * _b
        )
      );
      
      assemblytime += timer.elapsed(); timer.restart();
      
      // Set boundary conditions
      for_each_node(left,   dirichlet(lss, u) = u_wall, physical_model);
      for_each_node(right,  dirichlet(lss, u) = u_wall, physical_model);
      for_each_node(top,    dirichlet(lss, u) = u_lid,  physical_model);
      for_each_node(bottom, dirichlet(lss, u) = u_wall, physical_model);
      for_each_node(corner, dirichlet(lss, p) = 0., physical_model);
      
      // Solve the system!
      lss.solve();
      
      // Save previous velocities for exrapolation
      for_each_node(mesh->topology(), u3 = u2);
      for_each_node(mesh->topology(), u2 = u1);
      for_each_node(mesh->topology(), u1 = u);
      
      physical_model.update_fields(*mesh, lss.solution());
      
      // Get the maximum difference between two subsequent solutions
      Real maxdiff = 0.;
      for_each_node(mesh->topology(), boost::proto::lit(maxdiff) = _max(maxdiff, (transpose(u - u1) * (u - u1))[0]));
      
      if(maxdiff < 1e-12)
      {
        std::cout << "Convergence after " << iter << " iterations" << std::endl;
        break;
      }
    }
    
    total_time += total_timer.elapsed();
    
    // Output solution
    std::stringstream outname;
    outname << "navier-stokes-lid-tris-gen-";
    outname << x_segments << "x" << y_segments;
    URI output_file(outname.str() + ".vtk");
    writer->write_from_to(*mesh, output_file);
    
    std::stringstream profname;
    profname << "lid-center-tris-gen-";
    profname << x_segments << "x" << y_segments << ".txt";
    std::ofstream profile(profname.str().c_str());
    for_each_node(center_line, boost::proto::lit(profile) << coordinates[0] << " " << coordinates[1] << " " << u[0] << " " << u[1] << " " << p << "\n");
    
    ux.push_back(0.);
    uy.push_back(0.);
    pressures.push_back(0.);
    for_each_node(center_point, _cout << "sampling at point " << coordinates << "\n");
    for_each_node(center_point,boost::proto::lit(ux.back()) = u[0]);
    for_each_node(center_point,boost::proto::lit(uy.back()) = u[1]);
    for_each_node(center_point,boost::proto::lit(pressures.back()) = p);
  }
  std::cout << "Assembly took " << assemblytime/total_time*100. << "% out of a total of " << total_time << "s" << std::endl;
  std::cout << "Error evolution:" << std::endl;
  for(Uint i = 0; i != ux.size()-1; ++i)
  {
    std::cout << "ux: " << std::abs(ux[i+1] - ux[i]) << ", uy: " << std::abs(uy[i+1] - uy[i]) << ", p: " << std::abs(pressures[i+1] - pressures[i]) << std::endl;
  }
}

BOOST_AUTO_TEST_SUITE_END()
