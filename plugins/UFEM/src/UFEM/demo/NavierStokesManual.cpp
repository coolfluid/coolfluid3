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

#include "NavierStokesManual.hpp"

namespace cf3 {
namespace UFEM {
namespace demo {

using namespace solver::actions::Proto;

common::ComponentBuilder < NavierStokesManual, LSSAction, LibUFEMDemo > NavierStokesManual_builder;

class NavierStokesManualAssembly : public solver::Action
{
public:
  NavierStokesManualAssembly ( const std::string& name ) : solver::Action(name), rho(1.), invdt(1.), theta(0.5)
  {
  }
  
  static std::string type_name () { return "NavierStokesManualAssembly"; }
  
  virtual void execute()
  {
    if(is_null(m_lss))
      throw common::SetupError(FromHere(), "LSS not set for " + uri().path());
    
    math::LSS::Matrix& lss_mat = *m_lss->matrix();
    math::LSS::Vector& lss_rhs = *m_lss->rhs();
    
    typedef mesh::LagrangeP1::Triag2D ElementT;
    
    BOOST_FOREACH(const Handle<mesh::Region>& region, m_loop_regions)
    {
      BOOST_FOREACH(const mesh::Elements& elements, common::find_components_recursively_with_filter<mesh::Elements>(*region, mesh::IsElementType<mesh::LagrangeP1::Triag2D>()))
      {
        const Uint nb_elems = elements.size();
        const mesh::Connectivity& connectivity = elements.geometry_space().connectivity();
        const mesh::Field& coordinates = elements.geometry_fields().coordinates();
        Eigen::Matrix<Real, 3, 2> nodes, normals, velocity;
        math::LSS::BlockAccumulator acc;
        acc.resize(3, 3);

        Eigen::Matrix<Real, 3, 1> nu_eff;
        
        const mesh::Mesh& mesh = common::find_parent_component<mesh::Mesh>(elements);
        const mesh::Field& solution_field = common::find_component_recursively_with_tag<mesh::Field>(mesh, "navier_stokes_solution");
        const mesh::Field& nu_field = common::find_component_recursively_with_tag<mesh::Field>(mesh, "navier_stokes_viscosity");

        Eigen::Matrix<Real, 9, 9> A, T;
        Eigen::Matrix<Real, 9, 1> x;
        
        for(Uint elem = 0; elem != nb_elems; ++elem)
        {
          const mesh::Connectivity::ConstRow conn_row = connectivity[elem];
          acc.neighbour_indices(conn_row);
          for(Uint i = 0; i != 3; ++i)
          {
            const Uint node_idx = conn_row[i];
            nodes(i, XX) = coordinates[node_idx][XX];
            nodes(i, YY) = coordinates[node_idx][YY];
            velocity(i, XX) = solution_field[node_idx][0];
            velocity(i, YY) = solution_field[node_idx][1];
            x[i*3] = velocity(i, XX);
            x[i*3+1] = velocity(i, YY);
            x[i*3+2] = solution_field[node_idx][2];
            nu_eff[i] = nu_field[i][0];
          }
          
          const RealVector2 u_avg = velocity.colwise().mean();
          const Real volume = ElementT::volume(nodes);
          const Real fc = 0.5;
          const Real nu = fabs(nu_eff.mean());

          // Face normals
          normals(0, XX) = nodes(1, YY) - nodes(2, YY);
          normals(0, YY) = nodes(2, XX) - nodes(1, XX);
          normals(1, XX) = nodes(2, YY) - nodes(0, YY);
          normals(1, YY) = nodes(0, XX) - nodes(2, XX);
          normals(2, XX) = nodes(0, YY) - nodes(1, YY);
          normals(2, YY) = nodes(1, XX) - nodes(0, XX);

          const Real umag = u_avg.norm();

          // Get the minimal edge length
          Real h_rgn = 1e10;
          for(Uint i = 0; i != 3; ++i)
          {
            for(Uint j = 0; j != 3; ++j)
            {
              if(i != j)
              {
                h_rgn = std::min(h_rgn, (nodes.row(i) - nodes.row(j)).squaredNorm());
              }
            }
          }
          h_rgn = sqrt(h_rgn);
          
          const Real h_ugn = h_rgn;//fabs(2.*umag / (u.eval()*u.nabla()).sum());
          
          const Real tau_adv_inv = (2.*umag)/h_ugn;
          const Real tau_time_inv = 2.*invdt;
          const Real tau_diff_inv = (nu)/(h_rgn*h_rgn);
          
          const Real tau_su = 1./(tau_adv_inv + tau_time_inv + 4.*tau_diff_inv);
          const Real tau_ps = tau_su;
          const Real tau_bulk = tau_su*umag*umag;

          A.setZero();
          T.setZero();
          
          for(Uint i=0; i<3; ++i)
          {
            const Uint Ui = 3*i;
            const Uint Vi = Ui + 1;
            const Uint Pi = Ui + 2;
            const Real u_ni = u_avg[XX]*normals(i, XX)+u_avg[YY]*normals(i, YY);

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
          lss_mat.add_values(acc);
          lss_rhs.add_rhs_values(acc);
        }
      }
    }
  }
  
  Handle<math::LSS::System> m_lss;
  Real rho;
  Real invdt;
  Real theta;
};

common::ComponentBuilder < NavierStokesManualAssembly, common::Component, LibUFEM > NavierStokesManualAssembly_builder;

NavierStokesManual::NavierStokesManual ( const std::string& name ) : LSSActionUnsteady( name )
{
  // Option for the theta scheme
  options().add("theta", 1.)
    .pretty_name("Theta")
    .description("Theta coefficient for the theta-method.");

  // This determines the name of the field that will be used to store the solution
  set_solution_tag("navier_stokes_solution");

  // Create action components that wil be executed in the order they are created here:
  // 1. Set the linear system matrix and vectors to zero
  create_component<math::LSS::ZeroLSS>("ZeroLSS");

  // 2. Assemble the system matrix and RHS using an action component
  Handle<NavierStokesManualAssembly> assembly = create_component<NavierStokesManualAssembly>("Assembly");
  options().option("lss").link_to(&assembly->m_lss);

  // 3. Apply bondary conditions
  Handle<BoundaryConditions> bc = create_component<BoundaryConditions>("BoundaryConditions");
  bc->mark_basic();
  bc->set_solution_tag(solution_tag());

  // 4. Solve the linear system
  create_component<math::LSS::SolveLSS>("SolveLSS");

  // 5. Update the solution
  FieldVariable<0, VectorField> u("Velocity", solution_tag());
  FieldVariable<1, ScalarField> p("Pressure", solution_tag());
  create_component<ProtoAction>("Update")->set_expression(nodes_expression(group(p += solution(p), u += solution(u))));
}

void NavierStokesManual::execute()
{
  // Get the assembly component
  Handle<NavierStokesManualAssembly> assembly(get_child("Assembly"));
  cf3_assert(is_not_null(assembly));
  cf3_assert(is_not_null(assembly->m_lss));

  // Set the physical constants
  assembly->rho = physical_model().options().value<Real>("density");

  // Set theta
  assembly->theta = options().value<Real>("theta");

  // Set time step
  assembly->invdt = time().invdt();

  // Execute
  LSSActionUnsteady::execute();
}

void NavierStokesManual::on_initial_conditions_set(InitialConditions& initial_conditions)
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
