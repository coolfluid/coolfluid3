// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_PROTO_MAX_ARITY 10
#define BOOST_MPL_LIMIT_METAFUNCTION_ARITY 10

#include "common/Builder.hpp"
#include "common/OptionList.hpp"

#include "mesh/Elements.hpp"

#include "mesh/LagrangeP2/Line1D.hpp"
#include "mesh/LagrangeP2/LibLagrangeP2.hpp"
#include "mesh/LagrangeP1/LibLagrangeP1.hpp"

#include "math/LSS/SolveLSS.hpp"
#include "math/LSS/ZeroLSS.hpp"

#include "solver/actions/Proto/Expression.hpp"
#include "solver/Tags.hpp"

#include "UFEM/Tags.hpp"

#include "NavierStokesSpecialized.hpp"
#include "../SUPG.hpp"

namespace cf3 {
namespace UFEM {
namespace demo {

using namespace solver::actions::Proto;

common::ComponentBuilder < NavierStokesSpecialized, LSSAction, LibUFEMDemo > NavierStokesSpecialized_builder;

struct NavierStokesSpecializedAssembly
{
  typedef void result_type;

  template<typename PT, typename UT, typename NUT, typename LSST>
  void operator()(const PT& p, const UT& u, const NUT& nu_eff, const Real& rho, const Real& theta, const Real& invdt, LSST& lss, math::LSS::BlockAccumulator& acc) const
  {
    typedef mesh::LagrangeP1::Triag2D ElementT;
    const RealVector2 u_avg = u.value().colwise().mean();
    const ElementT::NodesT& nodes = u.support().nodes();
    const Real volume = u.support().volume();
    const Real fc = 0.5;
    const Real nu = fabs(nu_eff.value().mean());

    Eigen::Matrix<Real, 9, 9> A, T;
    Eigen::Matrix<Real, 9, 1> x;
    A.setZero();
    T.setZero();
    acc.neighbour_indices(u.support().element_connectivity());

    // Face normals
    ElementT::NodesT normals;
    normals(0, XX) = nodes(1, YY) - nodes(2, YY);
    normals(0, YY) = nodes(2, XX) - nodes(1, XX);
    normals(1, XX) = nodes(2, YY) - nodes(0, YY);
    normals(1, YY) = nodes(0, XX) - nodes(2, XX);
    normals(2, XX) = nodes(0, YY) - nodes(1, YY);
    normals(2, YY) = nodes(1, XX) - nodes(0, XX);

    Real tau_ps, tau_su, tau_bulk;
    ComputeTauImpl compute_tau;
    compute_tau.compute_coefficients(u, nu, 1./invdt, tau_ps, tau_su, tau_bulk);

    for(Uint i=0; i<3; ++i)
    {
      const Uint Ui = 3*i;
      const Uint Vi = Ui + 1;
      const Uint Pi = Ui + 2;

      const Real u_ni = u_avg[XX]*normals(i, XX)+u_avg[YY]*normals(i, YY);
      
      x[Ui] = u.value()(i, XX);
      x[Vi] = u.value()(i, YY);
      x[Pi] = p.value()[i];

      for(Uint j=0; j<3; ++j)
      {
        const Uint Uj = 3*j;
        const Uint Vj = Uj + 1;
        const Uint Pj = Uj + 2;

        const Real uk=u_avg[XX];
        const Real vk=u_avg[YY];
        const Real uknj=uk*normals(j, XX)+vk*normals(j, YY);

        // Convection (Standard + SUPG)
        Real val  = 1./6.*uknj;
        val += tau_su/(4.*volume)*uknj*u_ni;
        A(Ui,Uj) += val;
        A(Vi,Vj) += val;

        // Convection (PSPG)
        val = tau_ps/(4.*volume)*uknj;
        A(Pi,Uj) += val*normals(i, XX);
        A(Pi,Vj) += val*normals(i, YY);

        // Convection Skewsymm (Standard + SUPG)
        val  = fc/6.;
        val += fc*tau_su/(4.*volume)*u_ni;
        A(Ui,Uj) += val*uk*normals(j, XX);
        A(Ui,Vj) += val*uk*normals(j, YY);
        A(Vi,Uj) += val*vk*normals(j, XX);
        A(Vi,Vj) += val*vk*normals(j, YY);

        // Convection Skewsymm (PSPG)
        val = fc*tau_ps/(4.*volume);
        A(Pi,Uj) += val*normals(i, XX)*uk*normals(j, XX);
        A(Pi,Vj) += val*normals(i, XX)*uk*normals(j, YY);
        A(Pi,Uj) += val*normals(i, YY)*vk*normals(j, XX);
        A(Pi,Vj) += val*normals(i, YY)*vk*normals(j, YY);

        //difusion (Standard)
        val = nu/(4.*volume);
        A(Ui,Uj)+=val*(4./3.*normals(i, XX)*normals(j, XX)+      normals(i, YY)*normals(j, YY));
        A(Ui,Vj)+=val*       normals(i, XX)*normals(j, YY)/3.;
        A(Vi,Uj)+=val*       normals(i, YY)*normals(j, XX)/3.;
        A(Vi,Vj)+=val*(      normals(i, XX)*normals(j, XX)+4./3.*normals(i, YY)*normals(j, YY));

        // Pressure (Standard + SUPG)
        val  = 1./(6.*rho);
        val += tau_su/(4.*rho*volume)*u_ni;
        A(Ui,Pj) += normals(j, XX)*val;
        A(Vi,Pj) += normals(j, YY)*val;

        // Pressure (PSPG)
        const Real laplacian = 1./(4.*rho*volume)*(normals(i, XX)*normals(j, XX)+normals(i, YY)*normals(j, YY));
        A(Pi,Pj) += tau_ps*laplacian;

        // Continuity (Standard)
        val = 1./6.;
        A(Pi,Uj) += val*normals(j, XX);
        A(Pi,Vj) += val*normals(j, YY);

        // Bulk viscosity (Standard)
        val = tau_bulk/(4.*volume);
        A(Ui,Uj) += val*normals(i, XX)*normals(j, XX);
        A(Ui,Vj) += val*normals(i, XX)*normals(j, YY);
        A(Vi,Uj) += val*normals(i, YY)*normals(j, XX);
        A(Vi,Vj) += val*normals(i, YY)*normals(j, YY);

        // Time (Standard + SUPG)
        val  = volume/12.*(1.+((i)==(j)?1.:0.));
        val += tau_su/6.*u_ni;
        T(Ui,Uj) += val;
        T(Vi,Vj) += val;

        // Time (PSPG)
        val = tau_ps/6.;
        T(Pi,Uj) += val*normals(i, XX);
        T(Pi,Vj) += val*normals(i, YY);
      }
    }
    acc.rhs = -A*x;
    A.row(0) /= theta;
    A.row(3) /= theta;
    A.row(6) /= theta;
    acc.mat = invdt * T + theta*A;
    lss.matrix().add_values(acc);
    lss.rhs().add_rhs_values(acc);
  }
};

static solver::actions::Proto::MakeSFOp<NavierStokesSpecializedAssembly>::type const assemble_ns_triags = {};

NavierStokesSpecialized::NavierStokesSpecialized ( const std::string& name ) : LSSActionUnsteady( name )
{
  using boost::proto::lit;
  
  // Option for the theta scheme
  options().add("theta", 1.)
    .pretty_name("Theta")
    .description("Theta coefficient for the theta-method.")
    .link_to(&theta);

  // Size of the block accumulator
  m_block_accumulator.resize(3, 3);

  // This determines the name of the field that will be used to store the solution
  set_solution_tag("navier_stokes_solution");

  // Create action components that wil be executed in the order they are created here:
  // 1. Set the linear system matrix and vectors to zero
  create_component<math::LSS::ZeroLSS>("ZeroLSS");

  // 2. Assemble the system matrix and RHS using a custom proto expression
  FieldVariable<0, VectorField> u("Velocity", solution_tag());
  FieldVariable<1, ScalarField> p("Pressure", solution_tag());
  FieldVariable<2, ScalarField> nu_eff("EffectiveViscosity", "navier_stokes_viscosity");
  PhysicsConstant rho("density");
  
  Handle<ProtoAction> assembly = create_component<ProtoAction>("Assembly");
  assembly->set_expression(elements_expression(
    boost::mpl::vector1< mesh::LagrangeP1::Triag2D>(),
    assemble_ns_triags(p, u, nu_eff, rho, lit(theta), lit(invdt()), system_matrix, m_block_accumulator)
  ));


  // 3. Apply boundary conditions
  Handle<BoundaryConditions> bc = create_component<BoundaryConditions>("BoundaryConditions");
  bc->mark_basic();
  bc->set_solution_tag(solution_tag());

  // 4. Solve the linear system
  create_component<math::LSS::SolveLSS>("SolveLSS");

  // 5. Update the solution
  create_component<ProtoAction>("Update")->set_expression(nodes_expression(group(p += solution(p), u += solution(u))));
}

void NavierStokesSpecialized::on_initial_conditions_set(InitialConditions& initial_conditions)
{
  // Use proto to add an initial condition on the viscosity, setting it equal to the molecular viscosity
  FieldVariable<0, ScalarField> nu_eff("EffectiveViscosity", "navier_stokes_viscosity");
  PhysicsConstant nu("kinematic_viscosity");
  Handle<ProtoAction> visc_ic(initial_conditions.create_initial_condition("navier_stokes_viscosity", "cf3.solver.ProtoAction"));
  visc_ic->set_expression(nodes_expression(nu_eff = nu));
}


} // demo
} // UFEM
} // cf3
