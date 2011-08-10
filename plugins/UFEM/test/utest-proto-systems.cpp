// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for heat-conduction related proto operations"

#include <boost/test/unit_test.hpp>

#include "Common/Core.hpp"
#include "Common/CEnv.hpp"
#include "Common/CRoot.hpp"

#include "Mesh/CDomain.hpp"

#include "Solver/CModelUnsteady.hpp"
#include "Solver/CTime.hpp"

#include "Solver/Actions/Proto/CProtoAction.hpp"
#include "Solver/Actions/Proto/Expression.hpp"

#include "Tools/MeshGeneration/MeshGeneration.hpp"

#include "UFEM/LinearSolverUnsteady.hpp"
#include "UFEM/TimeLoop.hpp"

using namespace CF;
using namespace CF::Solver;
using namespace CF::Solver::Actions;
using namespace CF::Solver::Actions::Proto;
using namespace CF::Common;
using namespace CF::Math::Consts;
using namespace CF::Mesh;

using namespace boost;

typedef std::vector<std::string> StringsT;
typedef std::vector<Uint> SizesT;

BOOST_AUTO_TEST_SUITE( ProtoSystemSuite )

BOOST_AUTO_TEST_CASE( ProtoSystem )
{
  const Real length = 5.;
  RealVector outside_temp(2);
  outside_temp << 1., 1;
  const RealVector2 initial_temp(100., 200.);
  const Uint nb_segments = 10;
  const Real end_time = 0.5;
  const Real dt = 0.1;
  const boost::proto::literal<RealVector> alpha(RealVector2(1., 2.));

  // Setup a model
  CModelUnsteady& model = Core::instance().root().create_component<CModelUnsteady>("Model");
  CDomain& domain = model.create_domain("Domain");
  UFEM::LinearSolverUnsteady& solver = model.create_component<UFEM::LinearSolverUnsteady>("Solver");
  model.create_physics("CF.Physics.DynamicModel");

  // Setup mesh
  CMesh& mesh = domain.create_component<CMesh>("Mesh");
  Tools::MeshGeneration::create_rectangle(mesh, length, 0.5*length, 2*nb_segments, nb_segments);

  // Linear system setup (TODO: sane default config for this, so this can be skipped)
  CEigenLSS& lss = model.create_component<CEigenLSS>("LSS");
  lss.set_config_file(boost::unit_test::framework::master_test_suite().argv[1]);
  solver.solve_action().configure_option("lss", lss.uri());

  // Proto placeholders
  MeshTerm<0, VectorField> v("VectorVariable", "v");

  // Allowed elements (reducing this list improves compile times)
  boost::mpl::vector1<Mesh::SF::Quad2DLagrangeP1> allowed_elements;

  // build up the solver out of different actions
  solver
    << create_proto_action("Initialize", nodes_expression(v = initial_temp))
    <<
    (
      solver.create_component<UFEM::TimeLoop>("TimeLoop")
      << solver.zero_action()
      << create_proto_action
      (
        "Assembly",
        elements_expression // assembly
        (
          allowed_elements,
          group <<
          (
            _A = _0, _T = _0,
            element_quadrature <<
            (
              _A(v[_i], v[_i]) += transpose(nabla(v)) * alpha[_i] * nabla(v),
              _T(v[_i], v[_i]) += solver.invdt() * (transpose(N(v)) * N(v))
            ),
            solver.system_matrix += _T + 0.5 * _A,
            solver.system_rhs -= _A * _b
          )
        )
      )
      << solver.boundary_conditions()
      << solver.solve_action()
      << create_proto_action("Increment", nodes_expression(v += solver.solution(v)))
    );

  // Creating the physics here makes sure everything is up-to-date
  model.create_physics("CF.Physics.DynamicModel");
  solver.mesh_loaded(mesh);

  solver.boundary_conditions().add_constant_bc("left", "VectorVariable", outside_temp);
  solver.boundary_conditions().add_constant_bc("right", "VectorVariable", outside_temp);
  solver.boundary_conditions().add_constant_bc("bottom", "VectorVariable", outside_temp);
  solver.boundary_conditions().add_constant_bc("top", "VectorVariable", outside_temp);

  // Configure timings
  CTime& time = model.create_time();
  time.configure_option("time_step", dt);
  time.configure_option("end_time", end_time);

  // Run the solver
  model.simulate();

  // Write result
  domain.write_mesh(URI("systems.msh"));
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
