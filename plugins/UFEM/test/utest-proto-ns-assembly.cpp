// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Tests the assembly on a single element"

#include <boost/test/unit_test.hpp>

#define BOOST_PROTO_MAX_ARITY 10
#ifdef BOOST_MPL_LIMIT_METAFUNCTION_ARITY
  #undef BOOST_MPL_LIMIT_METAFUNCTION_ARITY
#endif
#define BOOST_MPL_LIMIT_METAFUNCTION_ARITY 10

#include "common/Core.hpp"
#include "common/Environment.hpp"
#include "common/Group.hpp"

#include "mesh/Domain.hpp"
#include "mesh/LagrangeP1/Triag2D.hpp"
#include "mesh/LagrangeP1/Tetra3D.hpp"

#include "solver/Tags.hpp"

#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"
#include <solver/ModelUnsteady.hpp>
#include <solver/Solver.hpp>

#include <UFEM/Solver.hpp>
#include <UFEM/LSSActionUnsteady.hpp>

using namespace cf3;
using namespace cf3::solver;
using namespace cf3::solver::actions;
using namespace cf3::solver::actions::Proto;
using namespace cf3::common;
using namespace cf3::mesh;
using namespace cf3::UFEM;

using namespace boost;

typedef std::vector<std::string> StringsT;
typedef std::vector<Uint> SizesT;

struct NavierStokesAssemblyFixture
{
  template<Uint Dim, typename ExprT>
  void run_model(const boost::shared_ptr<Mesh>& mesh, const ExprT& initial_condition_expression, const Real eps = 1e-10)
  {
    boost::shared_ptr<common::Group> root = allocate_component<Group>("Root");
    Handle<ModelUnsteady> model = root->create_component<ModelUnsteady>("NavierStokes");
    Domain& domain = model->create_domain("Domain");
    physics::PhysModel& physical_model = model->create_physics("cf3.UFEM.NavierStokesPhysics");

    Handle<UFEM::Solver> solver(model->create_solver("cf3.UFEM.Solver").handle());
    Handle<InitialConditions> ic = solver->create_initial_conditions();
    Handle<UFEM::LSSActionUnsteady> lss_action(solver->add_unsteady_solver("cf3.UFEM.NavierStokes"));

    physical_model.options().set("density", 1.);
    physical_model.options().set("dynamic_viscosity", 1.);

    Time& time = model->create_time();
    time.options().set("time_step", 1.);

    ic->remove_component(lss_action->solution_tag());

    domain.add_component(mesh);
    mesh->raise_mesh_loaded();

    solver->configure_option_recursively("regions", std::vector<URI>(1, mesh->topology().uri()));
    math::LSS::System& lss = lss_action->create_lss();

    const std::vector<std::string> disabled_actions = boost::assign::list_of("BoundaryConditions")("SolveLSS")("Update");
    lss_action->options().set("disabled_actions", disabled_actions);

    lss_action->options().set("use_specializations", true);
    time.options().set("end_time", 1.);
    solver->create_fields();
    for_each_node<Dim>(mesh->topology(), initial_condition_expression);
    model->simulate();

    const Uint matsize = lss.matrix()->blockcol_size()*lss.matrix()->neq();

    RealMatrix spec_result(matsize, matsize);
    for(Uint i = 0; i != matsize; ++i)
      for(Uint j = 0; j != matsize; ++j)
        lss.matrix()->get_value(i, j, spec_result(i,j));

    lss_action->options().set("use_specializations", false);
    time.options().set("end_time", 2.);
    model->simulate();

    RealMatrix generic_result(matsize, matsize);
    for(Uint i = 0; i != matsize; ++i)
      for(Uint j = 0; j != matsize; ++j)
        lss.matrix()->get_value(i, j, generic_result(i,j));

    check_close(generic_result, spec_result, eps);
  }

  boost::shared_ptr<Mesh> create_triangle(const RealVector2& a, const RealVector2& b, const RealVector2& c)
  {
    // coordinates
    boost::shared_ptr<Mesh> mesh_ptr = allocate_component<Mesh>("mesh");
    Mesh& mesh = *mesh_ptr;
    mesh.initialize_nodes(3, 2);
    Dictionary& geometry_dict = mesh.geometry_fields();
    Field& coords = geometry_dict.coordinates();
    coords << a[0] << a[1] << b[0] << b[1] << c[0] << c[1];

    // Define the volume cells, i.e. the blocks
    Elements& cells = mesh.topology().create_region("cells").create_elements("cf3.mesh.LagrangeP1.Triag2D", geometry_dict);
    cells.resize(1);
    cells.geometry_space().connectivity() << 0 << 1 << 2;

    return mesh_ptr;
  }

  boost::shared_ptr<Mesh> create_tetra(const RealVector3& a, const RealVector3& b, const RealVector3& c, const RealVector3& d)
  {
    boost::shared_ptr<Mesh> mesh_ptr = allocate_component<Mesh>("mesh");
    Mesh& mesh = *mesh_ptr;
    // coordinates
    mesh.initialize_nodes(4, 3);
    Dictionary& geometry_dict = mesh.geometry_fields();
    Field& coords = geometry_dict.coordinates();
    coords << a[0] << a[1] << a[2] << b[0] << b[1] << b[2] << c[0] << c[1] << c[2] << d[0] << d[1] << d[2];

    // Define the volume cells, i.e. the blocks
    Elements& cells = mesh.topology().create_region("cells").create_elements("cf3.mesh.LagrangeP1.Tetra3D", geometry_dict);
    cells.resize(1);
    cells.geometry_space().connectivity() << 0 << 1 << 2 << 3;

    return mesh_ptr;
  }

  void check_close(const RealMatrix& a, const RealMatrix& b, const Real eps)
  {
    for(Uint i = 0; i != a.rows(); ++i)
      for(Uint j = 0; j != a.cols(); ++j)
        BOOST_CHECK_CLOSE(a(i,j), b(i,j), eps);
  }
};

BOOST_FIXTURE_TEST_SUITE( NavierStokesAssemblySuite, NavierStokesAssemblyFixture )

BOOST_AUTO_TEST_CASE( InitMPI )
{
  common::PE::Comm::instance().init(boost::unit_test::framework::master_test_suite().argc, boost::unit_test::framework::master_test_suite().argv);
  BOOST_CHECK_EQUAL(common::PE::Comm::instance().size(), 1);
  Core::instance().environment().options().set("log_level", 1u);
}

BOOST_AUTO_TEST_CASE( UnitTriangleUniform )
{
  FieldVariable<0, VectorField> u("Velocity", "navier_stokes_solution");
  RealVector u_init(2); u_init << 1., 1.;
  // TODO: Check why the triangles fail
  //run_model<2>(create_triangle(RealVector2(0., 0.), RealVector2(1., 0.), RealVector2(0., 1.)), u = u_init);
}

BOOST_AUTO_TEST_CASE( TetraUniform )
{
  FieldVariable<0, VectorField> u("Velocity", "navier_stokes_solution");
  RealVector u_init(3); u_init << 1., 1., 1.;
  //run_model<3>(create_tetra(RealVector3(0., 0., 0.), RealVector3(1., 0., 0.), RealVector3(0., 1., 0.), RealVector3(0., 0., 1.)), u = u_init);
}

BOOST_AUTO_TEST_CASE( GenericTriangleUniform )
{
  FieldVariable<0, VectorField> u("Velocity", "navier_stokes_solution");
  RealVector u_init(2); u_init << 1., 1.;
  // TODO: Check why the triangles fail
  //run_model<2>(create_triangle(RealVector2(0.2, 0.1), RealVector2(0.75, -0.1), RealVector2(0.33, 0.83)), u = u_init);
}


BOOST_AUTO_TEST_CASE( GenericTetraUniform )
{
  FieldVariable<0, VectorField> u("Velocity", "navier_stokes_solution");
  RealVector u_init(3); u_init << 1., 1., 1.;
  //run_model<3>(create_tetra(RealVector3(0.2, 0.1, -0.1), RealVector3(0.75, -0.1, 0.05), RealVector3(0.33, 0.83, 0.23), RealVector3(0.1, -0.1, 0.67)), u = u_init);
}


BOOST_AUTO_TEST_CASE( GenericTriangleVortex )
{
  RealMatrix2 n_op; n_op << 0., 100., -100., 0.; // Linear operator to create a normal vector in 2D
  FieldVariable<0, VectorField> u("Velocity", "navier_stokes_solution");
  // TODO: Check why the triangles fail
  //run_model<2>(create_triangle(RealVector2(100.2, 100.1), RealVector2(100.75, 99.9), RealVector2(100.33, 100.83)), u = n_op*coordinates / (coordinates[0]*coordinates[0] + coordinates[1]*coordinates[1]), 0.5);
}

BOOST_AUTO_TEST_CASE( GenericTetraVortex )
{
  RealMatrix3 n_op; n_op << 0., 1., 0., -1., 0., 0., 0., 0., 0.; // Linear operator to create a normal vector
  FieldVariable<0, VectorField> u("Velocity", "navier_stokes_solution");
  //run_model<3>(create_tetra(RealVector3(100.2, 100.1, 99.9), RealVector3(100.75, 99.9, 100.05), RealVector3(100.33, 100.83, 100.23), RealVector3(100.1, 99.9, 100.67)), u = n_op*coordinates / (coordinates[0]*coordinates[0] + coordinates[1]*coordinates[1]), 5.);
}

BOOST_AUTO_TEST_SUITE_END()
