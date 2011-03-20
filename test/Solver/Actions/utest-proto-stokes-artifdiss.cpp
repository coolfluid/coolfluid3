// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for heat-conduction related proto operations"

#define BOOST_MPL_CFG_NO_PREPROCESSED_HEADERS
#define BOOST_MPL_LIMIT_METAFUNCTION_ARITY 6
#define BOOST_PROTO_MAX_ARITY 6

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

BOOST_AUTO_TEST_SUITE( ProtoSystemSuite )

// Solve the Stokes equations with artificial dissipation
BOOST_AUTO_TEST_CASE( ProtoStokesArtificialDissipation )
{
  const Real length = 5.;
  const Real height = 2.;
  const Uint x_segments = 25;
  const Uint y_segments = 10;
  
  const Real start_time = 0.;
  const Real end_time = 1000.;
  const Real dt = 10.;
  Real t = start_time;
  const Uint write_interval = 5000;
  const Real invdt = 1. / dt;
  
  const Real mu = 0.001;
  const Real rho = 100.;
  const Real epsilon = 100.;//rho/mu;
  
  const RealVector2 u_direction(1., 0.);
  const RealVector2 u_wall(0., 0.);
  const Real p0 = 10.;
  const Real p1 = 0.;
  const Real c = 0.5*(p1 - p0) / (rho * mu * length);
  
  // Load the required libraries (we assume the working dir is the binary path)
  LibLoader& loader = *OSystem::instance().lib_loader();
  
  const std::vector< boost::filesystem::path > lib_paths = boost::assign::list_of("../../../src/Mesh/Gmsh");
  loader.set_search_paths(lib_paths);
  
  loader.load_library("coolfluid_mesh_gmsh");
  
  // Setup document structure and mesh
  CRoot::Ptr root = Core::instance().root();
  
  CMesh::Ptr mesh = root->create_component<CMesh>("mesh");
  Tools::MeshGeneration::create_rectangle(*mesh, length, height, x_segments, y_segments);
  
  // Linear system
  CEigenLSS& lss = *root->create_component<CEigenLSS>("LSS");
  
  // Create output fields
  CField& u_fld = mesh->create_field2( "Velocity", CField::Basis::POINT_BASED, std::vector<std::string>(1, "u"), std::vector<CField::VarType>(1, CField::VECTOR_2D) );
  CField& p_fld = mesh->create_scalar_field("Pressure", "p", CF::Mesh::CField::Basis::POINT_BASED);
  
  lss.resize(u_fld.data().size() * 2 + p_fld.size());
  
  // Setup a mesh writer
  CMeshWriter::Ptr writer = create_component_abstract_type<CMeshWriter>("CF.Mesh.Gmsh.CWriter","meshwriter");
  root->add_component(writer);
  const std::vector<URI> out_fields = boost::assign::list_of(u_fld.full_path())(p_fld.full_path());
  writer->configure_property( "Fields", out_fields );
  
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
  physical_model.nb_dofs = 3;
  physical_model.variable_offsets["u"] = 0;
  physical_model.variable_offsets["p"] = 2;
  
  // Set initial conditions
  for_each_node(mesh->topology(), p = 0.);
  for_each_node(mesh->topology(), u = u_wall);//c * coordinates[1] * (coordinates[1] - height) * u_direction);
  
  while(t < end_time)
  {
    // Fill the system matrix
    lss.set_zero();
    for_each_element< boost::mpl::vector1<SF::Quad2DLagrangeP1> >
    (
      mesh->topology(),
      group
      (
        _A(p) = integral<1>(divergence_elm(u) * jacobian_determinant) + epsilon * integral<1>(laplacian_elm(p) * jacobian_determinant),
        _A(u) = mu * integral<1>(laplacian_elm(u) * jacobian_determinant) + 1/rho * integral<1>(gradient_elm(p) * jacobian_determinant), 
        _T(u) = invdt * integral<2>(value_elm(u) * jacobian_determinant),
        system_matrix(lss) += _T + 0.5 * _A,
        system_rhs(lss) -= _A * _b
      )
    );
    
    // Set boundary conditions
    for_each_node(left,   dirichlet(lss, p) = p0                                                           , physical_model);
    for_each_node(right,  dirichlet(lss, p) = p1                                                           , physical_model);
    for_each_node(left,   dirichlet(lss, u) = c * coordinates[1] * (coordinates[1] - height) * u_direction , physical_model);
    for_each_node(right,  dirichlet(lss, u) = c * coordinates[1] * (coordinates[1] - height) * u_direction , physical_model);
    for_each_node(top,    dirichlet(lss, u) = u_wall                                                       , physical_model);
    for_each_node(bottom, dirichlet(lss, u) = u_wall                                                       , physical_model);
    
    // Solve the system!
    lss.solve();
    const StringsT fields = boost::assign::list_of("Velocity")("Pressure");
    const StringsT vars = boost::assign::list_of("u")("p");
    const SizesT dims = boost::assign::list_of(2)(1);
    increment_solution(lss.solution(), fields, vars, dims, *mesh);

    t += dt;
        
    // Output using Gmsh
    if(t > 0. && (static_cast<Uint>(t / dt) % write_interval == 0 || t >= end_time))
    {
      boost::filesystem::path output_file("stokes-artifdiss.msh");
      writer->write_from_to(mesh, output_file);
    }
  }
}

BOOST_AUTO_TEST_SUITE_END()
