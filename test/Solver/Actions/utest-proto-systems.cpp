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
#include "Common/CreateComponent.hpp"
#include "Common/CRoot.hpp"
#include "Common/Log.hpp"
#include "Common/LibLoader.hpp"
#include "Common/OSystem.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CField2.hpp"
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

// Disabled for rewrite of system handling
BOOST_AUTO_TEST_CASE( ProtoSystem )
{
  const Real length = 5.;
  const RealVector2 outside_temp(1., 1.);
  const RealVector2 initial_temp(100., 200.);
  const Uint nb_segments = 10;
  const Real start_time = 0.;
  const Real end_time = 0.5;
  const Real dt = 0.5;
  Real t = start_time;
  const Uint write_interval = 5;
  
  RealMatrix alpha;
  alpha.setIdentity(8,8);
  alpha.block(4, 4, 4, 4) *= 2.;
  
  const Real invdt = 1. / dt;

  // Load the required libraries (we assume the working dir is the binary path)
  LibLoader& loader = *OSystem::instance().lib_loader();
  
  const std::vector< boost::filesystem::path > lib_paths = boost::assign::list_of("../../../src/Mesh/Gmsh");
  loader.set_search_paths(lib_paths);
  
  loader.load_library("coolfluid_mesh_gmsh");
  
  // Setup document structure and mesh
  CRoot::Ptr root = Core::instance().root();
  
  CMesh::Ptr mesh = root->create_component<CMesh>("mesh");
  Tools::MeshGeneration::create_rectangle(*mesh, length, 0.5*length, 2*nb_segments, nb_segments);
  
  // Linear system
  CEigenLSS& lss = *root->create_component<CEigenLSS>("LSS");
  
  // Create output field
  CField2& t_fld = mesh->create_field2( "Temperature", CField2::Basis::POINT_BASED, std::vector<std::string>(1, "T"), std::vector<CField2::VarType>(1, CField2::VECTOR_2D) );
  lss.resize(t_fld.data().size() * 2);
  
  // Setup a mesh writer
  CMeshWriter::Ptr writer = create_component_abstract_type<CMeshWriter>("CF.Mesh.Gmsh.CWriter","meshwriter");
  root->add_component(writer);
  writer->configure_property( "Fields", std::vector<URI>(1, t_fld.full_path() ) );
  
  // Regions
  CRegion& left = find_component_recursively_with_name<CRegion>(*mesh, "left");
  CRegion& right = find_component_recursively_with_name<CRegion>(*mesh, "right");
  CRegion& bottom = find_component_recursively_with_name<CRegion>(*mesh, "bottom");
  CRegion& top = find_component_recursively_with_name<CRegion>(*mesh, "top");

  // Expression variables
  MeshTerm<0, VectorField> temperature("Temperature", "T");
  
  // Set initial condition.
  for_each_node
  (
    mesh->topology(),
    temperature = initial_temp
  );

  while(t < end_time)
  {
    // Fill the system matrix
    lss.set_zero();
    for_each_element< boost::mpl::vector1<SF::Quad2DLagrangeP1> >
    (
      mesh->topology(),
      group
      (
        _A(temperature) = boost::proto::lit(alpha) * integral<1>(laplacian_elm(temperature) * jacobian_determinant),
        _T(temperature) = invdt * integral<1>(value_elm(temperature) * jacobian_determinant),
        system_matrix(lss) += _T + 0.5 * _A,
        system_rhs(lss) -= _A * _b
      )
    );
    
    // All boundaries at outside temp
    for_each_node
    (
      left,
      dirichlet(lss, temperature) = outside_temp
    );
    
    for_each_node
    (
      right,
      dirichlet(lss, temperature) = outside_temp
    );
    
    for_each_node
    (
      top,
      dirichlet(lss, temperature) = outside_temp
    );
    
    for_each_node
    (
      bottom,
      dirichlet(lss, temperature) = outside_temp
    );
     
    // Solve the system!
    lss.solve();
    increment_solution(lss.solution(), StringsT(1, "Temperature"), StringsT(1, "T"), SizesT(1, 2), *mesh);
    
    t += dt;
        
    // Output using Gmsh
    if(t > 0. && (static_cast<Uint>(t / dt) % write_interval == 0 || t >= end_time))
    {
      boost::filesystem::path output_file("grid-vec.msh");
      writer->write_from_to(mesh, output_file);
    }
  }
}

BOOST_AUTO_TEST_SUITE_END()
