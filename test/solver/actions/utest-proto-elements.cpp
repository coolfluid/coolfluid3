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

#include "solver/CModel.hpp"
#include "solver/CSolver.hpp"
#include "solver/Tags.hpp"

#include "solver/actions/Proto/CProtoAction.hpp"
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
  CModel& model = *Core::instance().root().create_component<CModel>("Model");
  physics::PhysModel& phys_model = model.create_physics("cf3.physics.DynamicModel");
  Domain& dom = model.create_domain("Domain");
  CSolver& solver = model.create_solver("cf3.solver.CSimpleSolver");

  Mesh& mesh = *dom.create_component<Mesh>("mesh");

  // Simple graded mesh (to get non-constant volume)
  const Real length = 20.;
  const Real height = 20.;
  const Real ratio = 0.2;
  const Uint x_segs = 10;
  const Uint y_segs = 10;

  BlockMesh::BlockData& blocks = *dom.create_component<BlockMesh::BlockData>("blocks");

  blocks.dimension = 2;
  blocks.scaling_factor = 1.;
  blocks.points += list_of(0.)(0.), list_of(length)(0.), list_of(length)(height), list_of(0.)(height);
  blocks.block_points += list_of(0)(1)(2)(3);
  blocks.block_subdivisions += list_of(x_segs)(y_segs);
  blocks.block_gradings += list_of(ratio)(ratio)(ratio)(ratio);
  blocks.patch_names += "bottom", "right", "top",  "left";
  blocks.patch_types += "wall", "wall",  "wall", "wall";
  blocks.patch_points += list_of(0)(1), list_of(1)(2), list_of(2)(3), list_of(3)(0);
  blocks.block_distribution += 0, 1;

  BlockMesh::build_mesh(blocks, mesh);

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
  std::vector<Handle< Field > > fields;
  fields.push_back(Handle<Field>(elems_P0.get_child("volumes")));
  writer.set_fields(fields);
  writer.write_from_to(mesh, "utest-proto-elements_output.pvtu");
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
