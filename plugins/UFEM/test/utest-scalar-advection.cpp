// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.
                                                                                     // boost UTF (from boost-doc:):
#define BOOST_TEST_DYN_LINK                                                          // To build/use dynamic library.
#define BOOST_TEST_MODULE "Test module for heat-conduction related proto operations" // To generate the test module initialization function, which uses the defined value to name the master
                                                                                     // test suite. For dynamic library variant default function main() implementation is generated as well
#include <boost/test/unit_test.hpp>

#define BOOST_PROTO_MAX_ARITY 10                                                     // Controls the maximum number of child nodes an expression may have.
#ifdef BOOST_MPL_LIMIT_METAFUNCTION_ARITY                                            // Is an overridable configuration macro regulating the maximum supported arity of metafunctions and
 #undef BOOST_MPL_LIMIT_METAFUNCTION_ARITY                                           // metafunction classes.
 #define BOOST_MPL_LIMIT_METAFUNCTION_ARITY 10
#endif

#include "common/Core.hpp"
#include "common/Environment.hpp"

#include "math/LSS/System.hpp"

#include "mesh/Domain.hpp"

#include "mesh/LagrangeP1/Line1D.hpp"
#include "solver/ModelUnsteady.hpp"

#include "solver/actions/AdvanceTime.hpp"
#include "solver/actions/CriterionTime.hpp"
#include "solver/actions/Iterate.hpp"
#include "solver/Time.hpp"
#include "math/LSS/SolveLSS.hpp"
#include "math/LSS/ZeroLSS.hpp"

#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"

#include "Tools/MeshGeneration/MeshGeneration.hpp"

#include "mesh/SimpleMeshGenerator.hpp"

#include "UFEM/LSSActionUnsteady.hpp"
#include "UFEM/Solver.hpp"
#include "UFEM/Tags.hpp"

#include "UFEM/SUPG.hpp"

using namespace cf3;
using namespace cf3::solver;
using namespace cf3::solver::actions;
using namespace cf3::solver::actions::Proto;
using namespace cf3::common;
using namespace cf3::math::Consts;
using namespace cf3::mesh;

using boost::proto::lit;


typedef std::vector<std::string> StringsT;
typedef std::vector<Uint> SizesT;

/// Check close, for testing purposes
inline void
check_close(const Real a, const Real b, const Real threshold)
{
  BOOST_CHECK_CLOSE(a, b, threshold);
}

static boost::proto::terminal< void(*)(Real, Real, Real) >::type const _check_close = {&check_close};

struct ProtoHeatFixture
{
  ProtoHeatFixture() :
    length(1.),
    Pe(0.0001),
    Pe2(100.),
    left_temp(1.),
    right_temp(0.),
    nb_segments(10),
    alpha(1.),


    root( Core::instance().root() )
  {
  }

  Component& root;

  /// Write the analytical solution
  void set_analytical_solution(Region& region, const std::string& field_name, const std::string& var_name)
  {
    FieldVariable<0, ScalarField > Temp(field_name, var_name);

      // Zero the field
      for_each_node
      (
        region,
            Temp = (_exp(Pe2*coordinates[0])-1.)/(_exp(Pe2)-1.) + 1.
      );

    }

  const Real length;
  const Real Pe;
  const Real Pe2;
  const Uint nb_segments;
  const Real alpha;
  const Real left_temp;
  const Real right_temp;

  Real t;

};


BOOST_FIXTURE_TEST_SUITE( ProtoHeatSuite, ProtoHeatFixture )

BOOST_AUTO_TEST_CASE( InitMPI )
{
 common::PE::Comm::instance().init(boost::unit_test::framework::master_test_suite().argc, boost::unit_test::framework::master_test_suite().argv);
 BOOST_CHECK_EQUAL(common::PE::Comm::instance().size(), 1);
}

BOOST_AUTO_TEST_CASE( Heat1DComponent )
{
  Core::instance().environment().options().set("log_level", 4u);

  // Parameters
  Real length            = 1.;
  const Uint nb_segments = 100 ;

  // Setup a model
  ModelUnsteady& model = *root.create_component<ModelUnsteady>("Model");
  Domain& domain = model.create_domain("Domain");
  UFEM::Solver& solver = *model.create_component<UFEM::Solver>("Solver");
  Handle<UFEM::LSSActionUnsteady> lss_action(solver.add_unsteady_solver("cf3.UFEM.LSSActionUnsteady"));
  Handle<common::ActionDirector> ic(solver.get_child("InitialConditions"));

  // Proto placeholders
  FieldVariable<0, ScalarField> T("Temperature", UFEM::Tags::solution());
  FieldVariable<1, VectorField> u_adv("AdvectionVelocity","linearized_velocity");
  FieldVariable<2, ScalarField> temperature_analytical("TemperatureAnalytical", UFEM::Tags::source_terms());
  FieldVariable<3, ScalarField> nu_eff("EffectiveViscosity", "navier_stokes_viscosity");

  PhysicsConstant nu("kinematic_viscosity");

  const Real alpha = 1;

  Real tau_su;

  // Allowed elements (reducing this list improves compile times)
  boost::mpl::vector1<mesh::LagrangeP1::Line1D> allowed_elements;

  // BCs
  boost::shared_ptr<UFEM::BoundaryConditions> bc = allocate_component<UFEM::BoundaryConditions>("BoundaryConditions");

  RealVector initial_u(1); initial_u.setConstant(-10.);

  // add the top-level actions (assembly, BC and solve)
  *ic << create_proto_action("Initialize", nodes_expression(group(T = 0., u_adv = initial_u, nu_eff = nu)));
  *lss_action
    << allocate_component<math::LSS::ZeroLSS>("ZeroLSS")
    << create_proto_action
       (
         "Assembly",
         elements_expression
         (
           allowed_elements,
           group
           (
             _A = _0, _T = _0,
             UFEM::compute_tau(u_adv, nu_eff, lit(tau_su)),
             element_quadrature( _A(T) += transpose(N(T)) * u_adv * nabla(T) + tau_su * transpose(u_adv*nabla(T))  * u_adv * nabla(T) +  alpha * transpose(nabla(T)) * nabla(T) ,
                   _T(T,T) +=  transpose(N(T) + tau_su * u_adv * nabla(T)) * N(T) ),
                   lss_action->system_matrix += lss_action->invdt() * _T + 1.0 * _A,
                   lss_action->system_rhs += -_A * _x
           )
         )
       )
  << bc
  << allocate_component<math::LSS::SolveLSS>("SolveLSS")
  << create_proto_action("Increment", nodes_expression(T += lss_action->solution(T)))
  << allocate_component<solver::actions::AdvanceTime>("AdvanceTime")
  << create_proto_action("Output", nodes_expression(_cout << "T(" << coordinates(0,0) << ") = " << T << "\n"));

  // Setup physics
  physics::PhysModel& physical_model = model.create_physics("cf3.UFEM.NavierStokesPhysics");
  physical_model.options().set("dynamic_viscosity", 1.7894e-5);
  physical_model.options().set("density", 1.);
  physical_model.options().set("reference_velocity", 1.);

  // Setup mesh
  // Mesh& mesh = *domain.create_component<Mesh>("Mesh");
  // Tools::MeshGeneration::create_line(mesh, length, nb_segments);
  boost::shared_ptr<MeshGenerator> create_line = build_component_abstract_type<MeshGenerator>("cf3.mesh.SimpleMeshGenerator","create_line");
  create_line->options().set("mesh",domain.uri()/"Mesh");
  create_line->options().set("lengths",std::vector<Real>(DIM_1D, length));
  create_line->options().set("nb_cells",std::vector<Uint>(DIM_1D, nb_segments));
  Mesh& mesh = create_line->generate();

  lss_action->options().set("regions", std::vector<URI>(1, mesh.topology().uri()));
  ic->get_child("Initialize")->options().set("regions", std::vector<URI>(1, mesh.topology().uri()));

  // Set boundary conditions
  bc->add_constant_bc("xneg", "Temperature", 1.);
  bc->add_constant_bc("xpos", "Temperature", 0.);

  // Configure timings
  Time& time = model.create_time();
  time.options().set("time_step", 1.);
  time.options().set("end_time", 100.);

  // Run the solver
  model.simulate();

  // Check result
//  t = model.time().current_time();
//  std::cout << "Checking solution at time " << t << std::endl;
//  for_each_node
//  (
//    mesh.topology(),
//        _check_close(-(_exp(Pe2 * coordinates[0])-1.)/(_exp(Pe2)-1.)+1, T, 1.)
//  );

  std::cout << "Finished test" << std::endl;

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
