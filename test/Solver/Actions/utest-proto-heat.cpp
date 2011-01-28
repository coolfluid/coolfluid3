// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for heat-conduction related proto operations"

#include <boost/foreach.hpp>
#include <boost/test/unit_test.hpp>

#include "Solver/Actions/Proto/CProtoElementsAction.hpp"
#include "Solver/Actions/Proto/CProtoNodesAction.hpp"
#include "Solver/Actions/Proto/ElementLooper.hpp"
#include "Solver/Actions/Proto/Functions.hpp"
#include "Solver/Actions/Proto/NodeLooper.hpp"
#include "Solver/Actions/Proto/Terminals.hpp"

#include "Common/CRoot.hpp"
#include "Common/Log.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CMeshReader.hpp"
#include "Mesh/ElementData.hpp"

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

BOOST_AUTO_TEST_SUITE( ProtoHeatSuite )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Laplacian1D )
{
  const Uint nb_segments = 5;
  
  // Setup document structure and mesh
  CRoot::Ptr root = CRoot::create("Root");
  
  CMesh::Ptr mesh = root->create_component<CMesh>("mesh");
  Tools::MeshGeneration::create_line(*mesh, 5., nb_segments);
  
  // Linear system
  CEigenLSS& lss = *root->create_component<CEigenLSS>("LSS");
  
  // Create output field
  const std::vector<std::string> vars(1, "T[1]");
  mesh->create_field("Temperature", vars, CField::NODE_BASED);
  lss.configure_property("SolutionField", URI("cpath://Root/mesh/Temperature"));
  
  // Geometric suport
  MeshTerm<0, ConstNodes> nodes( "CalculationRegion", find_component_ptr_recursively_with_name<CRegion>(*mesh, "region") );
  
  for_each_element< boost::mpl::vector<SF::Line1DLagrangeP1> >
  (
    _cout << "elem result:\n" << integral<1>( laplacian(nodes, nodes) * jacobian_determinant(nodes) ) << "\n"
  );
  
  for_each_element< boost::mpl::vector<SF::Line1DLagrangeP1> >
  (
    
    system_matrix(lss) +=integral<1>
    (
      transpose( gradient(nodes, nodes) ) * gradient(nodes, nodes) * jacobian_determinant(nodes)
    )
  );
  
  std::cout << lss.matrix() << std::endl;
}

BOOST_AUTO_TEST_CASE( Heat1D )
{
  Real length     =     5.;
  Real temp_start =   100.;
  Real temp_stop  =   500.;

  const Uint nb_segments = 20;

  // Setup document structure and mesh
  CRoot::Ptr root = CRoot::create("Root");
  
  CMesh::Ptr mesh = root->create_component<CMesh>("mesh");
  Tools::MeshGeneration::create_line(*mesh, length, nb_segments);
  
  // Linear system
  CEigenLSS& lss = *root->create_component<CEigenLSS>("LSS");
  
  // Create output field
  const std::vector<std::string> vars(1, "T[1]");
  mesh->create_field("Temperature", vars, CField::NODE_BASED);
  lss.configure_property("SolutionField", URI("cpath://Root/mesh/Temperature"));
  
  // Geometric suport
  MeshTerm<0, ConstNodes> nodes( "CalculationRegion", find_component_ptr_recursively_with_name<CRegion>(*mesh, "region") );
  
  for_each_element< boost::mpl::vector<SF::Line1DLagrangeP1> >
  (
    system_matrix(lss) += integral<1>( laplacian(nodes, nodes) * jacobian_determinant(nodes) )
  );
  
  // Left boundary at temp_start
  for_each_node
  (
    find_component_recursively_with_name<CRegion>(*mesh, "xneg"),
    dirichlet(lss) = temp_start
  );

  // Right boundary at temp_stop
  for_each_node
  (
    find_component_recursively_with_name<CRegion>(*mesh, "xpos"),
    dirichlet(lss) = temp_stop
  );

  // Solve the system!
  const RealVector solution = lss.matrix().colPivHouseholderQr().solve(lss.rhs());
  
  // Check solution
  for(int i = 0; i != solution.rows(); ++i)
  {
    Real x = i * length / static_cast<Real>(nb_segments);
    CFinfo << "T(" << x << ") = " << solution[i] << CFendl;
    BOOST_CHECK_CLOSE(solution[i],temp_start + i * ( temp_stop - temp_start ) / static_cast<Real>(nb_segments), 1e-6);
  } 
}

// Heat conduction with Neumann BC
BOOST_AUTO_TEST_CASE( Heat1DNeumannBC )
{
  const Real length     =     5.;
  const Real temp_start =   100.;
  const Real temp_stop  =   500.;
  const Real k = 1.;
  const Real q = k * (temp_stop - temp_start) / length;

  const Uint nb_segments = 5;

  // Setup document structure and mesh
  CRoot::Ptr root = CRoot::create("Root");
  
  CMesh::Ptr mesh = root->create_component<CMesh>("mesh");
  Tools::MeshGeneration::create_line(*mesh, length, nb_segments);
  
  // Linear system
  CEigenLSS& lss = *root->create_component<CEigenLSS>("LSS");
  
  // Create output field
  const std::vector<std::string> vars(1, "T[1]");
  mesh->create_field("Temperature", vars, CField::NODE_BASED);
  lss.configure_property("SolutionField", URI("cpath://Root/mesh/Temperature"));
  
  // Term for the geometric suport
  MeshTerm<0, ConstNodes> nodes( "CalculationRegion", mesh->topology().as_type<CRegion>() );
  
  MeshTerm<2, ConstField<Real> > temperature("Temperature", "T");
  
  for_each_element< boost::mpl::vector<SF::Line1DLagrangeP1> >
  (
    system_matrix(lss) += integral<1>( k * laplacian(nodes, temperature) * jacobian_determinant(nodes) )
  );
  
  // Right boundary at constant heat flux
  for_each_node
  (
    find_component_recursively_with_name<CRegion>(*mesh, "xpos"),
    neumann(lss) = q
  );
  
  // Left boundary at temp_start
  for_each_node
  (
    find_component_recursively_with_name<CRegion>(*mesh, "xneg"),
    dirichlet(lss) = temp_start
  );
  
  // Solve the system!
  lss.solve();
  
  // Check solution
  for_each_node
  (
    mesh->topology(),
    _check_close(temp_start + q * nodes(0,0), temperature, 1e-6)
  );
}

BOOST_AUTO_TEST_CASE( Heat1DComponent )
{
  // Parameters
  Real length            = 5.;
  const Uint nb_segments = 5 ;
  
  BOOST_CHECK(true);

  // Setup document structure and mesh
  CRoot::Ptr root = CRoot::create("Root");
  
  CMesh::Ptr mesh = root->create_component<CMesh>("mesh");
  Tools::MeshGeneration::create_line(*mesh, length, nb_segments);
  
  // Linear system
  CEigenLSS& lss = *root->create_component<CEigenLSS>("LSS");
  
  BOOST_CHECK(true);

  // Create output field
  const std::vector<std::string> vars(1, "T[1]");
  mesh->create_field("Temperature", vars, CField::NODE_BASED);
  lss.configure_property("SolutionField", URI("cpath://Root/mesh/Temperature"));
  
  BOOST_CHECK(true);

  // Variable holding the geometric support
  MeshTerm<0, ConstNodes> nodes("CalculationRegion");
  
  // Variable holding the field
  MeshTerm<1, ConstField<Real> > temperature("Temperature", "T");
  
  BOOST_CHECK(true);

  // Create a CAction executing the expression
  CAction::Ptr heat1d_action = build_elements_action("Heat1D", *root, system_matrix(lss) += integral<1>( laplacian(nodes, temperature) * jacobian_determinant(nodes) ));
  
  BOOST_CHECK(true);

  // Configure the CAction
  heat1d_action->configure_property("CalculationRegion", mesh->topology().full_path());
  
  // Run the expression
  heat1d_action->execute();
  
  BOOST_CHECK(true);

  // Left boundary condition
  MeshTerm<3, ConfigurableConstant<Real> > xneg_temp("XnegTemp", "Temperature at the start of the domain");
  CAction::Ptr xneg_action = build_nodes_action("xneg", *root, dirichlet(lss) = xneg_temp );  
  xneg_action->configure_property("Region", find_component_recursively_with_name<CRegion>(mesh->topology(), "xneg").full_path());
  xneg_action->configure_property("XnegTemp", 10.);
  xneg_action->execute();
  
  BOOST_CHECK(true);

  // Right boundary condition
  MeshTerm<4, ConfigurableConstant<Real> > xpos_temp("XposTemp", "Temperature at the end of the domain");
  CAction::Ptr xpos_action = build_nodes_action("xpos", *root, dirichlet(lss) = xpos_temp );  
  xpos_action->configure_property("Region", find_component_recursively_with_name<CRegion>(mesh->topology(), "xpos").full_path());
  xpos_action->configure_property("XposTemp", 35.);
  xpos_action->execute();
  
  // Solve system
  lss.solve();
  
  // Print solution field
  for_each_node
  (
    mesh->topology(),
    _cout << "T(" << nodes << ") = " << temperature << "\n"
  );
}

/// 1D heat conduction with a volume heat source
BOOST_AUTO_TEST_CASE( Heat1DVolumeTerm )
{
  Real length              = 5.;
  Real ambient_temp        = 293.;
  const Uint nb_segments   = 20;
  const Real k             = 100.; // thermal conductivity
  const Real q             = 100.; // Heat production per volume

  // Setup document structure and mesh
  CRoot::Ptr root = CRoot::create("Root");
  
  CMesh::Ptr mesh = root->create_component<CMesh>("mesh");
  Tools::MeshGeneration::create_line(*mesh, length, nb_segments);
  
  // Linear system
  CEigenLSS& lss = *root->create_component<CEigenLSS>("LSS");
  
  // Geometric suport
  MeshTerm<0, ConstNodes> nodes( "CalculationRegion", mesh->topology().as_type<CRegion>() );
  
  // Create output field
  const std::vector<std::string> vars(1, "T[1]");
  mesh->create_field("Temperature", vars, CField::NODE_BASED);
  lss.configure_property("SolutionField", URI("cpath://Root/mesh/Temperature"));
  
  // Variable holding the field
  MeshTerm<2, ConstField<Real> > temperature("Temperature", "T");
  
  for_each_element< boost::mpl::vector<SF::Line1DLagrangeP1> >
  (
    system_matrix(lss) += integral<1>( k * laplacian(nodes, temperature) * jacobian_determinant(nodes) )
  );
  
  MeshTerm<3, ConstField<Real> > heat("heat", q);
  
  for_each_element< boost::mpl::vector<SF::Line1DLagrangeP1> >
  (
    system_rhs(lss) += integral<1>( jacobian_determinant(nodes) * sf_outer_product(temperature) * heat )
  );
  
  // Left boundary at ambient temperature
  for_each_node
  (
    find_component_recursively_with_name<CRegion>(mesh->topology(), "xneg"),
    dirichlet(lss) = ambient_temp
  );
  
  // Right boundary at ambient temperature
  for_each_node
  (
    find_component_recursively_with_name<CRegion>(mesh->topology(), "xpos"),
    dirichlet(lss) = ambient_temp
  );
  // Solve the system!
  const RealVector solution = lss.matrix().colPivHouseholderQr().solve(lss.rhs());
  
  // Check solution
  for(int i = 0; i != solution.rows(); ++i)
  {
    Real x = i * length / static_cast<Real>(nb_segments);
    BOOST_CHECK_CLOSE
    (
      solution[i],
      -q/(2.*k)*x*x + q*length/(2.*k)*x + ambient_temp, // analytical solution, see "A Heat transfer textbook, section 2.2
      1e-6
    );
  }
  CFinfo << CFendl;
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
