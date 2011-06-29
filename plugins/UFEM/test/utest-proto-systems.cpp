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
  const Real dt = 0.1;
  Real t = start_time;
  
  const boost::proto::literal<RealVector2> alpha(RealVector2(1., 2.));
  
  const Real invdt = 1. / dt;
  
  // Setup document structure and mesh
  CRoot& root = Core::instance().root();
  
  CMesh::Ptr mesh = root.create_component_ptr<CMesh>("mesh");
  Tools::MeshGeneration::create_rectangle(*mesh, length, 0.5*length, 2*nb_segments, nb_segments);
  
  // Linear system
  CEigenLSS& lss = *root.create_component_ptr<CEigenLSS>("LSS");
  lss.set_config_file(boost::unit_test::framework::master_test_suite().argv[1]);
  
  // Regions
  CRegion& left = find_component_recursively_with_name<CRegion>(*mesh, "left");
  CRegion& right = find_component_recursively_with_name<CRegion>(*mesh, "right");
  CRegion& bottom = find_component_recursively_with_name<CRegion>(*mesh, "bottom");
  CRegion& top = find_component_recursively_with_name<CRegion>(*mesh, "top");

  // Expression variables
  MeshTerm<0, VectorField> v("VectorVariable", "v");
  
  // Set up a physical model (normally handled automatically if using the Component wrappers)
  PhysicalModel physical_model;
  physical_model.register_variable(v, true);
  physical_model.create_fields(*mesh);
  lss.resize(physical_model.nb_dofs() * mesh->nodes().size());
  
  // Setup a mesh writer
  CMeshWriter::Ptr writer = build_component_abstract_type<CMeshWriter>("CF.Mesh.Gmsh.CWriter","meshwriter");
  root.add_component(writer);
  writer->configure_option( "fields", std::vector<URI>(1, mesh->get_child("VectorVariable").uri() ) );
  
  // Set initial condition.
  for_each_node
  (
    mesh->topology(),
    v = initial_temp
  );
  
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
        element_quadrature <<
        (
          _A(v[_i], v[_i]) += transpose(nabla(v)) * alpha[_i] * nabla(v),
          _T(v[_i], v[_i]) += invdt * (transpose(N(v)) * N(v))
        ),
        system_matrix(lss) += _T + 0.5 * _A,
        system_rhs(lss) -= _A * _b
      )
    );
    
    // All boundaries at outside temp
    for_each_node
    (
      left,
      dirichlet(lss, v) = outside_temp,
      physical_model
    );
    
    for_each_node
    (
      right,
      dirichlet(lss, v) = outside_temp,
      physical_model
    );
    
    for_each_node
    (
      top,
      dirichlet(lss, v) = outside_temp,
      physical_model
    );
    
    for_each_node
    (
      bottom,
      dirichlet(lss, v) = outside_temp,
      physical_model
    );
     
    // Solve the system!
    lss.solve();
    physical_model.update_fields(*mesh, lss.solution());
    
    t += dt;
  }
  URI output_file("systems.msh");
  writer->write_from_to(*mesh, output_file);
}

// Expected matrices:
// 82:  0.5    0 -0.5    0    0    0    0    0
// 82:    0  0.5    0 -0.5    0    0    0    0
// 82: -0.5    0  0.5    0    0    0    0    0
// 82:    0 -0.5    0  0.5    0    0    0    0
// 82:    0    0    0    0  0.5    0 -0.5    0
// 82:    0    0    0    0    0  0.5    0 -0.5
// 82:    0    0    0    0 -0.5    0  0.5    0
// 82:    0    0    0    0    0 -0.5    0  0.5
// 82: 
// 82: 0.0078125 0.0078125 0.0078125 0.0078125         0         0         0         0
// 82: 0.0078125 0.0078125 0.0078125 0.0078125         0         0         0         0
// 82: 0.0078125 0.0078125 0.0078125 0.0078125         0         0         0         0
// 82: 0.0078125 0.0078125 0.0078125 0.0078125         0         0         0         0
// 82:         0         0         0         0 0.0078125 0.0078125 0.0078125 0.0078125
// 82:         0         0         0         0 0.0078125 0.0078125 0.0078125 0.0078125
// 82:         0         0         0         0 0.0078125 0.0078125 0.0078125 0.0078125
// 82:         0         0         0         0 0.0078125 0.0078125 0.0078125 0.0078125

BOOST_AUTO_TEST_SUITE_END()
