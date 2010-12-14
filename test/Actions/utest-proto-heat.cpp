// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for heat-conduction related proto operations"

#include <boost/foreach.hpp>
#include <boost/test/unit_test.hpp>

#include "Actions/Proto/CProtoElementsAction.hpp"
#include "Actions/Proto/CProtoNodesAction.hpp"
#include "Actions/Proto/ElementLooper.hpp"
#include "Actions/Proto/NodeLooper.hpp"
#include "Actions/Proto/Terminals.hpp"

#include "Common/CRoot.hpp"
#include "Common/Log.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CFieldElements.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CMeshReader.hpp"
#include "Mesh/ElementData.hpp"

#include "Mesh/Integrators/Gauss.hpp"
#include "Mesh/SF/Types.hpp"

#include "Solver/CPhysicalModel.hpp"
#include "Solver/CEigenLSS.hpp"

#include "Tools/MeshGeneration/MeshGeneration.hpp"

using namespace CF;
using namespace CF::Actions;
using namespace CF::Actions::Proto;
using namespace CF::Mesh;
using namespace CF::Common;
using namespace CF::Solver;

using namespace boost;

BOOST_AUTO_TEST_SUITE( ProtoHeatSuite )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Laplacian1D )
{
  const Uint nb_segments = 5;
  
  CMesh::Ptr mesh(allocate_component<CMesh>("line"));
  Tools::MeshGeneration::create_line(*mesh, 5., nb_segments);
  
  // Geometric suport
  MeshTerm<0, ConstNodes> nodes( "ConductivityRegion", find_component_ptr_recursively_with_name<CRegion>(*mesh, "region") );
  
  for_each_element< boost::mpl::vector<SF::Line1DLagrangeP1> >
  (
    _cout << "elem result:\n" << integral<1>( laplacian(nodes, nodes) * jacobian_determinant(nodes) ) << "\n"
  );
  
  // Linear system
  CEigenLSS::Ptr lss(allocate_component<CEigenLSS>("LSS"));
  lss->matrix().resize(nb_segments+1, nb_segments+1);
  lss->matrix().setZero();
  lss->rhs().resize(nb_segments+1);
  lss->rhs().setZero();
  MeshTerm<1, LSS> blocks("system", lss);
  
  for_each_element< boost::mpl::vector<SF::Line1DLagrangeP1> >
  (
    
    blocks +=integral<1>
    (
      transpose( gradient(nodes, nodes) ) * gradient(nodes, nodes) * jacobian_determinant(nodes)
    )
  );
  
  std::cout << lss->matrix() << std::endl;
}


BOOST_AUTO_TEST_CASE( Heat1D )
{
  Real lenght     =     5.;
  Real temp_start =   100.;
  Real temp_stop  =   500.;

  const Uint nb_segments = 20;

  // build the mesh
  CMesh::Ptr mesh(allocate_component<CMesh>("line"));
  Tools::MeshGeneration::create_line(*mesh, lenght, nb_segments);
  
  // Geometric suport
  MeshTerm<0, ConstNodes> nodes( "ConductivityRegion", find_component_ptr_recursively_with_name<CRegion>(*mesh, "region") );
  
  // Linear system
  CEigenLSS::Ptr lss(allocate_component<CEigenLSS>("LSS"));
  lss->matrix().resize(nb_segments+1, nb_segments+1);
  lss->matrix().setZero();
  lss->rhs().resize(nb_segments+1);
  lss->rhs().setZero();
  MeshTerm<1, LSS> blocks("system", lss);
  
  for_each_element< boost::mpl::vector<SF::Line1DLagrangeP1> >
  (
    blocks += integral<1>(laplacian(nodes, nodes))
  );
  
  // Left boundary at temp_start
  for_each_node
  (
    find_component_recursively_with_name<CRegion>(*mesh, "xneg"),
    dirichlet(blocks) = temp_start
  );
  
  // Right boundary at temp_stop
  for_each_node
  (
    find_component_recursively_with_name<CRegion>(*mesh, "xpos"),
    dirichlet(blocks) = temp_stop
  );
  
  // Solve the system!
  const RealVector solution = lss->matrix().colPivHouseholderQr().solve(lss->rhs());
  
  // Check solution
  for(int i = 0; i != solution.rows(); ++i)
  {
    Real x = i * lenght / static_cast<Real>(nb_segments);
    CFinfo << "T(" << x << ") = " << solution[i] << CFendl;
    BOOST_CHECK_CLOSE(solution[i],temp_start + i * ( temp_stop - temp_start ) / static_cast<Real>(nb_segments), 1e-6);
  } 
}

BOOST_AUTO_TEST_CASE( Heat1DComponent )
{
  // Parameters
  Real length            = 5.;
  const Uint nb_segments = 5 ;
  
  // Create a document structure
  CRoot::Ptr root = CRoot::create("Root");
  CMesh::Ptr mesh = root->create_component<CMesh>("mesh");
  root->create_component<Solver::CPhysicalModel>("PhysicalModel");
  Tools::MeshGeneration::create_line(*mesh, length, nb_segments);
  CEigenLSS::Ptr lss = root->create_component<CEigenLSS>("LSS");
  
  // Create output field
  const std::vector<std::string> vars(1, "T[1]");
  mesh->create_field("Temperature", vars, CField::NODE_BASED);
  lss->configure_property("SolutionField", std::string("//Root/mesh/Temperature"));
  
  // Variable holding the geometric support
  MeshTerm<0, ConstNodes> nodes("ConductivityRegion");
  
  // Variable holding the field
  MeshTerm<1, ConstField<Real> > temperature("Temperature", "T");
  
  // Variable holding the LSS
  MeshTerm<2, LSS> blocks("LSS");
  
  // Create a CAction executing the expression
  CAction::Ptr heat1d_action = build_elements_action("Heat1D", *root, blocks += integral<1>( laplacian(nodes, temperature) * jacobian_determinant(nodes) ));
  
  // Configure the CAction
  heat1d_action->configure_property("ConductivityRegion", std::string("//Root/mesh/region"));
  heat1d_action->configure_property("LSS", std::string("//Root/LSS"));
  
  // Run the expression
  heat1d_action->execute();
  
  // Left boundary condition
  MeshTerm<3, ConfigurableConstant<Real> > xneg_temp("XnegTemp", "Temperature at the start of the domain");
  CAction::Ptr xneg_action = build_nodes_action("xneg", *root, dirichlet(blocks) = xneg_temp );  
  xneg_action->configure_property("Region", std::string("//Root/mesh/region/xneg"));
  xneg_action->configure_property("LSS", std::string("//Root/LSS"));
  xneg_action->configure_property("XnegTemp", 10.);
  xneg_action->execute();
  
  // Right boundary condition
  MeshTerm<4, ConfigurableConstant<Real> > xpos_temp("XposTemp", "Temperature at the end of the domain");
  CAction::Ptr xpos_action = build_nodes_action("xpos", *root, dirichlet(blocks) = xpos_temp );  
  xpos_action->configure_property("Region", std::string("//Root/mesh/region/xpos"));
  xpos_action->configure_property("LSS", std::string("//Root/LSS"));
  xpos_action->configure_property("XposTemp", 35.);
  xpos_action->execute();
  
  // Solve system
  lss->solve();
  
  // Print solution field
  for_each_node
  (
    get_named_component_typed<CRegion>(*mesh, "region"),
    _cout << "T(" << nodes << ") = " << temperature << "\n"
  );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
