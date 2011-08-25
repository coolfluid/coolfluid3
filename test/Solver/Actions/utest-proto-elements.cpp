// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for proto operators"

#include <boost/foreach.hpp>
#include <boost/test/unit_test.hpp>

#include "Common/Core.hpp"
#include "Common/CRoot.hpp"
#include "Common/Log.hpp"

#include "Math/MatrixTypes.hpp"

#include "Mesh/CDomain.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CMeshWriter.hpp"
#include "Mesh/ElementData.hpp"
#include "Mesh/FieldManager.hpp"
#include "Mesh/Geometry.hpp"

#include "Mesh/Integrators/Gauss.hpp"
#include "Mesh/SF/Types.hpp"
#include "Mesh/SF/SFQuadLagrangeP0.hpp"

#include "Physics/PhysModel.hpp"

#include "Solver/CModel.hpp"
#include "Solver/CSolver.hpp"
#include "Solver/Tags.hpp"

#include "Solver/Actions/Proto/CProtoAction.hpp"
#include "Solver/Actions/Proto/ElementLooper.hpp"
#include "Solver/Actions/Proto/Expression.hpp"
#include "Solver/Actions/Proto/Functions.hpp"
#include "Solver/Actions/Proto/NodeLooper.hpp"
#include "Solver/Actions/Proto/Terminals.hpp"

#include "Tools/MeshGeneration/MeshGeneration.hpp"
#include "Tools/Testing/TimedTestFixture.hpp"
#include "Tools/Testing/ProfiledTestFixture.hpp"

using namespace CF;
using namespace CF::Solver;
using namespace CF::Solver::Actions;
using namespace CF::Solver::Actions::Proto;
using namespace CF::Mesh;
using namespace CF::Common;

using namespace CF::Math::Consts;

using namespace boost;

/// Check close, for testing purposes
inline void check_close(const RealMatrix2& a, const RealMatrix2& b, const Real threshold)
{
  for(Uint i = 0; i != a.rows(); ++i)
    for(Uint j = 0; j != a.cols(); ++j)
      BOOST_CHECK_CLOSE(a(i,j), b(i,j), threshold);
}

static boost::proto::terminal< void(*)(const RealMatrix2&, const RealMatrix2&, Real) >::type const _check_close = {&check_close};

////////////////////////////////////////////////////

/// List of all supported shapefunctions that allow high order integration
typedef boost::mpl::vector3< SF::Line1DLagrangeP1,
                            SF::Quad2DLagrangeP1,
                            SF::Hexa3DLagrangeP1
> HigherIntegrationElements;

typedef boost::mpl::vector5< SF::Line1DLagrangeP1,
                            SF::Triag2DLagrangeP1,
                            SF::Quad2DLagrangeP1,
                            SF::Hexa3DLagrangeP1,
                            SF::Tetra3DLagrangeP1
> VolumeTypes;

BOOST_AUTO_TEST_SUITE( ProtoOperatorsSuite )

//////////////////////////////////////////////////////////////////////////////

// Test working with element-based fields
BOOST_AUTO_TEST_CASE( ProtoElementField )
{
  // Setup a model
  CModel& model = Core::instance().root().create_component<CModel>("Model");
  Physics::PhysModel& phys_model = model.create_physics("CF.Physics.DynamicModel");
  CDomain& dom = model.create_domain("Domain");
  CSolver& solver = model.create_solver("CF.Solver.CSimpleSolver");

  CMesh& mesh = dom.create_component<CMesh>("mesh");
  Tools::MeshGeneration::create_rectangle(mesh, 5, 5, 5, 5);

  // Declare a mesh variable
  MeshTerm<0, ScalarField> T("Temperature", "solution");

  // Accepted element types
  boost::mpl::vector2<Mesh::SF::SFQuadLagrangeP0, Mesh::SF::Quad2DLagrangeP1> allowed_elements;
  
  // Expression to compute volumes
  boost::shared_ptr<Expression> volumes = elements_expression(allowed_elements, _cout << nodal_values(T));
  // Register the variables
  volumes->register_variables(phys_model);
  // Add action
  solver << create_proto_action("Volumes", volumes);

  // Create the fields
  solver.field_manager().create_field("solution", mesh.geometry());
  
  // Set the region of all children to the root region of the mesh
  std::vector<URI> root_regions;
  root_regions.push_back(mesh.topology().uri());
  solver.configure_option_recursively(Solver::Tags::regions(), root_regions);

  // Run
  model.simulate();

  // Write mesh
  CMeshWriter& writer = model.domain().add_component(build_component_abstract_type<CMeshWriter>("CF.Mesh.VTKXML.CWriter", "writer")).as_type<CMeshWriter>();
  std::vector<Field::Ptr> fields;
  fields.push_back(mesh.geometry().get_child("solution").as_ptr<Field>());
  writer.set_fields(fields);
  writer.write_from_to(mesh, "utest-proto-elements_output.pvtu");
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
