// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for heat-conduction related proto operations"

#include <boost/test/unit_test.hpp>

#define BOOST_PROTO_MAX_ARITY 10                        //explained in boost doc
#ifdef BOOST_MPL_LIMIT_METAFUNCTION_ARITY
 #undef BOOST_MPL_LIMIT_METAFUNCTION_ARITY
 #define BOOST_MPL_LIMIT_METAFUNCTION_ARITY 10
#endif

#include "common/Core.hpp"
#include "common/Environment.hpp"
#include "common/Libraries.hpp"

#include "math/LSS/System.hpp"

#include "mesh/Domain.hpp"
#include "mesh/LagrangeP0/Quad.hpp"
#include "mesh/LagrangeP0/LibLagrangeP0.hpp"
#include <mesh/LagrangeP0/Line.hpp>
#include "mesh/LagrangeP1/Quad2D.hpp"

#include "solver/Model.hpp"

#include "math/LSS/SolveLSS.hpp"

#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"

#include "Tools/MeshGeneration/MeshGeneration.hpp"

#include "mesh/BlockMesh/BlockData.hpp"
#include <mesh/FieldManager.hpp>

#include "UFEM/LSSAction.hpp"
#include "UFEM/Solver.hpp"
#include "UFEM/Tags.hpp"
#include "UFEM/AdjacentCellToFace.hpp"

using namespace cf3;
using namespace cf3::solver;
using namespace cf3::solver::actions;
using namespace cf3::solver::actions::Proto;
using namespace cf3::common;
using namespace cf3::math::Consts;
using namespace cf3::mesh;

using namespace boost;


typedef std::vector<std::string> StringsT;
typedef std::vector<Uint> SizesT;

/// Check close, for testing purposes
inline void
check_close(const Real a, const Real b, const Real threshold)
{
  BOOST_CHECK_CLOSE(a, b, threshold);
}

static boost::proto::terminal< void(*)(Real, Real, Real) >::type const _check_close = {&check_close};

struct NeumannFixture
{
  NeumannFixture() :
    root( Core::instance().root() )
  {
  }

  Component& root;

};

BOOST_FIXTURE_TEST_SUITE( NeumannSuite, NeumannFixture )

BOOST_AUTO_TEST_CASE( InitMPI )
{
  common::PE::Comm::instance().init(boost::unit_test::framework::master_test_suite().argc, boost::unit_test::framework::master_test_suite().argv);
  BOOST_CHECK_EQUAL(common::PE::Comm::instance().size(), 1);
}

BOOST_AUTO_TEST_CASE( NeumannTest )
{
  Core::instance().environment().options().set("log_level", 4u);
  //Core::instance().environment().options().set("exception_aborts", true);

  // Setup a model
  Model& model = *root.create_component<Model>("Model");
  Domain& domain = model.create_domain("Domain");
  physics::PhysModel& physics = model.create_physics("cf3.UFEM.NavierStokesPhysics");
  // Add a UFEM solver, the top layer for the simulation
  Handle<UFEM::Solver> solver(model.create_solver("cf3.UFEM.Solver").handle());
  // This steady heat conduction solver will be used on the bottom region:
  Handle<UFEM::LSSAction> hc_bottom(solver->add_direct_solver("cf3.UFEM.HeatConductionSteady"));
  // coupling_bc will hold the actions related to the heat transfer coupling condition between the regions
  Handle<solver::ActionDirector> coupling_bc = solver->create_component<solver::ActionDirector>("heat_bc");
  // hc_top solves the heat conduction equation in the top part of the mesh
  Handle<UFEM::LSSAction> hc_top(solver->add_direct_solver("cf3.UFEM.HeatConductionSteady"));
  
  // Handles to boundary conditions
  Handle<UFEM::BoundaryConditions> bc_bot(hc_bottom->get_child("BoundaryConditions"));
  Handle<UFEM::BoundaryConditions> bc_top(hc_top->get_child("BoundaryConditions"));
  

  ////////////////////////////////////////
  // Special-purpose boundary condition //
  ////////////////////////////////////////
  
  // Represents the temperature field, as calculated
  FieldVariable<0, ScalarField> T("Temperature", "solution");
  // Represents the gradient of the temperature, to be stored in an (element based) field
  FieldVariable<1, VectorField> GradT("GradT", "element_fields", Core::instance().libraries().library<mesh::LagrangeP0::LibLagrangeP0>()->library_namespace());

  // For quads, the center is at mapped coordinates (0,0)
  RealVector2 center; center.setZero();

  // Calculate the gradient, at the cell centroid:
  // nabla(T, center) is the shape function gradient matrix evaluated at the element center
  // T are the nodal values for the temperature
  boost::shared_ptr<Expression> grad_t_expr = elements_expression
  (
    boost::mpl::vector2<mesh::LagrangeP0::Quad, mesh::LagrangeP1::Quad2D>(),
    GradT = nabla(T, center)*nodal_values(T)
  );

  // Register the variables, making sure a field description for GradT exists
  grad_t_expr->register_variables(physics);

  // Add an action to do the gradient calculation
  (*coupling_bc ) << create_proto_action("GradT", grad_t_expr);
  
  // This will create values at the boundary starting from the cell next to the wall
  common::Action& create_boundary_gradient = *coupling_bc->create_component<UFEM::AdjacentCellToFace>("CreateBoundaryGradient");
  // Must be the tag for the field we want to copy, in this case GradT:
  create_boundary_gradient.options().set("field_tag", std::string("element_fields"));
  
  // Add the neumann boundary condition, which is expressed using a proto action:
  Component& neumann_bc = bc_top->add_component(create_proto_action("NeumannHeat", elements_expression
  (
    boost::mpl::vector2<mesh::LagrangeP0::Line, mesh::LagrangeP1::Line2D>(), // Valid for surface element types
    hc_top->system_rhs(T) += integral<1>(transpose(N(T))*GradT*normal) // Classical Neumann condition formulation for finite elements
  )));

  // Build the mesh
  Handle<BlockMesh::BlockArrays> blocks = domain.create_component<BlockMesh::BlockArrays>("blocks");
  (*blocks->create_points(2, 6)) << 0. << 0.
                                << 1. << 0.
                                << 0. << 0.5
                                << 1. << 0.5
                                << 0. << 1.
                                << 1. << 1.;

  (*blocks->create_blocks(2)) << 0 << 1 << 3 << 2
                             << 2 << 3 << 5 << 4;

  (*blocks->create_block_subdivisions()) << 40 << 20 << 40 << 20;
  (*blocks->create_block_gradings()) << 1. << 1. << 1. << 1. << 1. << 1. << 1. << 1.;

  *blocks->create_patch("left", 2) << 2 << 0 << 4 << 2;
  *blocks->create_patch("right", 2) << 1 << 3 << 3 << 5;
  *blocks->create_patch("top", 1) << 5 << 4;
  *blocks->create_patch("bottom", 1) << 0 << 1;

  // Setup a different region for each of the two blocks
  std::vector<std::string> block_regions(2); block_regions[0] = "solid_bottom"; block_regions[1] = "solid_top";
  blocks->options().set("block_regions", block_regions);

  Handle<Mesh> mesh = domain.create_component<Mesh>("Mesh");
  blocks->create_mesh(*mesh);
  
  // Set up the regular bottom and top solvers
  hc_bottom->options().set("regions", std::vector<URI>(1, mesh->access_component("topology/solid_bottom")->uri()));
  hc_top->options().set("regions", std::vector<URI>(1, mesh->access_component("topology/solid_top")->uri()));

  math::LSS::System& bot_lss = hc_bottom->create_lss("cf3.math.LSS.TrilinosFEVbrMatrix");
  math::LSS::System& top_lss = hc_top->create_lss("cf3.math.LSS.TrilinosFEVbrMatrix");
  
  bc_bot->options().set("regions", std::vector<URI>(1, mesh->topology().uri()));
  bc_bot->add_constant_bc("bottom", "Temperature")->options().set("value", 10.);
  bc_bot->add_constant_bc("region_bnd_solid_bottom_solid_top", "Temperature")->options().set("value", 50.);

  bc_top->options().set("regions", std::vector<URI>(1, mesh->topology().uri()));
  bc_top->add_constant_bc("top", "Temperature")->options().set("value", 10.);
  
  
  // Set up the regions (needs to be done after mesh creation)
  coupling_bc->get_child("GradT")->options().set("regions", std::vector<URI>(1, mesh->access_component("topology/solid_bottom")->uri()));
  neumann_bc.options().set("regions", std::vector<URI>(1, mesh->access_component("topology/region_bnd_solid_top_solid_bottom")->uri()));
  create_boundary_gradient.options().set("regions", std::vector<URI>(1, mesh->access_component("topology/region_bnd_solid_top_solid_bottom")->uri()));

  // Run the simulation and save the mesh
  model.simulate();
  top_lss.rhs()->print_native(std::cout);
  domain.write_mesh("utest-neumann.msh");
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
