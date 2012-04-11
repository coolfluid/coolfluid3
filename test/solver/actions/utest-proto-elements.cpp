// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for proto operators"

#include <boost/assign.hpp>
#include <boost/foreach.hpp>
#include <boost/test/unit_test.hpp>

#include "common/Core.hpp"
#include "common/Log.hpp"

#include "math/MatrixTypes.hpp"
#include "math/Consts.hpp"

#include "mesh/Domain.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Region.hpp"
#include "mesh/Elements.hpp"
#include "mesh/MeshWriter.hpp"
#include "mesh/ElementData.hpp"
#include "mesh/FieldManager.hpp"
#include "mesh/Dictionary.hpp"

#include "mesh/Integrators/Gauss.hpp"
#include "mesh/ElementTypes.hpp"
#include "mesh/LagrangeP0/Quad.hpp"

#include "mesh/BlockMesh/BlockData.hpp"

#include "physics/PhysModel.hpp"

#include "solver/Model.hpp"
#include "solver/Solver.hpp"
#include "solver/Tags.hpp"

#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/ElementLooper.hpp"
#include "solver/actions/Proto/Expression.hpp"
#include "solver/actions/Proto/Functions.hpp"
#include "solver/actions/Proto/NodeLooper.hpp"
#include "solver/actions/Proto/Terminals.hpp"

#include "Tools/Testing/TimedTestFixture.hpp"

using namespace cf3;
using namespace cf3::solver;
using namespace cf3::solver::actions;
using namespace cf3::solver::actions::Proto;
using namespace cf3::mesh;
using namespace cf3::common;

using namespace cf3::math::Consts;

using namespace boost::assign;

////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( ProtoOperatorsSuite )

//////////////////////////////////////////////////////////////////////////////

// Test working with element-based fields
BOOST_AUTO_TEST_CASE( ProtoElementField )
{
  // Setup a model
  Model& model = *Core::instance().root().create_component<Model>("Model");
  physics::PhysModel& phys_model = model.create_physics("cf3.physics.DynamicModel");
  Domain& dom = model.create_domain("Domain");
  Solver& solver = model.create_solver("cf3.solver.SimpleSolver");

  Mesh& mesh = *dom.create_component<Mesh>("mesh");

  // Simple graded mesh (to get non-constant volume)
  const Real length = 20.;
  const Real height = 20.;
  const Real ratio = 0.2;
  const Uint x_segs = 10;
  const Uint y_segs = 10;

  BlockMesh::BlockArrays& blocks = *dom.create_component<BlockMesh::BlockArrays>("blocks");

  *blocks.create_points(2, 4) << 0. << 0. << length << 0. << length << height << 0. << height;
  *blocks.create_blocks(1) << 0 << 1 << 2 << 3;
  *blocks.create_block_subdivisions() << x_segs << y_segs;
  *blocks.create_block_gradings() << ratio << ratio << ratio << ratio;
  
  *blocks.create_patch("bottom", 1) << 0 << 1;
  *blocks.create_patch("right", 1) << 1 << 2;
  *blocks.create_patch("top", 1) << 2 << 3;
  *blocks.create_patch("left", 1) << 3 << 0;

  blocks.create_mesh(mesh);

  mesh.check_sanity();

  // Declare a mesh variable
  MeshTerm<0, ScalarField> V("CellVolume", "volumes");

  // Store the total error
  Real total_error = 0;

  // Accepted element types
  boost::mpl::vector2<mesh::LagrangeP0::Quad, mesh::LagrangeP1::Quad2D> allowed_elements;

  // Expression to compute volumes, assuming rectangles
  boost::shared_ptr<Expression> volumes = elements_expression
  (
    allowed_elements,
    V = (nodes[1][0] - nodes[0][0]) * (nodes[3][1] - nodes[0][1])
  );

  // Register the variables
  volumes->register_variables(phys_model);
  // Add actions
  solver
    << create_proto_action("Volumes", volumes) // Setting the field
    << create_proto_action("Output", elements_expression(allowed_elements, total_error += V - volume)); // error calculation

  // Create the fields
  Dictionary& elems_P0 = mesh.create_discontinuous_space("elems_P0","cf3.mesh.LagrangeP0");
  solver.field_manager().create_field("volumes", elems_P0);

  // Set the region of all children to the root region of the mesh
  std::vector<URI> root_regions;
  root_regions.push_back(mesh.topology().uri());
  solver.configure_option_recursively(solver::Tags::regions(), root_regions);

  // Run
  model.simulate();

  BOOST_CHECK_SMALL(total_error, 1e-12);

  // Write mesh
  MeshWriter& writer = *model.domain().add_component(build_component_abstract_type<MeshWriter>("cf3.mesh.VTKXML.Writer", "writer")).handle<MeshWriter>();
  std::vector<URI> fields;
  fields.push_back(elems_P0.uri()/"volumes");
  writer.options().configure_option("fields",fields);
  writer.options().configure_option("mesh",mesh.handle<Mesh>());
  writer.options().configure_option("file",URI("utest-proto-elements_output.pvtu"));
  writer.execute();
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
