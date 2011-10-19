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

#include "Common/Core.hpp"
#include "Common/CRoot.hpp"
#include "Common/Log.hpp"

#include "Math/MatrixTypes.hpp"
#include "Math/Consts.hpp"

#include "Mesh/CDomain.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CMeshWriter.hpp"
#include "Mesh/ElementData.hpp"
#include "Mesh/FieldManager.hpp"
#include "Mesh/Geometry.hpp"

#include "Mesh/Integrators/Gauss.hpp"
#include "Mesh/ElementTypes.hpp"
#include "Mesh/LagrangeP0/Quad.hpp"

#include "Mesh/BlockMesh/BlockData.hpp"

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

#include "Tools/Testing/TimedTestFixture.hpp"

using namespace cf3;
using namespace cf3::Solver;
using namespace cf3::Solver::Actions;
using namespace cf3::Solver::Actions::Proto;
using namespace cf3::Mesh;
using namespace cf3::common;

using namespace cf3::Math::Consts;

using namespace boost::assign;

////////////////////////////////////////////////////

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

  // Simple graded mesh (to get non-constant volume)
  const Real length = 20.;
  const Real height = 20.;
  const Real ratio = 0.2;
  const Uint x_segs = 10;
  const Uint y_segs = 10;

  BlockMesh::BlockData& blocks = dom.create_component<BlockMesh::BlockData>("blocks");

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
  boost::mpl::vector2<Mesh::LagrangeP0::Quad, Mesh::LagrangeP1::Quad2D> allowed_elements;

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
  boost_foreach(CEntities& elements, mesh.topology().elements_range())
  {
    elements.create_space("elems_P0","CF.Mesh.LagrangeP0."+elements.element_type().shape_name());
  }
  FieldGroup& elems_P0 = mesh.create_field_group("elems_P0",FieldGroup::Basis::ELEMENT_BASED);
  solver.field_manager().create_field("volumes", elems_P0);

  // Set the region of all children to the root region of the mesh
  std::vector<URI> root_regions;
  root_regions.push_back(mesh.topology().uri());
  solver.configure_option_recursively(Solver::Tags::regions(), root_regions);

  // Run
  model.simulate();

  BOOST_CHECK_SMALL(total_error, 1e-12);

  // Write mesh
  CMeshWriter& writer = model.domain().add_component(build_component_abstract_type<CMeshWriter>("CF.Mesh.VTKXML.CWriter", "writer")).as_type<CMeshWriter>();
  std::vector<Field::Ptr> fields;
  fields.push_back(elems_P0.get_child("volumes").as_ptr<Field>());
  writer.set_fields(fields);
  writer.write_from_to(mesh, "utest-proto-elements_output.pvtu");
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
