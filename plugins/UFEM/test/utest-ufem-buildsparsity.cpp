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
#include "common/Root.hpp"

#include "common/PE/CommPattern.hpp"

#include "math/LSS/System.hpp"

#include "mesh/Domain.hpp"
#include "mesh/LagrangeP1/Line1D.hpp"

#include "Solver/CModel.hpp"

#include "Solver/Actions/Proto/CProtoAction.hpp"
#include "Solver/Actions/Proto/Expression.hpp"

#include "Tools/MeshGeneration/MeshGeneration.hpp"

#include "UFEM/LinearSolver.hpp"
#include "UFEM/SparsityBuilder.hpp"
#include "UFEM/Tags.hpp"

using namespace cf3;
using namespace cf3::Solver;
using namespace cf3::Solver::Actions;
using namespace cf3::Solver::Actions::Proto;
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

  Root& root;
};

BOOST_FIXTURE_TEST_SUITE( UFEMBuildSparsitySuite, UFEMBuildSparsityFixture )

BOOST_AUTO_TEST_CASE( InitMPI )
{
  common::PE::Comm::instance().init(boost::unit_test::framework::master_test_suite().argc, boost::unit_test::framework::master_test_suite().argv);
  BOOST_CHECK_EQUAL(common::PE::Comm::instance().size(), 1);
}

BOOST_AUTO_TEST_CASE( Sparsity1D )
{
  Core::instance().environment().configure_option("log_level", 4u);

  // Parameters
  Real length            = 5.;
  const Uint nb_segments = 5 ;
  const Uint nb_nodes = nb_segments + 1;

  // Setup a model
  CModel& model = root.create_component<CModel>("Model");
  Domain& domain = model.create_domain("Domain");

  LSS::System& lss = model.create_component<LSS::System>("LSS");
  lss.option("solver").change_value(std::string("Trilinos"));

  // Setup mesh
  Mesh& mesh = domain.create_component<Mesh>("Mesh");
  Tools::MeshGeneration::create_line(mesh, length, nb_segments);

  // Setup sparsity
  std::vector<Uint> node_connectivity, starting_indices;
  UFEM::build_sparsity(mesh, node_connectivity, starting_indices);

  // Check result
  BOOST_CHECK_EQUAL(starting_indices[0], 0u);
  for(Uint i = 2; i != nb_nodes; ++i)
    BOOST_CHECK_EQUAL(starting_indices[i] - starting_indices[i-1], 3);

  // Create the LSS
  lss.create(mesh.geometry().comm_pattern(), 1u, node_connectivity, starting_indices);


  // Write the matrix
  lss.matrix()->print("utest-ufem-buildsparsity_heat_matrix_1D.plt");
}

BOOST_AUTO_TEST_CASE( Sparsity2DQuads )
{
  Core::instance().environment().configure_option("log_level", 4u);

  // Parameters
  Real length            = 5.;
  const Uint nb_segments = 5 ;
  const Uint nb_nodes = (nb_segments+1) * (nb_segments+1);

  // Setup a model
  CModel& model = root.create_component<CModel>("Model");
  Domain& domain = model.create_domain("Domain");

  LSS::System& lss = model.create_component<LSS::System>("LSS");
  lss.option("solver").change_value(std::string("Trilinos"));

  // Setup mesh
  Mesh& mesh = domain.create_component<Mesh>("Mesh");
  Tools::MeshGeneration::create_rectangle(mesh, length, length, nb_segments, nb_segments);

  // Setup sparsity
  std::vector<Uint> node_connectivity, starting_indices;
  UFEM::build_sparsity(mesh, node_connectivity, starting_indices);

  // Create the LSS
  lss.create(mesh.geometry().comm_pattern(), 1u, node_connectivity, starting_indices);


  // Write the matrix
  lss.matrix()->print("utest-ufem-buildsparsity_heat_matrix_2DQuads.plt");
}

BOOST_AUTO_TEST_CASE( Sparsity2DTris )
{
  Core::instance().environment().configure_option("log_level", 4u);

  // Parameters
  Real length            = 5.;
  const Uint nb_segments = 5 ;
  const Uint nb_nodes = (nb_segments+1) * (nb_segments+1);

  // Setup a model
  CModel& model = root.create_component<CModel>("Model");
  Domain& domain = model.create_domain("Domain");

  LSS::System& lss = model.create_component<LSS::System>("LSS");
  lss.option("solver").change_value(std::string("Trilinos"));

  // Setup mesh
  Mesh& mesh = domain.create_component<Mesh>("Mesh");
  Tools::MeshGeneration::create_rectangle_tris(mesh, length, length, nb_segments, nb_segments);

  // Setup sparsity
  std::vector<Uint> node_connectivity, starting_indices;
  UFEM::build_sparsity(mesh, node_connectivity, starting_indices);

  // Create the LSS
  lss.create(mesh.geometry().comm_pattern(), 1u, node_connectivity, starting_indices);


  // Write the matrix
  lss.matrix()->print("utest-ufem-buildsparsity_heat_matrix_2DTris.plt");
}

// Single block, meshed with the blockmesher
BOOST_AUTO_TEST_CASE( Sparsity3DHexaBlock )
{
  Core::instance().environment().configure_option("log_level", 4u);

  // Parameters
  Real length            = 5.;
  const Uint nb_segments = 10;
  const Uint nb_nodes = (nb_segments+1) * (nb_segments+1) * (nb_segments+1);

  // Setup a model
  CModel& model = root.create_component<CModel>("Model");
  Domain& domain = model.create_domain("Domain");

  LSS::System& lss = model.create_component<LSS::System>("LSS");
  lss.option("solver").change_value(std::string("Trilinos"));

  // Setup mesh
  Mesh& mesh = domain.create_component<Mesh>("Mesh");
  BlockMesh::BlockData& blocks = domain.create_component<BlockMesh::BlockData>("blocks");
  blocks.scaling_factor = 1.;
  blocks.dimension = 3;
  blocks.points += list_of(0.    )(0.    )(0.    )
                 , list_of(length)(0.    )(0.    )
                 , list_of(0.    )(length)(0.    )
                 , list_of(length)(length)(0.    )
                 , list_of(0.    )(0.    )(length)
                 , list_of(length)(0.    )(length)
                 , list_of(0.    )(length)(length)
                 , list_of(length)(length)(length);
  blocks.block_points += list_of(0)(1)(3)(2)(4)(5)(7)(6);
  blocks.block_subdivisions += list_of(nb_segments)(nb_segments)(nb_segments);
  blocks.block_gradings += list_of(1.)(1.)(1.)(1.)(1.)(1.)(1.)(1.)(1.)(1.)(1.)(1.);
  blocks.block_distribution += 0, 1;
  blocks.patch_names += "bottomWall", "topWall", "side1", "side2", "side3", "side4";
  blocks.patch_types += "wall" , "wall"   , "wall", "wall", "wall", "wall";
  blocks.patch_points += list_of(0)(1)(3)(2),
                         list_of(4)(5)(7)(6),
                         list_of(1)(5)(7)(3),
                         list_of(0)(4)(5)(1),
                         list_of(6)(4)(0)(2),
                         list_of(2)(3)(7)(6);
  BlockMesh::build_mesh(blocks, mesh);

  BOOST_CHECK_EQUAL(nb_nodes, mesh.geometry().coordinates().size());

  // Setup sparsity
  std::vector<Uint> node_connectivity, starting_indices;
  UFEM::build_sparsity(mesh, node_connectivity, starting_indices);

  // Create the LSS
  lss.create(mesh.geometry().comm_pattern(), 1u, node_connectivity, starting_indices);


  // Write the matrix
  lss.matrix()->print("utest-ufem-buildsparsity_heat_matrix_3DHexaBlock.plt");
}

BOOST_AUTO_TEST_CASE( Sparsity3DHexaChannel )
{
  Core::instance().environment().configure_option("log_level", 4u);

  // Parameters
  Real length            = 5.;
  const Uint nb_segments = 4;
  const Uint nb_nodes = (nb_segments+1) * (nb_segments+1) * (nb_segments+1);

  // Setup a model
  CModel& model = root.create_component<CModel>("Model");
  Domain& domain = model.create_domain("Domain");

  LSS::System& lss = model.create_component<LSS::System>("LSS");
  lss.option("solver").change_value(std::string("Trilinos"));

  // Setup mesh
  Mesh& mesh = domain.create_component<Mesh>("Mesh");
  BlockMesh::BlockData& blocks = domain.create_component<BlockMesh::BlockData>("blocks");
  Tools::MeshGeneration::create_channel_3d(blocks, length, length/8., length, nb_segments, nb_segments/2, nb_segments, 1.);
  BlockMesh::build_mesh(blocks, mesh);

  BOOST_CHECK_EQUAL(nb_nodes, mesh.geometry().coordinates().size());

  // Setup sparsity
  std::vector<Uint> node_connectivity, starting_indices;
  UFEM::build_sparsity(mesh, node_connectivity, starting_indices);

  // Create the LSS
  lss.create(mesh.geometry().comm_pattern(), 1u, node_connectivity, starting_indices);

  // Write the matrix
  lss.matrix()->print("utest-ufem-buildsparsity_heat_matrix_3DHexaChannel.plt");
}

BOOST_AUTO_TEST_CASE( Heat1DComponent )
{
  Core::instance().environment().configure_option("log_level", 4u);

  // Parameters
  Real length            = 5.;
  const Uint nb_segments = 5 ;
  const Uint nb_nodes = nb_segments + 1;

  // Setup a model
  CModel& model = root.create_component<CModel>("Model");
  Domain& domain = model.create_domain("Domain");
  UFEM::LinearSolver& solver = model.create_component<UFEM::LinearSolver>("Solver");

  // Proto placeholders
  MeshTerm<0, ScalarField> temperature("Temperature", UFEM::Tags::solution());

  // Allowed elements (reducing this list improves compile times)
  boost::mpl::vector1<mesh::LagrangeP1::Line1D> allowed_elements;

  // add the top-level actions (assembly, BC and solve)
  solver
    << create_proto_action
    (
      "Assembly",
      elements_expression
      (
        allowed_elements,
        group <<
        (
          _A = _0,
          element_quadrature( _A(temperature) += transpose(nabla(temperature)) * nabla(temperature) ),
          solver.system_matrix += _A
        )
      )
    )
    << solver.boundary_conditions()
    << solver.solve_action()
    << create_proto_action("Increment", nodes_expression(temperature += solver.solution(temperature)))
    << create_proto_action("Output", nodes_expression(_cout << "T(" << coordinates(0,0) << ") = " << temperature << "\n"));

  // Setup physics
  model.create_physics("CF.Physics.DynamicModel");

  // Setup mesh
  Mesh& mesh = domain.create_component<Mesh>("Mesh");
  Tools::MeshGeneration::create_line(mesh, length, nb_segments);

  LSS::System& lss = model.create_component<LSS::System>("LSS");
  lss.option("solver").change_value(std::string("Trilinos"));
  solver.configure_option("lss", lss.uri());

  // Write the matrix
  lss.matrix()->print("utest-ufem-buildsparsity_heat_matrix_1DHeat.plt");
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
