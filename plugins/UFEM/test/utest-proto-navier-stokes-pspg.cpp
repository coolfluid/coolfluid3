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

Real tau_pspg(const Real volume, const Real nu, const Real u_ref)
{
  Real he=sqrt(4./3.141592654*volume);
  Real ree=u_ref*he/(2.*nu);
  Real xi=std::max(0.,std::min(ree/3.,1.));
  return he*xi/(2.*u_ref);
}

static boost::proto::terminal< Real(*)(Real, Real, Real) >::type const _tau_pspg = {&tau_pspg};

/// Probe based on a coordinate value
void probe(const Real coord, const Real val, Real& result)
{
  if(coord > -0.1 && coord < 0.1)
    result = val;
}

static boost::proto::terminal< void(*)(Real, Real, Real&) >::type const _probe = {&probe};

BOOST_AUTO_TEST_SUITE( ProtoSystemSuite )

// Solve the Stokes equations with PSPG
BOOST_AUTO_TEST_CASE( ProtoStokesPSPG )
{
  int    argc = boost::unit_test::framework::master_test_suite().argc;
  char** argv = boost::unit_test::framework::master_test_suite().argv;

  std::ofstream outfile("navier-stokes-pspg-stats.txt");
  outfile << "# Time(s) Velocity magnitude(m_s)" << std::endl;
  
  const Real cell_size = 0.1;
  
  const Real start_time = 0.;
  const Real end_time = boost::lexical_cast<Real>(argv[1]);
  const Real dt = boost::lexical_cast<Real>(argv[2]);
  Real t = start_time;
  const Uint write_interval = 5;
  const Real invdt = 1. / dt;
  
  const Real mu = 0.01;
  const Real rho = 1.;
  
  const RealVector2 u_inf(0.6, 0.);
  const RealVector2 u_wall(0., 0.);
  const Real p_out = 0.;
  
  Real tau_ps_val;
  StoredReference<Real> tau_ps = store(tau_ps_val);
  
  // Load the required libraries (we assume the working dir is the binary path)
  LibLoader& loader = *OSystem::instance().lib_loader();
  
  const std::vector< boost::filesystem::path > lib_paths = boost::assign::list_of("../../../dso")("../../../src/Mesh/Tecplot");
  loader.set_search_paths(lib_paths);
  
  loader.load_library("coolfluid_mesh_tecplot");
  
  // Setup document structure and mesh
  CRoot::Ptr root = Core::instance().root();
  
  boost::filesystem::path input_file = boost::filesystem::path(argv[4]);
  
  CMeshReader::Ptr mesh_reader = create_component_abstract_type<CMeshReader>( "CF.Mesh.Neu.CReader", "NeutralReader" );
  root->add_component(mesh_reader);
  
  CMesh::Ptr mesh = root->create_component<CMesh>("mesh");
  mesh_reader->read_from_to(input_file, mesh);
  
  // Linear system
  CEigenLSS& lss = *root->create_component<CEigenLSS>("LSS");
  lss.set_config_file(argv[3]);
  
  // Create output fields
  CField& u_fld = mesh->create_field2( "Velocity", CField::Basis::POINT_BASED, std::vector<std::string>(1, "u"), std::vector<CField::VarType>(1, CField::VECTOR_2D) );
  CField& p_fld = mesh->create_scalar_field("Pressure", "p", CF::Mesh::CField::Basis::POINT_BASED);
  CField& u_mag_fld = mesh->create_scalar_field("VelMag", "u_mag", CF::Mesh::CField::Basis::POINT_BASED);
  CField& u_x_fld = mesh->create_scalar_field("XVel", "u_x", CF::Mesh::CField::Basis::POINT_BASED);
  CField& u_y_fld = mesh->create_scalar_field("YVel", "u_y", CF::Mesh::CField::Basis::POINT_BASED);
  
  lss.resize(u_fld.data().size() * 2 + p_fld.size());
  
  // Setup a mesh writer
  CMeshWriter::Ptr writer = create_component_abstract_type<CMeshWriter>("CF.Mesh.Tecplot.CWriter","meshwriter");
  root->add_component(writer);
  const std::vector<URI> out_fields = boost::assign::list_of(u_fld.full_path())(p_fld.full_path())(u_mag_fld.full_path())(u_x_fld.full_path())(u_y_fld.full_path());
  writer->configure_property( "Fields", out_fields );
  
  // Regions
  CRegion& in = find_component_recursively_with_name<CRegion>(*mesh, "in");
  CRegion& out = find_component_recursively_with_name<CRegion>(*mesh, "out");
  CRegion& symm = find_component_recursively_with_name<CRegion>(*mesh, "symm");
  CRegion& wall = find_component_recursively_with_name<CRegion>(*mesh, "wall");

  // Expression variables
  MeshTerm<0, VectorField> u("Velocity", "u");
  MeshTerm<1, ScalarField> p("Pressure", "p");
  MeshTerm<1, ScalarField> u_mag("VelMag", "u_mag");
  MeshTerm<1, ScalarField> u_x("XVel", "u_x");
  MeshTerm<1, ScalarField> u_y("YVel", "u_y");
  
  // Set up a physical model (normally handled automatically if using the Component wrappers)
  PhysicalModel physical_model;
  physical_model.nb_dofs = 3;
  physical_model.variable_offsets["u"] = 0;
  physical_model.variable_offsets["p"] = 2;
  
  // Set initial conditions
  for_each_node(mesh->topology(), p = 0.);
  for_each_node(mesh->topology(), u = u_wall);
  
  while(t < end_time)
  {
    // Fill the system matrix
    lss.set_zero();
    for_each_element< boost::mpl::vector1<SF::Triag2DLagrangeP1> >
    (
      mesh->topology(),
      group <<
      (
        boost::proto::lit(tau_ps) = _tau_pspg(volume, mu/rho, u_inf[0]),
        _A(p) = /*2.*/integral<1>((                                     // Mass equation
                  divergence_elm(u)                                          // standard
                + tau_ps * laplacian_elm(p)                                  // PSPG
                ) * jacobian_determinant),
        _A(p, u) += /*2.*/tau_ps * integral<1>( ( transpose(gradient(u)) * linearize(advection(u), u) ) * jacobian_determinant ), // PSPG for advective term
        _A(u) = integral<1>( (advection_elm(u) +  mu * laplacian_elm(u) + 1./rho * gradient_elm(p) ) * jacobian_determinant),     // Momentum
        _T(p) = tau_ps * integral<1>(divergence_elm(u) * jacobian_determinant),                                                   // Time, PSPG
        _T(u) = integral<2>(value_elm(u) * jacobian_determinant),                                                                 // Time, standard
        system_matrix(lss) += invdt * _T + 1.0 * _A,
        system_rhs(lss) -= _A * _b
      )
    );
    
    // Set boundary conditions
    for_each_node(in,   dirichlet(lss, u) = u_inf , physical_model);
    for_each_node(out,  dirichlet(lss, p) = p_out , physical_model);
    for_each_node(symm, dirichlet(lss, u) = u_inf , physical_model);
    for_each_node(wall, dirichlet(lss, u) = u_wall, physical_model);
    
    std::cout << "Solving for time " << t << std::endl;
    
    // Solve the system!
    lss.solve();
    const StringsT fields = boost::assign::list_of("Velocity")("Pressure");
    const StringsT vars = boost::assign::list_of("u")("p");
    const SizesT dims = boost::assign::list_of(2)(1);
    increment_solution(lss.solution(), fields, vars, dims, *mesh);

    t += dt;
    
    Real probeval = 0;
    for_each_node(out, _probe(coordinates[1], _sqrt(u[0]*u[0] + u[1]*u[1]), boost::proto::lit(probeval)));
    
    outfile << t << " " << probeval << std::endl;
    
    // Output solution
    if(t > 0. && (static_cast<Uint>(t / dt) % write_interval == 0 || t >= end_time))
    {
      for_each_node(mesh->topology(), u_mag = _sqrt(u[0]*u[0] + u[1]*u[1]));
      for_each_node(mesh->topology(), u_x = u[0]);
      for_each_node(mesh->topology(), u_y = u[1]);
      std::stringstream outname;
      outname << "navier-stokes-pspg-";
      outname << std::setfill('0') << std::setw(5) << static_cast<Uint>(t / dt);
      boost::filesystem::path output_file(outname.str() + ".tec");
      writer->write_from_to(mesh, output_file);
    }
  }
  
  
}

BOOST_AUTO_TEST_SUITE_END()
