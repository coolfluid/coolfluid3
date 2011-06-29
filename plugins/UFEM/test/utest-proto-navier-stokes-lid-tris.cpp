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

BOOST_AUTO_TEST_SUITE( ProtoNSLidTrisSuite )

/// Construct element matrix for 2D P1 triangles
/// Code from CF2 by Tamás Bányai
struct NavierStokesSUPGTriag2DP1Op
{
  typedef void result_type;
  
  template<typename UT, typename MatrixT>
  void operator()(const UT& u, const SUPGCoeffs& tau, MatrixT& A, MatrixT& T)
  {
    const RealVector2 u_avg = u.value().colwise().mean();
    const typename UT::SF::NodeMatrixT& nodes = u.support().nodes();
    const Real volume = u.support().volume();
    const Real fc = 0.5;
    
    // Face normals
    typename UT::SF::NodeMatrixT normals;
    normals(0, XX) = nodes(1, YY) - nodes(2, YY);
    normals(0, YY) = nodes(2, XX) - nodes(1, XX);
    normals(1, XX) = nodes(2, YY) - nodes(0, YY);
    normals(1, YY) = nodes(0, XX) - nodes(2, XX);
    normals(2, XX) = nodes(0, YY) - nodes(1, YY);
    normals(2, YY) = nodes(1, XX) - nodes(0, XX);
    
    for(Uint i=0; i<3; ++i)
    {
      const Uint Pi = i + 6;
      const Uint Ui = i;
      const Uint Vi = i + 3;

      const Real u_ni = u_avg[XX]*normals(i, XX)+u_avg[YY]*normals(i, YY);

      for(Uint j=0; j<3; ++j)
      {
        const Uint Pj = j + 6;
        const Uint Uj = j;
        const Uint Vj = j + 3;
        
        const Real uk=u_avg[XX];
        const Real vk=u_avg[YY];
        const Real uknj=uk*normals(j, XX)+vk*normals(j, YY);
        
        // Convection (Standard + SUPG)
        Real val  = 1./6.*uknj;
        val += tau.su/(4.*volume)*uknj*u_ni;
        A(Ui,Uj) += val;
        A(Vi,Vj) += val;

        // Convection (PSPG)
        val = tau.ps/(4.*volume)*uknj;
        A(Pi,Uj) += val*normals(i, XX);
        A(Pi,Vj) += val*normals(i, YY);

        // Convection Skewsymm (Standard + SUPG)
        val  = fc/6.;
        val += fc*tau.su/(4.*volume)*u_ni;
        A(Ui,Uj) += val*uk*normals(j, XX);
        A(Ui,Vj) += val*uk*normals(j, YY);
        A(Vi,Uj) += val*vk*normals(j, XX);
        A(Vi,Vj) += val*vk*normals(j, YY);

        // Convection Skewsymm (PSPG)
        val = fc*tau.ps/(4.*volume);
        A(Pi,Uj) += val*normals(i, XX)*uk*normals(j, XX);
        A(Pi,Vj) += val*normals(i, XX)*uk*normals(j, YY);
        A(Pi,Uj) += val*normals(i, YY)*vk*normals(j, XX);
        A(Pi,Vj) += val*normals(i, YY)*vk*normals(j, YY);

        //difusion (Standard)
//         val = tau.nu/(4.*volume);
//         A(Ui,Uj)+=val*(4./3.*normals(i, XX)*normals(j, XX)+      normals(i, YY)*normals(j, YY));
//         A(Ui,Vj)+=val*       normals(i, YY)*normals(j, XX);
//         A(Vi,Uj)+=val*       normals(i, XX)*normals(j, YY);
//         A(Vi,Vj)+=val*(      normals(i, XX)*normals(j, XX)+4./3.*normals(i, YY)*normals(j, YY));
        const Real laplacian = 1./(4.*tau.rho*volume)*(normals(i, XX)*normals(j, XX)+normals(i, YY)*normals(j, YY));
        val = tau.nu*laplacian;
        A(Ui,Uj) += val;
        A(Vi,Vj) += val;
        
        // Pressure (Standard + SUPG)
        val  = 1./(6.*tau.rho);
        val += tau.su/(4.*tau.rho*volume)*u_ni;
        A(Ui,Pj) += normals(j, XX)*val;
        A(Vi,Pj) += normals(j, YY)*val;

        // Pressure (PSPG)
        A(Pi,Pj) += tau.ps*laplacian;

        // Continuity (Standard)
        val = 1./6.;
        A(Pi,Uj) += val*normals(j, XX);
        A(Pi,Vj) += val*normals(j, YY);

        // Bulk viscosity (Standard)
        val = tau.bulk/(4.*volume);
        A(Ui,Uj) += val*normals(i, XX)*normals(j, XX);
        A(Ui,Vj) += val*normals(i, XX)*normals(j, YY);
        A(Vi,Uj) += val*normals(i, YY)*normals(j, XX);
        A(Vi,Vj) += val*normals(i, YY)*normals(j, YY);

        // Time (Standard + SUPG)
        val  = volume/12.*(1.+((i)==(j)?1.:0.));
        val += tau.su/6.*u_ni;
        T(Ui,Uj) += val;
        T(Vi,Vj) += val;

        // Time (PSPG)
        val = tau.ps/6.;
        T(Pi,Uj) += val*normals(i, XX);
        T(Pi,Vj) += val*normals(i, YY);

        // Increasing diagonal dominance - crank nicholson related
        A(Pi,Pj) *= 2.;
        A(Pi,Uj) *= 2.;
        A(Pi,Vj) *= 2.;
      }
    }
  }
};

/// Placeholder for the triangle op
static MakeSFOp<NavierStokesSUPGTriag2DP1Op>::type const ns_triags = {};

Real max(const Real a, const Real b)
{
  return a > b ? a : b;
}

static boost::proto::terminal< Real(*)(const Real, const Real) >::type const _max = {&max};

// Solve the Navier-Stokes equations for lid driven cavity flow
BOOST_AUTO_TEST_CASE( ProtoNavierStokesLidTris )
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
    
    // Setup a mesh writer
    CMeshWriter::Ptr writer = build_component_abstract_type<CMeshWriter>("CF.Mesh.VTKLegacy.CWriter","meshwriter");
    root.add_component(writer);
    const std::vector<URI> out_fields = boost::assign::list_of(mesh->get_child("Velocity").uri())(mesh->get_child("Pressure").uri());
    writer->configure_option( "fields", out_fields );
    
    Timer total_timer;
    
    // Set initial conditions
    for_each_node(mesh->topology(), p = 0.);
    for_each_node(mesh->topology(), u = u_wall);
    
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
          _A(u) = _0, _A(p) = _0, _T(u) = _0, _T(p) = _0,
          compute_tau(u, tau),
          ns_triags(u_adv, tau, _A, _T),
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
    outname << "navier-stokes-lid-tris-";
    outname << x_segments << "x" << y_segments;
    URI output_file(outname.str() + ".vtk");
    writer->write_from_to(*mesh, output_file);
    
    std::stringstream profname;
    profname << "lid-center-tris-";
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
