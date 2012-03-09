// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for heat-conduction related proto operations"

#include <boost/test/unit_test.hpp>

#define BOOST_PROTO_MAX_ARITY 10
#ifdef BOOST_MPL_LIMIT_METAFUNCTION_ARITY
 #undef BOOST_MPL_LIMIT_METAFUNCTION_ARITY
 #define BOOST_MPL_LIMIT_METAFUNCTION_ARITY 10
#endif

#include "common/Core.hpp"
#include "common/Environment.hpp"

#include "mesh/Domain.hpp"

#include "solver/ModelUnsteady.hpp"
#include "solver/Time.hpp"

#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"
#include "solver/actions/Iterate.hpp"
#include "solver/actions/CriterionTime.hpp"
#include "solver/actions/AdvanceTime.hpp"

#include "Tools/MeshGeneration/MeshGeneration.hpp"
#include "mesh/MeshGenerator.hpp"

#include "UFEM/LinearSolverUnsteady.hpp"
#include "UFEM/Tags.hpp"
#include "solver/actions/ZeroLSS.hpp"
#include "solver/actions/SolveLSS.hpp"

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

BOOST_AUTO_TEST_SUITE( ProtoSystemSuite )

BOOST_AUTO_TEST_CASE( InitMPI )
{
  common::PE::Comm::instance().init(boost::unit_test::framework::master_test_suite().argc, boost::unit_test::framework::master_test_suite().argv);
  BOOST_CHECK_EQUAL(common::PE::Comm::instance().size(), 1);
}

BOOST_AUTO_TEST_CASE( ProtoSystem )
{
  const Real length = 5.;
  std::vector<Real> outside_temp(2, 1.);
  RealVector initial_temp(2); initial_temp << 100., 200.;
  const Uint nb_segments = 10;
  const Real end_time = 0.5;
  const Real dt = 0.1;
  const boost::proto::literal<RealVector> alpha(RealVector2(1., 2.));

  // Setup a model
  ModelUnsteady& model = *Core::instance().root().create_component<ModelUnsteady>("Model");
  Domain& domain = model.create_domain("Domain");
  UFEM::LinearSolverUnsteady& solver = *model.create_component<UFEM::LinearSolverUnsteady>("Solver");

  // Linear system setup (TODO: sane default config for this, so this can be skipped)
  math::LSS::System& lss = *model.create_component<math::LSS::System>("LSS");
  lss.options().configure_option("solver", std::string("Trilinos"));
  solver.options().configure_option("lss", lss.handle<math::LSS::System>());
  
  boost::shared_ptr<solver::actions::Iterate> time_loop = allocate_component<solver::actions::Iterate>("TimeLoop");
  time_loop->create_component<solver::actions::CriterionTime>("CriterionTime");

  // Proto placeholders
  MeshTerm<0, VectorField> v("VectorVariable", UFEM::Tags::solution());

  // Allowed elements (reducing this list improves compile times)
  boost::mpl::vector1<mesh::LagrangeP1::Quad2D> allowed_elements;

  // BCs
  boost::shared_ptr<UFEM::BoundaryConditions> bc = allocate_component<UFEM::BoundaryConditions>("BoundaryConditions");

  // build up the solver out of different actions
  solver
    << create_proto_action("Initialize", nodes_expression(v = initial_temp))
    <<
    (
      time_loop
      << allocate_component<solver::actions::ZeroLSS>("ZeroLSS")
      << create_proto_action
      (
        "Assembly",
        elements_expression // assembly
        (
          allowed_elements,
          group
          (
            _A = _0, _T = _0,
            element_quadrature
            (
              _A(v[_i], v[_i]) += transpose(nabla(v)) * alpha[_i] * nabla(v),
              _T(v[_i], v[_i]) += solver.invdt() * (transpose(N(v)) * N(v))
            ),
            solver.system_matrix += _T + 0.5 * _A,
            solver.system_rhs += -(_A * _b)
          )
        )
      )
      << bc
      << allocate_component<solver::actions::SolveLSS>("SolveLSS")
      << create_proto_action("Increment", nodes_expression(v += solver.solution(v)))
      << allocate_component<solver::actions::AdvanceTime>("AdvanceTime")
    );

  // Setup physics
  model.create_physics("cf3.physics.DynamicModel");

  // Setup mesh
  // Mesh& mesh = *domain.create_component<Mesh>("Mesh");
  // Tools::MeshGeneration::create_rectangle(mesh, length, 0.5*length, 2*nb_segments, nb_segments);
  boost::shared_ptr<MeshGenerator> create_rectangle = build_component_abstract_type<MeshGenerator>("cf3.mesh.SimpleMeshGenerator","create_line");
  create_rectangle->options().configure_option("mesh",domain.uri()/"Mesh");
  std::vector<Real> lengths(2);     lengths[XX] = length;            lengths[YY]  = 0.5*length;
  std::vector<Uint> nb_cells(2);    nb_cells[XX] = 2*nb_segments;    nb_cells[YY] = nb_segments;
  create_rectangle->options().configure_option("lengths",lengths);
  create_rectangle->options().configure_option("nb_cells",nb_cells);
  Mesh& mesh = create_rectangle->generate();



  lss.matrix()->options().configure_option("settings_file", std::string(boost::unit_test::framework::master_test_suite().argv[1]));

  bc->add_constant_bc("left", "VectorVariable", outside_temp);
  bc->add_constant_bc("right", "VectorVariable", outside_temp);
  bc->add_constant_bc("bottom", "VectorVariable", outside_temp);
  bc->add_constant_bc("top", "VectorVariable", outside_temp);

  // Configure timings
  Time& time = model.create_time();
  time.options().configure_option("time_step", dt);
  time.options().configure_option("end_time", end_time);

  // Run the solver
  model.simulate();

  // Write result
  domain.create_component("VTKwriter", "cf3.mesh.VTKXML.Writer");
  domain.write_mesh(URI("systems.pvtu"));
};

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
