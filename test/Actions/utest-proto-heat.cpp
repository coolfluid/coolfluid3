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

#include "Tools/MeshGeneration/MeshGeneration.hpp"

using namespace CF;
using namespace CF::Actions;
using namespace CF::Actions::Proto;
using namespace CF::Mesh;
using namespace CF::Common;

using namespace boost;

BOOST_AUTO_TEST_SUITE( ProtoHeatSuite )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Laplacian1D )
{
  const Uint nb_segments = 5;
  
  CMesh::Ptr mesh(allocate_component_type<CMesh>("line"));
  Tools::MeshGeneration::create_line(*mesh, 5., nb_segments);
  
  // Geometric suport
  MeshTerm<0, ConstNodes> nodes( "ConductivityRegion", find_component_ptr_recursively_with_name<CRegion>(*mesh, "region") );
  
  for_each_element< boost::mpl::vector<SF::Line1DLagrangeP1> >
  (
    _cout << "elem result:\n" << integral<1>( laplacian(nodes, nodes) * jacobian_determinant(nodes) ) << "\n"
  );
  
  // Linear system
  CProtoLSS::Ptr lss(allocate_component_type<CProtoLSS>("LSS"));
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
  CMesh::Ptr mesh(allocate_component_type<CMesh>("line"));
  Tools::MeshGeneration::create_line(*mesh, lenght, nb_segments);
  
  // Geometric suport
  MeshTerm<0, ConstNodes> nodes( "ConductivityRegion", find_component_ptr_recursively_with_name<CRegion>(*mesh, "region") );
  
  // Linear system
  CProtoLSS::Ptr lss(allocate_component_type<CProtoLSS>("LSS"));
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
    recursive_get_named_component_typed<CRegion>(*mesh, "xneg"),
    dirichlet(blocks) = temp_start
  );
  
  // Right boundary at temp_stop
  for_each_node
  (
    recursive_get_named_component_typed<CRegion>(*mesh, "xpos"),
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
  Real lenght     =     5.;
  Real temp_start =   100.;
  Real temp_stop  =   500.;
  const Uint nb_segments = 5;
  
  // Create a document structure
  CRoot::Ptr root = CRoot::create("Root");
  CMesh::Ptr mesh = root->create_component_type<CMesh>("mesh");
  root->create_component_type<Solver::CPhysicalModel>("PhysicalModel");
  Tools::MeshGeneration::create_line(*mesh, lenght, nb_segments);
  CProtoLSS::Ptr lss = root->create_component_type<CProtoLSS>("LSS");
  
  // Variable holding the geometric support
  MeshTerm<0, ConstNodes> nodes("ConductivityRegion");
  
  // Variable holding the LSS
  MeshTerm<1, LSS> blocks("LSS");
  
  // Create a CAction executing the expression
  CAction::Ptr heat1d_action = build_elements_action("Heat1D", *root, blocks += integral<1>( laplacian(nodes, nodes) * jacobian_determinant(nodes) ));
  
  // Configure the CAction
  heat1d_action->configure_property("ConductivityRegion", std::string("//Root/mesh/region"));
  heat1d_action->configure_property("LSS", std::string("//Root/LSS"));
  
  // Run the expression
  heat1d_action->execute();
  
  // Left boundary condition
  CAction::Ptr xmin_action = build_nodes_action("xmin", *root, dirichlet(blocks) = temp_start );
  
  xmin_action->configure_property("Region", std::string("//Root/mesh/xpos"));
  xmin_action->configure_property("LSS", std::string("//Root/LSS"));
  
  //xmin_action->execute();
  
  // print result
  std::cout << lss->matrix() << std::endl;
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
