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

#include "mesh/Domain.hpp"
#include "mesh/LagrangeP1/Triag2D.hpp"
#include "mesh/LagrangeP1/Tetra3D.hpp"

#include "solver/Tags.hpp"

#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"

#include "UFEM/NavierStokesOps.hpp"

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
  NavierStokesAssemblyFixture()
  {
    c.rho = 1.;
    c.mu = 1.;
    c.one_over_rho = 1.;
    c.u_ref = 1.;
    c.tau_bulk = 1.;
    c.tau_ps = 1.;
    c.tau_su = 1.;
  }
  Domain& domain()
  {
    if(is_null(m_domain))
      m_domain = Handle<Domain>(Core::instance().root().get_child("domain"));
    if(is_null(m_domain))
      m_domain = Core::instance().root().create_component<Domain>("domain");

    return *m_domain;
  }

  void create_triangle(Mesh& mesh, const RealVector2& a, const RealVector2& b, const RealVector2& c)
  {
    // coordinates
    mesh.initialize_nodes(3, 2);
    Dictionary& geometry_dict = mesh.geometry_fields();
    Field& coords = geometry_dict.coordinates();
    coords << a[0] << a[1] << b[0] << b[1] << c[0] << c[1];

    // Define the volume cells, i.e. the blocks
    Elements& cells = mesh.topology().create_region("cells").create_elements("cf3.mesh.LagrangeP1.Triag2D", geometry_dict);
    cells.resize(1);
    cells.geometry_space().connectivity() << 0 << 1 << 2;

    Field& solution = mesh.geometry_fields().create_field("solution", "p[scalar],u[vector]");
    solution.add_tag("solution");
  }

  void create_tetra(Mesh& mesh, const RealVector3& a, const RealVector3& b, const RealVector3& c, const RealVector3& d)
  {
    // coordinates
    mesh.initialize_nodes(4, 3);
    Dictionary& geometry_dict = mesh.geometry_fields();
    Field& coords = geometry_dict.coordinates();
    coords << a[0] << a[1] << a[2] << b[0] << b[1] << b[2] << c[0] << c[1] << c[2] << d[0] << d[1] << d[2];

    // Define the volume cells, i.e. the blocks
    Elements& cells = mesh.topology().create_region("cells").create_elements("cf3.mesh.LagrangeP1.Tetra3D", geometry_dict);
    cells.resize(1);
    cells.geometry_space().connectivity() << 0 << 1 << 2 << 3;

    Field& solution = mesh.geometry_fields().create_field("solution", "p[scalar],u[vector]");
    solution.add_tag("solution");
  }

  template<typename ElementT, typename UT, typename PT>
  boost::shared_ptr<Expression> generic_ns_assembly(RealMatrix& A, RealMatrix& T, UT& u, PT& p, SUPGCoeffs& c)
  {
    return elements_expression(boost::mpl::vector1<ElementT>(),
      group
      (
        _A(p) = _0, _A(u) = _0, _T(p) = _0, _T(u) = _0,
        compute_tau(u, c),
        element_quadrature
        (
          _A(p    , u[_i]) +=          transpose(N(p) + c.tau_ps*u*nabla(p)*0.5)       * nabla(u)[_i] + c.tau_ps * transpose(nabla(p)[_i]) * u*nabla(u), // Standard continuity + PSPG for advection
          _A(p    , p)     += c.tau_ps * transpose(nabla(p))     * nabla(p) * c.one_over_rho,     // Continuity, PSPG
          _A(u[_i], u[_i]) += c.mu     * transpose(nabla(u))     * nabla(u) * c.one_over_rho     + transpose(N(u) + c.tau_su*u*nabla(u)) * u*nabla(u),     // Diffusion + advection
          _A(u[_i], p)     += c.one_over_rho * transpose(N(u) + c.tau_su*u*nabla(u)) * nabla(p)[_i], // Pressure gradient (standard and SUPG)
          _A(u[_i], u[_j]) += transpose((c.tau_bulk + 0.33333333333333*boost::proto::lit(c.mu)*c.one_over_rho)*nabla(u)[_i] // Bulk viscosity and second viscosity effect
                                          + 0.5*u[_i]*(N(u) + c.tau_su*u*nabla(u))) * nabla(u)[_j],  // skew symmetric part of advection (standard +SUPG)
          _T(p    , u[_i]) += c.tau_ps * transpose(nabla(p)[_i]) * N(u),         // Time, PSPG
          _T(u[_i], u[_i]) += transpose(N(u) + c.tau_su*u*nabla(u))         * N(u)          // Time, standard and SUPG
        ),
        boost::proto::lit(A) = _A, boost::proto::lit(T) = _T
      )
    );
  }

  template<typename ElementT>
  void runtest(Mesh& mesh)
  {
    MeshTerm<0, ScalarField> p("p", "solution");
    MeshTerm<1, VectorField> u("u", "solution");

    RealMatrix A, A_spec, T, T_spec;

    generic_ns_assembly<ElementT>(A, T, u, p, c)->loop(mesh.topology());

    for_each_element< boost::mpl::vector1<ElementT> >
    (
      mesh.topology(),
      group
      (
        _A(p) = _0, _A(u) = _0, _T(p) = _0, _T(u) = _0,
      supg_specialized(p, u, c, _A, _T),
      boost::proto::lit(A_spec) = _A, boost::proto::lit(T_spec) = _T
      )
    );

    std::cout << "A:\n" << A << "\nT:\n" << T << std::endl;

    check_close(A, A_spec);
    check_close(T, T_spec);
  }

  void check_close(const RealMatrix& a, const RealMatrix& b)
  {
    for(Uint i = 0; i != a.rows(); ++i)
      for(Uint j = 0; j != a.cols(); ++j)
        BOOST_CHECK_CLOSE(a(i,j), b(i,j), 1e-12);
  }

  SUPGCoeffs c;
  Handle<Domain> m_domain;
};

BOOST_FIXTURE_TEST_SUITE( NavierStokesAssemblySuite, NavierStokesAssemblyFixture )

BOOST_AUTO_TEST_CASE( UnitTriangleUniform )
{
  Mesh& mesh = *domain().create_component<Mesh>("TriangleUniform");
  create_triangle(mesh, RealVector2(0., 0.), RealVector2(1., 0.), RealVector2(0., 1.));

  // Initialize u
  MeshTerm<0, VectorField> u("u", "solution");
  RealVector u_init(2); u_init << 1., 1.;
  for_each_node(mesh.topology(), u = u_init);

  runtest<LagrangeP1::Triag2D>(mesh);
}

BOOST_AUTO_TEST_CASE( TetraUniform )
{
  Mesh& mesh = *domain().create_component<Mesh>("TetraUniform");
  create_tetra(mesh, RealVector3(0., 0., 0.), RealVector3(1., 0., 0.), RealVector3(0., 1., 0.), RealVector3(0., 0., 1.));

  // Initialize u
  MeshTerm<0, VectorField> u("u", "solution");
  RealVector u_init(3); u_init << 1., 1., 1.;
  for_each_node(mesh.topology(), u = u_init);

  runtest<LagrangeP1::Tetra3D>(mesh);
}

BOOST_AUTO_TEST_CASE( GenericTriangleUniform )
{
  Mesh& mesh = *domain().create_component<Mesh>("TriangleUniform");
  create_triangle(mesh, RealVector2(0.2, 0.1), RealVector2(0.75, -0.1), RealVector2(0.33, 0.83));

  // Initialize u
  MeshTerm<0, VectorField> u("u", "solution");
  RealVector u_init(2); u_init << 1., 1.;
  for_each_node(mesh.topology(), u = u_init);

  runtest<LagrangeP1::Triag2D>(mesh);
}


BOOST_AUTO_TEST_CASE( GenericTetraUniform )
{
  Mesh& mesh = *domain().create_component<Mesh>("TriangleUniform");
  create_tetra(mesh, RealVector3(0.2, 0.1, -0.1), RealVector3(0.75, -0.1, 0.05), RealVector3(0.33, 0.83, 0.23), RealVector3(0.1, -0.1, 0.67));

  // Initialize u
  MeshTerm<0, VectorField> u("u", "solution");
  RealVector u_init(3); u_init << 1., 1., 1.;
  for_each_node(mesh.topology(), u = u_init);

  runtest<LagrangeP1::Tetra3D>(mesh);
}

BOOST_AUTO_TEST_SUITE_END()
