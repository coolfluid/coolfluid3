// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "../NavierStokesSemiImplicit.hpp"

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include <boost/mpl/back_inserter.hpp>
#include <boost/mpl/copy.hpp>

#include <Thyra_DefaultDiagonalLinearOp.hpp>
#include <Thyra_DefaultScaledAdjointLinearOp.hpp>
#include <Thyra_DefaultSpmdMultiVector.hpp>
#include <Thyra_VectorStdOps.hpp>
#include <Thyra_MultiVectorStdOps.hpp>

#include "common/Component.hpp"
#include "common/Builder.hpp"
#include "common/OptionT.hpp"
#include "common/OptionArray.hpp"
#include "common/PropertyList.hpp"

#include "math/LSS/SolveLSS.hpp"
#include "math/LSS/ZeroLSS.hpp"
#include "math/LSS/Trilinos/TrilinosCrsMatrix.hpp"
#include "math/LSS/Trilinos/TekoBlockedOperator.hpp"
#include "math/LSS/Trilinos/TrilinosVector.hpp"

#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"
#include "solver/actions/Iterate.hpp"
#include "solver/CriterionTime.hpp"
#include "solver/actions/AdvanceTime.hpp"
#include "solver/Time.hpp"
#include "solver/Tags.hpp"

#include "../../Tags.hpp"

namespace cf3 {
namespace UFEM {

using namespace common;
using namespace solver;
using namespace solver::actions;
using namespace solver::actions::Proto;

using boost::proto::lit;

ComponentBuilder < NavierStokesSemiImplicit, common::ActionDirector, LibUFEM > NavierStokesSemiImplicit_builder;

namespace detail
{
  // Helper function to create vectors
  Handle<math::LSS::Vector> create_vector(math::LSS::System& lss, const std::string& name)
  {
    Handle<math::LSS::Vector> vec(lss.create_component(name, "cf3.math.LSS.TrilinosVector"));
    lss.solution()->clone_to(*vec);
    return vec;
  }
  
  const std::string& my_tag()
  {
    static const std::string tag("ns_semi_region_update");
    return tag;
  }
}

/// This is the inner solve loop
struct InnerLoop : solver::Action
{
  InnerLoop ( const std::string& name ) :
    solver::Action(name)
  {
    options().add(solver::Tags::time(), m_time)
      .pretty_name("Time")
      .description("Component that keeps track of time for this simulation")
      .link_to(&m_time);
    
    nb_iterations = 2;
    m_u_rhs_assembly = create_component<solver::ActionDirector>("URHSAssembly");
    m_p_rhs_assembly = create_component<solver::ActionDirector>("PRHSAssembly");
    m_apply_aup = create_component<solver::ActionDirector>("ApplyAup");
    
    solve_u_lss = create_component<math::LSS::SolveLSS>("SolveUSystem");
    solve_p_lss = create_component<math::LSS::SolveLSS>("SolvePSystem");
  }
  
  static std::string type_name () { return "InnerLoop"; }
  
  void execute()
  {
    typedef std::pair<Uint,Uint> BlockrowIdxT;
    
    a->reset(0.);
    delta_p_sum->reset(0.);
    u->assign(*u_lss->solution());
    p->assign(*p_lss->solution());

    // Apply velocity BC
    u_lss->rhs()->reset(0.);
    velocity_bc->execute();
    // The velocity BC deals with velocity, so we need to write this in terms of acceleration
    u_lss->rhs()->scale(1./m_time->dt());
    
    for(Uint i = 0; i != nb_iterations; ++i)
    {
      if(i != 0)
      {
        u_lss->rhs()->reset(0.);
      }
      // Velocity system: compute delta_a_star
      auu->apply(u_lss->rhs(), u, -1., 1.); // Apply pre-built Auu matrix
      auu->apply(u_lss->rhs(), a, m_time->dt()*(1.-theta), 1.);
      m_u_rhs_assembly->execute(); // In-place assembly for the storage-heavy operators
      // Zero boundary condition after first pass
      if(i != 0)
      {
        BOOST_FOREACH(const BlockrowIdxT& diri_idx, Handle<math::LSS::TrilinosCrsMatrix>(u_lss->matrix())->get_dirichlet_nodes())
        {
          u_lss->rhs()->set_value(diri_idx.first, diri_idx.second, 0.);
        }
      }
      u_lss->solution()->reset(0.);
      CFdebug << "Solving velocity LSS..." << CFendl;
      solve_u_lss->execute();
      u_lss->solution()->sync();

      // Pressure system: compute delta_p
      p_lss->rhs()->reset(0.);
      m_p_rhs_assembly->execute();
      p_lss->solution()->reset(0.);
      // Apply BC if the first iteration, set RHS to 0 otherwise
      if(i ==0)
      {
        pressure_bc->execute();
      }
      else
      {
        BOOST_FOREACH(const BlockrowIdxT& diri_idx, Handle<math::LSS::TrilinosCrsMatrix>(p_lss->matrix())->get_dirichlet_nodes())
        {
          p_lss->rhs()->set_value(diri_idx.first, diri_idx.second, 0.);
        }
      }
      CFdebug << "Solving pressure LSS..." << CFendl;
      solve_p_lss->execute();
      p_lss->solution()->sync();

      // Compute delta_a
      u_lss->rhs()->reset(0.);
      m_apply_aup->execute(); // Compute Aup*delta_p (stored in u_lss RHS)
      // delta_a is delta_a_star for the dirichlet nodes
      BOOST_FOREACH(const BlockrowIdxT& diri_idx, Handle<math::LSS::TrilinosCrsMatrix>(u_lss->matrix())->get_dirichlet_nodes())
      {
        u_lss->rhs()->set_value(diri_idx.first, diri_idx.second, 0.);
      }
      Thyra::apply(*lumped_m_op, Thyra::NOTRANS, *aup_delta_p, delta_a.ptr(), -1., 1.); // delta_a = delta_a_star - Ml_inv*Aup*delta_p
      u_lss->solution()->sync(); // delta_a is a link to u_lss->solution(), so it needs a sync after matrix apply
      
      
      const math::LSS::Vector& da = *u_lss->solution();
      const math::LSS::Vector& dp = *p_lss->solution();
      
      a->update(da);
      u->update(da, m_time->dt());
      p->update(dp);
      delta_p_sum->update(dp);
    }
    
    u_lss->solution()->assign(*u);
    p_lss->solution()->assign(*p);
  }

  // Data members are public, because these are initialized where appropriate
  Handle<math::LSS::System> p_lss;
  Handle<math::LSS::System> u_lss;
  
  Handle<math::LSS::SolveLSS> solve_p_lss;
  Handle<math::LSS::SolveLSS> solve_u_lss;
  
  Handle<math::LSS::Matrix> auu;
  Handle<math::LSS::Vector> lumped_m_diag;
  
  Handle<common::Action> pressure_bc;
  Handle<common::Action> velocity_bc;

  int nb_iterations;

  Teuchos::RCP<const Thyra::LinearOpBase<Real> > lumped_m_op;

  Handle< math::LSS::Vector > u;
  Handle< math::LSS::Vector > a;
  Handle< math::LSS::Vector > p;
  Teuchos::RCP<Thyra::MultiVectorBase<Real> > delta_a;
  Teuchos::RCP<Thyra::MultiVectorBase<Real> > aup_delta_p; // This is actually u_lss->rhs()
  Handle< math::LSS::Vector > delta_p_sum;
  
  Handle<solver::Time> m_time;
  Real theta;

private:
  Handle<solver::ActionDirector> m_u_rhs_assembly;
  Handle<solver::ActionDirector> m_p_rhs_assembly;
  Handle<solver::ActionDirector> m_apply_aup;
};

ComponentBuilder < InnerLoop, common::Action, LibUFEM > InnerLoop_builder;

/// Initialize inner loop data
struct SetupInnerLoopData : solver::Action
{
  SetupInnerLoopData ( const string& name ) : solver::Action(name)
  {
  }

  static std::string type_name() { return "SetupInnerLoopData"; }

  void execute()
  {
    // We use the auu RHS as lumped mass matrix
    Handle<math::LSS::ThyraVector> lumped_diag(inner_loop->lumped_m_diag);
    Teuchos::RCP< Thyra::VectorBase<Real> > lumped_inv_diag = lumped_diag->thyra_vector();
    Thyra::reciprocal(*lumped_inv_diag, lumped_inv_diag.ptr());
    // Construct the diagonal op
    inner_loop->lumped_m_op = Thyra::diagonal(lumped_inv_diag);
    inner_loop->lumped_m_diag->sync();
  }

  Handle<InnerLoop> inner_loop;
};

ComponentBuilder < SetupInnerLoopData, common::Action, LibUFEM > SetupInnerLoopData_builder;

NavierStokesSemiImplicit::NavierStokesSemiImplicit(const std::string& name) :
  solver::ActionDirector(name),
  u("Velocity", "navier_stokes_u_solution"),
  p("Pressure", "navier_stokes_p_solution"),
  u_adv("AdvectionVelocity", "linearized_velocity"),
  u1("AdvectionVelocity1", "linearized_velocity"),
  u2("AdvectionVelocity2", "linearized_velocity"),
  u3("AdvectionVelocity3", "linearized_velocity"),
  nu_eff("EffectiveViscosity", "navier_stokes_viscosity"),
  u_ref("reference_velocity"),
  nu("kinematic_viscosity")
{
  options().add("theta", 1.)
    .pretty_name("Theta")
    .description("Theta coefficient for the theta-method.")
    .link_to(&theta)
    .attach_trigger(boost::bind(&NavierStokesSemiImplicit::trigger_theta, this));

  options().add("nb_iterations", 2)
    .pretty_name("Nb Iterations")
    .description("The number of iterations for the inner loop")
    .attach_trigger(boost::bind(&NavierStokesSemiImplicit::trigger_nb_iterations, this));
    
  options().add("initial_conditions", m_initial_conditions)
    .pretty_name("Initial Conditions")
    .description("The component that is used to manage the initial conditions in the solver this action belongs to")
    .link_to(&m_initial_conditions)
    .attach_trigger(boost::bind(&NavierStokesSemiImplicit::trigger_initial_conditions, this));
    
  options().add(solver::Tags::time(), m_time)
    .pretty_name("Time")
    .description("Component that keeps track of time for this simulation")
    .attach_trigger(boost::bind(&NavierStokesSemiImplicit::trigger_time, this))
    .link_to(&m_time);

  add_component(create_proto_action("LinearizeU", nodes_expression(u_adv = 2.1875*u - 2.1875*u1 + 1.3125*u2 - 0.3125*u3)));
  get_child("LinearizeU")->add_tag(detail::my_tag());

  m_p_lss = create_component<LSSAction>("PressureLSS");
  m_p_lss->mark_basic();
  m_p_lss->set_solution_tag("navier_stokes_p_solution");
  Handle<math::LSS::ZeroLSS> zero_p_lss = m_p_lss->create_component<math::LSS::ZeroLSS>("ZeroLSS");
  zero_p_lss->options().set("reset_matrix", false);
  m_p_lss->add_tag(detail::my_tag());
  m_p_lss->options().set("matrix_builder", std::string("cf3.math.LSS.TrilinosCrsMatrix"));

  m_u_lss = create_component<LSSAction>("VelocityLSS");
  m_u_lss->mark_basic();
  m_u_lss->create_component<math::LSS::ZeroLSS>("ZeroLSS");
  m_u_lss->set_solution_tag("navier_stokes_u_solution");
  m_u_lss->add_tag(detail::my_tag());
  m_u_lss->options().set("matrix_builder", std::string("cf3.math.LSS.TrilinosCrsMatrix"));

  m_auu_lss = create_component<LSSAction>("AuuLSS");
  m_auu_lss->mark_basic();
  m_auu_lss->create_component<math::LSS::ZeroLSS>("ZeroLSS")->options().set("reset_rhs", false);
  m_auu_lss->set_solution_tag("navier_stokes_u_solution");
  m_auu_lss->add_tag(detail::my_tag());
  
  // Assembly of the velocity system matrices
  m_velocity_assembly = create_component<solver::ActionDirector>("VelocityAssembly");
  m_velocity_assembly->add_tag(detail::my_tag());

  // Boundary conditions
  Handle<BoundaryConditions> pressure_bc =  m_p_lss->create_component<BoundaryConditions>("BC");
  pressure_bc->mark_basic();
  pressure_bc->set_solution_tag("navier_stokes_p_solution");
  m_p_lss->options().set("disabled_actions", std::vector<std::string>(1, "BC")); // Disabled because we execute it from the inner loop
  
  Handle<BoundaryConditions> velocity_bc =  m_u_lss->create_component<BoundaryConditions>("BC");
  velocity_bc->mark_basic();
  velocity_bc->set_solution_tag("navier_stokes_u_solution");
  m_u_lss->options().set("disabled_actions", std::vector<std::string>(1, "BC")); // Disabled because we execute it from the inner loop

  // Copy the current solution to the solution vectors
  create_component<ProtoAction>("SetSolution")->set_expression(nodes_expression(group(m_u_lss->solution(u) = u, m_p_lss->solution(p) = p)));
  get_child("SetSolution")->add_tag(detail::my_tag());
  
  // Solve the systems iteratively
  m_inner_loop = create_component<InnerLoop>("InnerLoop");
  m_inner_loop->mark_basic();
  m_inner_loop->add_tag(detail::my_tag());
  Handle<InnerLoop>(m_inner_loop)->pressure_bc = pressure_bc;
  Handle<InnerLoop>(m_inner_loop)->velocity_bc = velocity_bc;

  // Update the solution
  create_component<ProtoAction>("Update")->set_expression(nodes_expression(group(u3 = u2, u2 = u1, u1 = u, u = m_u_lss->solution(u), p = m_p_lss->solution(p))));
  get_child("Update")->add_tag(detail::my_tag());
  
  trigger_theta();
}

NavierStokesSemiImplicit::~NavierStokesSemiImplicit()
{
}

void NavierStokesSemiImplicit::trigger_initial_conditions()
{
  // Initial condition for the viscosity, defaulting to the molecular viscosity
  Handle<ProtoAction> visc_ic(m_initial_conditions->create_initial_condition("navier_stokes_viscosity", "cf3.solver.ProtoAction"));
  visc_ic->set_expression(nodes_expression(nu_eff = nu));
  visc_ic->add_tag(detail::my_tag());

  // Use a proto action to set the linearized_velocity easily
  Handle<ProtoAction> lin_vel_ic (m_initial_conditions->create_initial_condition("linearized_velocity", "cf3.solver.ProtoAction"));
  lin_vel_ic->set_expression(nodes_expression(group(u_adv = u, u1 = u, u2 = u, u3 = u)));
  lin_vel_ic->add_tag(detail::my_tag());

  if(is_not_null(m_pressure_assembly))
    m_initial_conditions->remove_component("PressureAssembly");
  m_pressure_assembly = m_initial_conditions->create_initial_condition("PressureAssembly", "cf3.solver.ActionDirector");
  m_pressure_assembly->add_tag(detail::my_tag());
  
  if(is_not_null(m_mass_matrix_assembly))
    m_initial_conditions->remove_component("MassMatrixAssembly");
  m_mass_matrix_assembly = m_initial_conditions->create_initial_condition("MassMatrixAssembly", "cf3.solver.ActionDirector");
  m_mass_matrix_assembly->add_tag(detail::my_tag());
  
  Handle<InnerLoop> inner_loop(get_child("InnerLoop"));
  if(is_not_null(m_initial_conditions->get_child("SetupInnerLoopData")))
    m_initial_conditions->remove_component("SetupInnerLoopData");
  m_initial_conditions->create_component<SetupInnerLoopData>("SetupInnerLoopData")->inner_loop = inner_loop;
  
  
  set_elements_expressions_quad();
  set_elements_expressions_triag();
}

void NavierStokesSemiImplicit::on_regions_set()
{
  BOOST_FOREACH(Component& comp, find_components_recursively_with_tag(*this, detail::my_tag()))
  {
    comp.configure_option_recursively(solver::Tags::regions(), options().option(solver::Tags::regions()).value());
    comp.configure_option_recursively(solver::Tags::physical_model(), options().option(solver::Tags::physical_model()).value());
  }
  
  if(is_not_null(m_initial_conditions))
  {
    BOOST_FOREACH(Component& comp, find_components_recursively_with_tag(*m_initial_conditions, detail::my_tag()))
    {
      comp.configure_option_recursively(solver::Tags::regions(), options().option(solver::Tags::regions()).value());
      comp.configure_option_recursively(solver::Tags::physical_model(), options().option(solver::Tags::physical_model()).value());
    }
  }
  
  Handle<math::LSS::System> p_lss(m_p_lss->get_child("LSS"));
  Handle<math::LSS::System> u_lss(m_u_lss->get_child("LSS"));
  Handle<math::LSS::System> auu_lss(m_auu_lss->get_child("LSS"));
  
  cf3_assert(p_lss->is_created());
  cf3_assert(u_lss->is_created());
  cf3_assert(auu_lss->is_created());
  
  Handle<InnerLoop> inner_loop(get_child("InnerLoop"));
  inner_loop->p_lss = p_lss;
  inner_loop->solve_p_lss->options().set("lss", p_lss);
  inner_loop->u_lss = u_lss;
  inner_loop->solve_u_lss->options().set("lss", u_lss);
  inner_loop->auu = auu_lss->matrix();
  inner_loop->lumped_m_diag = auu_lss->rhs(); // Lumped mass matrix will be stored in the auu RHS
  
  // Create vectors for temporary data
  inner_loop->u = detail::create_vector(*u_lss, "U");
  inner_loop->a = detail::create_vector(*u_lss, "A");
  inner_loop->p = detail::create_vector(*p_lss, "P");
  inner_loop->delta_a = u_lss->solution()->handle<math::LSS::ThyraVector>()->thyra_vector();
  inner_loop->delta_p_sum = detail::create_vector(*p_lss, "DeltaPSum");
  inner_loop->aup_delta_p = u_lss->rhs()->handle<math::LSS::ThyraVector>()->thyra_vector();
  
  // Make sure the terminals refer to the correct vectors
  u_vec.op.set_vector(inner_loop->u, *u_lss);
  p_vec.op.set_vector(inner_loop->p, *p_lss);
  a.op.set_vector(inner_loop->a, *u_lss);
  delta_a.op.set_vector(u_lss->solution(), *u_lss);
  delta_p.op.set_vector(p_lss->solution(), *p_lss);
  delta_p_sum.op.set_vector(inner_loop->delta_p_sum, *p_lss);
}

void NavierStokesSemiImplicit::trigger_theta()
{
  Handle<InnerLoop>(m_inner_loop)->theta = theta;
}

void NavierStokesSemiImplicit::trigger_nb_iterations()
{
  Handle<InnerLoop>(m_inner_loop)->nb_iterations = options().option("nb_iterations").value<int>();
}

void NavierStokesSemiImplicit::trigger_time()
{
  if(is_null(m_time))
    return;
  
  m_time->options().option("time_step").attach_trigger(boost::bind(&NavierStokesSemiImplicit::trigger_timestep, this));
}

void NavierStokesSemiImplicit::trigger_timestep()
{
  dt = m_time->dt();
}

} // UFEM
} // cf3
