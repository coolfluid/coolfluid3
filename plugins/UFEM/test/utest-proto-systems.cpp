// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for heat-conduction related proto operations"

#include <boost/test/unit_test.hpp>

#include "Solver/Actions/Proto/Expression.hpp"
#include "Solver/CTime.hpp"

#include "Common/Core.hpp" 
#include "Common/CRoot.hpp"

#include "Tools/MeshGeneration/MeshGeneration.hpp"

#include "UFEM/LinearProblem.hpp"
#include "UFEM/UnsteadyModel.hpp"

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

BOOST_AUTO_TEST_CASE( ProtoSystem )
{
  const Real length = 5.;
  RealVector outside_temp(2);
  outside_temp << 1., 1;
  const RealVector2 initial_temp(100., 200.);
  const Uint nb_segments = 10;
  const Real end_time = 0.5;
  const Real dt = 0.1;
  const boost::proto::literal<RealVector2> alpha(RealVector2(1., 2.));
  
  // Setup a UFEM model
  UFEM::UnsteadyModel& model = Core::instance().root().create_component<UFEM::UnsteadyModel>("Model");
  CMesh& mesh = model.get_child("Mesh").as_type<CMesh>();
  Tools::MeshGeneration::create_rectangle(mesh, length, 0.5*length, 2*nb_segments, nb_segments);

  // Linear system setup (TODO: sane default config for this, so this can be skipped)
  CEigenLSS& lss = model.create_component<CEigenLSS>("LSS");
  lss.set_config_file(boost::unit_test::framework::master_test_suite().argv[1]);
  model.problem().solve_action().configure_option("lss", lss.uri());
  
  // Proto placeholders
  MeshTerm<0, VectorField> v("VectorVariable", "v");
  
  // shorthand for the problem and boundary conditions
  UFEM::LinearProblem& p = model.problem();
  UFEM::BoundaryConditions& bc = p.boundary_conditions();
  
  // Allowed elements (reducing this list improves compile times)
  boost::mpl::vector1<Mesh::SF::Quad2DLagrangeP1> allowed_elements;

  // build up the solver out of different actions
  model << model.add_action("Initialize", nodes_expression(v = initial_temp)) <<
  ( // Time loop
    p << p.add_action
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
            _T(v[_i], v[_i]) += model.invdt() * (transpose(N(v)) * N(v))
          ),
          p.system_matrix += _T + 0.5 * _A,
          p.system_rhs -= _A * _b
        )
      )
    ) <<
    ( // boundary conditions
      bc
      << bc.add_constant_bc("left", "VectorVariable", outside_temp)
      << bc.add_constant_bc("right", "VectorVariable", outside_temp)
      << bc.add_constant_bc("bottom", "VectorVariable", outside_temp)
      << bc.add_constant_bc("top", "VectorVariable", outside_temp)
    )
    << p.solve_action() // solve
    << p.add_action("Increment", nodes_expression(v += p.solution(v))) // increment solution
  );
  
  // Configure timings
  model.time().configure_option("time_step", dt);
  model.time().configure_option("end_time", end_time);
      
  // Run the solver
  model.execute();
  
  // Write result
  URI output_file("systems.msh");
  model.configure_option("output_file", output_file);
  SignalArgs a;
  model.signal_write_mesh(a);
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
