// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for heat-conduction related proto operations"

#include <boost/assign.hpp>
#include <boost/test/unit_test.hpp>

#include "common/Core.hpp"
#include "common/Environment.hpp"

#include "common/PE/CommPattern.hpp"

#include "math/LSS/System.hpp"

#include "mesh/Domain.hpp"
#include "mesh/LagrangeP1/Line1D.hpp"

#include "solver/Model.hpp"

#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"

#include "Tools/MeshGeneration/MeshGeneration.hpp"
#include "mesh/MeshGenerator.hpp"

#include "UFEM/LSSAction.hpp"
#include "UFEM/Solver.hpp"
#include "UFEM/SparsityBuilder.hpp"
#include "UFEM/Tags.hpp"
#include "math/LSS/SolveLSS.hpp"

using namespace cf3;
using namespace cf3::solver;
using namespace cf3::solver::actions;
using namespace cf3::solver::actions::Proto;
using namespace cf3::common;
using namespace cf3::math;
using namespace cf3::math::Consts;
using namespace cf3::mesh;

using namespace boost::assign;

struct UFEMBuildSparsityFixture
{
  UFEMBuildSparsityFixture() :
    root( Core::instance().root() )
  {
  }

  Component& root;
};

BOOST_FIXTURE_TEST_SUITE( UFEMBuildSparsitySuite, UFEMBuildSparsityFixture )

BOOST_AUTO_TEST_CASE( InitMPI )
{
  common::PE::Comm::instance().init(boost::unit_test::framework::master_test_suite().argc, boost::unit_test::framework::master_test_suite().argv);
  BOOST_CHECK_EQUAL(common::PE::Comm::instance().size(), 1);
}

BOOST_AUTO_TEST_CASE( Sparsity1D )
{
  Core::instance().environment().options().set("log_level", 4u);

  // Parameters
  Real length            = 5.;
  const Uint nb_segments = 5 ;
  const Uint nb_nodes = nb_segments + 1;

  // Setup a model
  Model& model = *root.create_component<Model>("Model");
  Domain& domain = model.create_domain("Domain");

  LSS::System& lss = *model.create_component<LSS::System>("LSS");
  lss.options().option("matrix_builder").change_value(std::string("cf3.math.LSS.TrilinosFEVbrMatrix"));

  // Setup mesh
  // Mesh& mesh = *domain.create_component<Mesh>("Mesh");
  // Tools::MeshGeneration::create_line(mesh, length, nb_segments);
  boost::shared_ptr<MeshGenerator> create_line = build_component_abstract_type<MeshGenerator>("cf3.mesh.SimpleMeshGenerator","create_line");
  create_line->options().set("mesh",domain.uri()/"Mesh");
  create_line->options().set("lengths",std::vector<Real>(DIM_1D, length));
  create_line->options().set("nb_cells",std::vector<Uint>(DIM_1D, nb_segments));
  Mesh& mesh = create_line->generate();

  // Setup sparsity
  std::vector<Uint> node_connectivity, starting_indices;
  Handle< List<Uint> > gids = domain.create_component< List<Uint> >("GIDs");
  Handle< List<Uint> > ranks = domain.create_component< List<Uint> >("Ranks");
  Handle< List<Uint> > used_node_map = domain.create_component< List<Uint> >("used_node_map");
  UFEM::build_sparsity(std::vector< Handle<Region> >(1, mesh.topology().handle<Region>()), mesh.geometry_fields(), node_connectivity, starting_indices, *gids, *ranks, *used_node_map);

  // Check result
  BOOST_CHECK_EQUAL(starting_indices[0], 0u);
  for(Uint i = 2; i != nb_nodes; ++i)
    BOOST_CHECK_EQUAL(starting_indices[i] - starting_indices[i-1], 3);

  PE::CommPattern& comm_pattern = *domain.create_component<PE::CommPattern>("CommPattern");
  comm_pattern.insert("gid",gids->array(),false);
  comm_pattern.setup(Handle<PE::CommWrapper>(comm_pattern.get_child("gid")),ranks->array());

  // Create the LSS
  lss.create(comm_pattern, 1u, node_connectivity, starting_indices);


  // Write the matrix
  lss.matrix()->print("utest-ufem-buildsparsity_heat_matrix_1D.plt");
}

BOOST_AUTO_TEST_CASE( Sparsity2DQuads )
{
  Core::instance().environment().options().set("log_level", 4u);

  // Parameters
  Real length            = 5.;
  const Uint nb_segments = 5 ;
  const Uint nb_nodes = (nb_segments+1) * (nb_segments+1);

  // Setup a model
  Model& model = *root.create_component<Model>("Model");
  Domain& domain = model.create_domain("Domain");

  LSS::System& lss = *model.create_component<LSS::System>("LSS");
  lss.options().option("matrix_builder").change_value(std::string("cf3.math.LSS.TrilinosFEVbrMatrix"));

  // Setup mesh
  Mesh& mesh = *domain.create_component<Mesh>("Mesh");
  Tools::MeshGeneration::create_rectangle(mesh, length, length, nb_segments, nb_segments);

  // Setup sparsity
  std::vector<Uint> node_connectivity, starting_indices;
  Handle< List<Uint> > gids = domain.create_component< List<Uint> >("GIDs");
  Handle< List<Uint> > ranks = domain.create_component< List<Uint> >("Ranks");
  Handle< List<Uint> > used_node_map = domain.create_component< List<Uint> >("used_node_map");
  UFEM::build_sparsity(std::vector< Handle<Region> >(1, mesh.topology().handle<Region>()), mesh.geometry_fields(), node_connectivity, starting_indices, *gids, *ranks, *used_node_map);

  PE::CommPattern& comm_pattern = *domain.create_component<PE::CommPattern>("CommPattern");
  comm_pattern.insert("gid",gids->array(),false);
  comm_pattern.setup(Handle<PE::CommWrapper>(comm_pattern.get_child("gid")),ranks->array());

  // Create the LSS
  lss.create(comm_pattern, 1u, node_connectivity, starting_indices);


  // Write the matrix
  lss.matrix()->print("utest-ufem-buildsparsity_heat_matrix_2DQuads.plt");
}

BOOST_AUTO_TEST_CASE( Sparsity2DTris )
{
  Core::instance().environment().options().set("log_level", 4u);

  // Parameters
  Real length            = 5.;
  const Uint nb_segments = 5 ;
  const Uint nb_nodes = (nb_segments+1) * (nb_segments+1);

  // Setup a model
  Model& model = *root.create_component<Model>("Model");
  Domain& domain = model.create_domain("Domain");

  LSS::System& lss = *model.create_component<LSS::System>("LSS");
  lss.options().option("matrix_builder").change_value(std::string("cf3.math.LSS.TrilinosFEVbrMatrix"));

  // Setup mesh
  Mesh& mesh = *domain.create_component<Mesh>("Mesh");
  Tools::MeshGeneration::create_rectangle_tris(mesh, length, length, nb_segments, nb_segments);

  // Setup sparsity
  std::vector<Uint> node_connectivity, starting_indices;
  Handle< List<Uint> > gids = domain.create_component< List<Uint> >("GIDs");
  Handle< List<Uint> > ranks = domain.create_component< List<Uint> >("Ranks");
  Handle< List<Uint> > used_node_map = domain.create_component< List<Uint> >("used_node_map");
  UFEM::build_sparsity(std::vector< Handle<Region> >(1, mesh.topology().handle<Region>()), mesh.geometry_fields(), node_connectivity, starting_indices, *gids, *ranks, *used_node_map);

  PE::CommPattern& comm_pattern = *domain.create_component<PE::CommPattern>("CommPattern");
  comm_pattern.insert("gid",gids->array(),false);
  comm_pattern.setup(Handle<PE::CommWrapper>(comm_pattern.get_child("gid")),ranks->array());

  // Create the LSS
  lss.create(comm_pattern, 1u, node_connectivity, starting_indices);


  // Write the matrix
  lss.matrix()->print("utest-ufem-buildsparsity_heat_matrix_2DTris.plt");
}

// Single block, meshed with the blockmesher
BOOST_AUTO_TEST_CASE( Sparsity3DHexaBlock )
{
  Core::instance().environment().options().set("log_level", 4u);

  // Parameters
  Real length            = 5.;
  const Uint nb_segments = 10;
  const Uint nb_nodes = (nb_segments+1) * (nb_segments+1) * (nb_segments+1);

  // Setup a model
  Model& model = *root.create_component<Model>("Model");
  Domain& domain = model.create_domain("Domain");

  LSS::System& lss = *model.create_component<LSS::System>("LSS");
  lss.options().option("matrix_builder").change_value(std::string("cf3.math.LSS.TrilinosFEVbrMatrix"));

  // Setup mesh
  Mesh& mesh = *domain.create_component<Mesh>("Mesh");

  BlockMesh::BlockArrays& blocks = *domain.create_component<BlockMesh::BlockArrays>("blocks");

  *blocks.create_points(3, 8) << 0.     << 0.     << 0.
                              << length << 0.     << 0.
                              << 0.     << length << 0.
                              << length << length << 0.
                              << 0.     << 0.     << length
                              << length << 0.     << length
                              << 0.     << length << length
                              << length << length << length;

  *blocks.create_blocks(1) << 0 << 1 << 3 << 2 << 4 << 5 << 7 << 6;
  *blocks.create_block_subdivisions() << nb_segments << nb_segments << nb_segments;
  *blocks.create_block_gradings() << 1. << 1. << 1. << 1. << 1. << 1. << 1. << 1. << 1. << 1. << 1. << 1.;

  *blocks.create_patch("bottomWall", 1) << 0 << 2 << 3 << 1;
  *blocks.create_patch("topWall", 1) << 4 << 5 << 7 << 6;
  *blocks.create_patch("side1", 1) << 1 << 3 << 7 << 5;
  *blocks.create_patch("side2", 1) << 0 << 1 << 5 << 4;
  *blocks.create_patch("side3", 1) << 0 << 4 << 6 << 2;
  *blocks.create_patch("side4", 1) << 2 << 6 << 7 << 3;

  blocks.create_mesh(mesh);

  BOOST_CHECK_EQUAL(nb_nodes, mesh.geometry_fields().coordinates().size());

  // Setup sparsity
  std::vector<Uint> node_connectivity, starting_indices;
  Handle< List<Uint> > gids = domain.create_component< List<Uint> >("GIDs");
  Handle< List<Uint> > ranks = domain.create_component< List<Uint> >("Ranks");
  Handle< List<Uint> > used_node_map = domain.create_component< List<Uint> >("used_node_map");
  UFEM::build_sparsity(std::vector< Handle<Region> >(1, mesh.topology().handle<Region>()), mesh.geometry_fields(), node_connectivity, starting_indices, *gids, *ranks, *used_node_map);

  PE::CommPattern& comm_pattern = *domain.create_component<PE::CommPattern>("CommPattern");
  comm_pattern.insert("gid",gids->array(),false);
  comm_pattern.setup(Handle<PE::CommWrapper>(comm_pattern.get_child("gid")),ranks->array());

  // Create the LSS
  lss.create(comm_pattern, 1u, node_connectivity, starting_indices);


  // Write the matrix
  lss.matrix()->print("utest-ufem-buildsparsity_heat_matrix_3DHexaBlock.plt");
}

BOOST_AUTO_TEST_CASE( Sparsity3DHexaChannel )
{
  Core::instance().environment().options().set("log_level", 4u);

  // Parameters
  Real length            = 5.;
  const Uint nb_segments = 4;
  const Uint nb_nodes = (nb_segments+1) * (nb_segments+1) * (nb_segments+1);

  // Setup a model
  Model& model = *root.create_component<Model>("Model");
  Domain& domain = model.create_domain("Domain");

  LSS::System& lss = *model.create_component<LSS::System>("LSS");
  lss.options().option("matrix_builder").change_value(std::string("cf3.math.LSS.TrilinosFEVbrMatrix"));

  // Setup mesh
  Mesh& mesh = *domain.create_component<Mesh>("Mesh");
  BlockMesh::BlockArrays& blocks = *domain.create_component<BlockMesh::BlockArrays>("blocks");
  Tools::MeshGeneration::create_channel_3d(blocks, length, length/8., length, nb_segments, nb_segments/2, nb_segments, 1.);
  blocks.create_mesh(mesh);

  BOOST_CHECK_EQUAL(nb_nodes, mesh.geometry_fields().coordinates().size());

  // Setup sparsity
  std::vector<Uint> node_connectivity, starting_indices;
  Handle< List<Uint> > gids = domain.create_component< List<Uint> >("GIDs");
  Handle< List<Uint> > ranks = domain.create_component< List<Uint> >("Ranks");
  Handle< List<Uint> > used_node_map = domain.create_component< List<Uint> >("used_node_map");
  UFEM::build_sparsity(std::vector< Handle<Region> >(1, mesh.topology().handle<Region>()), mesh.geometry_fields(), node_connectivity, starting_indices, *gids, *ranks, *used_node_map);

  PE::CommPattern& comm_pattern = *domain.create_component<PE::CommPattern>("CommPattern");
  comm_pattern.insert("gid",gids->array(),false);
  comm_pattern.setup(Handle<PE::CommWrapper>(comm_pattern.get_child("gid")),ranks->array());

  // Create the LSS
  lss.create(comm_pattern, 1u, node_connectivity, starting_indices);

  // Write the matrix
  lss.matrix()->print("utest-ufem-buildsparsity_heat_matrix_3DHexaChannel.plt");
}

BOOST_AUTO_TEST_CASE( Heat1DComponent )
{
  Core::instance().environment().options().set("log_level", 4u);

  // Parameters
  Real length            = 5.;
  const Uint nb_segments = 5 ;
  const Uint nb_nodes = nb_segments + 1;

  // Setup a model
  Model& model = *root.create_component<Model>("Model");
  Domain& domain = model.create_domain("Domain");
  UFEM::Solver& solver = *model.create_component<UFEM::Solver>("Solver");

  Handle<UFEM::LSSAction> lss_action(solver.add_direct_solver("cf3.UFEM.LSSAction"));

  // Proto placeholders
  FieldVariable<0, ScalarField> temperature("Temperature", UFEM::Tags::solution());

  // Allowed elements (reducing this list improves compile times)
  boost::mpl::vector1<mesh::LagrangeP1::Line1D> allowed_elements;

  // add the top-level actions (assembly, BC and solve)
  *lss_action
    << create_proto_action
    (
      "Assembly",
      elements_expression
      (
        allowed_elements,
        group
        (
          _A = _0,
          element_quadrature( _A(temperature) += transpose(nabla(temperature)) * nabla(temperature) ),
          lss_action->system_matrix += _A
        )
      )
    )
    << allocate_component<UFEM::BoundaryConditions>("BoundaryConditions")
    << allocate_component<math::LSS::SolveLSS>("SolveLSS")
    << create_proto_action("Increment", nodes_expression(temperature += lss_action->solution(temperature)))
    << create_proto_action("Output", nodes_expression(_cout << "T(" << coordinates(0,0) << ") = " << temperature << "\n"));

  // Setup physics
  model.create_physics("cf3.physics.DynamicModel");

  // Setup mesh
  Mesh& mesh = *domain.create_component<Mesh>("Mesh");
  Tools::MeshGeneration::create_line(mesh, length, nb_segments);

  lss_action->options().set("regions", std::vector<URI>(1, mesh.topology().uri()));
  
  LSS::System& lss = lss_action->create_lss("cf3.math.LSS.TrilinosFEVbrMatrix");

  // Write the matrix
  lss.matrix()->print("utest-ufem-buildsparsity_heat_matrix_1DHeat.plt");
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
