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
using namespace CF::Solver;
using namespace CF::Solver::Actions;
using namespace CF::Solver::Actions::Proto;
using namespace CF::Common;
using namespace CF::Math::MathConsts;
using namespace CF::Mesh;

using namespace boost;

struct ProtoUnsteadyFixture
{
  ProtoUnsteadyFixture() :
    length(5.),
    ambient_temp(500.),
    initial_temp(298.),
    nb_segments(100),
    k(1.),
    alpha(1.),
    start_time(1.),
    end_time(1.1),
    dt(0.005),
    t(start_time)
  {
  }
  
  /// Write the analytical solution, according to "A Heat transfer textbook", section 5.3
  void set_analytical_solution(CRegion& region, const std::string& field_name, const std::string& var_name)
  {
    MeshTerm<0, ConstNodes> nodes( "ConductivityRegion", boost::dynamic_pointer_cast<CRegion>(region.follow()) );
    MeshTerm<1, Field<Real> > T(field_name, var_name);
    
    // Zero the field
    for_each_node
    (
      region,
      T = 0.
    );
    
    const Real Fo = alpha * t / (0.25*length*length); // Fourier number
    for(Uint i = 0; i != 1000; ++i) // First 1000 (to be sure ;) terms of the series that makes up the analytical solution (in terms of adimensional temperature)
    {
      const Real n = 1. + 2. * static_cast<Real>(i);
      for_each_node
      (
        region,
        T += 4./pi() * 1./n * _exp( pow<2>( 0.5*n*pi() ) * (-Fo) ) * _sin(0.5*n*pi()*nodes(0,0)/(0.5*length))
      );
    }
    
    // Convert the result from the adimensional form to a real temperature
    for_each_node
    (
      region,
      T = T*(initial_temp - ambient_temp) + ambient_temp
    );
  }
  
  const Real length;
  const Real ambient_temp;
  const Real initial_temp;
  const Uint nb_segments;
  const Real k;
  const Real alpha;
  const Real start_time;
  const Real end_time;
  const Real dt;
  Real t;
};

BOOST_FIXTURE_TEST_SUITE( ProtoUnsteadySuite, ProtoUnsteadyFixture )

BOOST_AUTO_TEST_CASE( Heat1DUnsteady )
{
  const Real invdt = 1. / dt;

  // Setup document structure and mesh
  CRoot::Ptr root = CRoot::create("Root");
  
  CMesh::Ptr mesh = root->create_component<CMesh>("mesh");
  root->create_component<Solver::CPhysicalModel>("PhysicalModel");
  Tools::MeshGeneration::create_line(*mesh, length, nb_segments);
  
  // Linear system
  CEigenLSS::Ptr lss = root->create_component<CEigenLSS>("LSS");
  lss->matrix().resize(nb_segments+1, nb_segments+1);
  lss->matrix().setZero();
  lss->rhs().resize(nb_segments+1);
  lss->rhs().setZero();
  
  // Create output field
  const std::vector<std::string> vars(1, "T[1]");
  mesh->create_field("Temperature", vars, CField::NODE_BASED);
  lss->configure_property("SolutionField", URI("cpath://Root/mesh/Temperature"));
  
  // Create a field for the analytical solution
  mesh->create_field("TemperatureAnalytical", vars, CField::NODE_BASED);
  
  // Regions
  CRegion& fluid = find_component_recursively_with_name<CRegion>(*mesh, "region");
  CRegion& xneg = find_component_recursively_with_name<CRegion>(*mesh, "xneg");
  CRegion& xpos = find_component_recursively_with_name<CRegion>(*mesh, "xpos");
  
  // Term for the geometric suport
  MeshTerm<0, ConstNodes> nodes( "ConductivityRegion", find_component_ptr_recursively_with_name<CRegion>(*mesh, "region") );
  
  // Term for the linear system
  MeshTerm<1, LSS> blocks("system", lss);
  
  // Read-only access to the result field (faster)
  MeshTerm<2, ConstField<Real> > temperature("Temperature", "T");
  
  // Writable access to the result field
  MeshTerm<3, Field<Real> > temperature_writable("Temperature", "T");
  
  MeshTerm<4, Field<Real> > temperature_analytical("TemperatureAnalytical", "T");
  
  // Set initial condition. We should use the analytical solution at start_time != 0 because it is smooth
  set_analytical_solution(fluid, "Temperature", "T");
  
  while(t < end_time)
  {
    // Fill the system matrix
    for_each_element< boost::mpl::vector<SF::Line1DLagrangeP1> >
    (
      blocks += integral<1>( ( invdt * sf_outer_product(temperature) + 0.5 * alpha * laplacian(nodes, temperature) ) * jacobian_determinant(nodes) )
    );
    
    // Fill RHS
    for_each_element< boost::mpl::vector<SF::Line1DLagrangeP1> >
    (
      accumulate_rhs(blocks) += integral<1>( ( laplacian(nodes, temperature) ) * ( jacobian_determinant(nodes) * (-alpha) ) * temperature) 
    );
    
    // Left boundary at ambient temperature
    for_each_node
    (
      xneg,
      dirichlet(blocks) = ambient_temp - temperature
    );
    
    // Right boundary at ambient temperature
    for_each_node
    (
      xpos,
      dirichlet(blocks) = ambient_temp - temperature
    );
    
    // Solve the system!
    lss->solve();
    
    // Check the solution
    set_analytical_solution(fluid, "TemperatureAnalytical", "T");
    for_each_node
    (
      find_component_with_name<CRegion>(*mesh, "region"),
      _check_close(temperature_analytical, temperature, 0.2)
    );
    
    t += dt;
  }
}

BOOST_AUTO_TEST_SUITE_END()
