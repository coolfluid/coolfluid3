// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "../NavierStokesSemiImplicit.hpp"
#include "../PressureSystem.hpp"

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include <boost/mpl/back_inserter.hpp>
#include <boost/mpl/copy.hpp>

#include <Thyra_DefaultDiagonalLinearOp.hpp>
#include <Thyra_DefaultScaledAdjointLinearOp.hpp>
#include <Thyra_DefaultSpmdMultiVector.hpp>
#include <Thyra_VectorStdOps.hpp>
#include <Thyra_MultiVectorStdOps.hpp>
#include <Thyra_VectorBase.hpp>

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

#include "solver/ActionDirectorWithSkip.hpp"
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

//void write_mat(const Teuchos::RCP<const Thyra::LinearOpBase<Real> >& mat, const std::string& filename)
//{
//  Teuchos::RCP<std::ofstream> mat_out = Teuchos::rcp(new  std::ofstream(filename.c_str(), std::ios::out));
//  Teuchos::RCP<Teuchos::FancyOStream> mat_fancy_out = Teuchos::fancyOStream(mat_out);
//  Thyra::describeLinearOp(*mat, *mat_fancy_out, Teuchos::VERB_EXTREME);
//}

//void write_vec(const Teuchos::RCP<const Thyra::MultiVectorBase<Real> >& vec, const std::string& filename)
//{
//  Teuchos::RCP<std::ofstream> mat_out = Teuchos::rcp(new  std::ofstream(filename.c_str(), std::ios::out));
//  Teuchos::RCP<Teuchos::FancyOStream> mat_fancy_out = Teuchos::fancyOStream(mat_out);
//  vec->describe(*mat_fancy_out, Teuchos::VERB_EXTREME);
//}

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
    for(Uint i = 0; i != nb_iterations; ++i)
    {
      u_lss->rhs()->reset(0.);

      // Computing m_velocity_assembly in the loop. Modified for the ABL implementation.
      u_lss->matrix()->reset(0.);
      m_velocity_assembly->execute();

      // Velocity system: compute delta_a_star
      m_u_rhs_assembly->execute();
      if(i == 0) // Apply velocity BC the first inner iteration
      {
        u_lss->rhs()->scale(m_time->dt());
        velocity_bc->execute();
        // The velocity BC deals with velocity, so we need to write this in terms of acceleration
        u_lss->rhs()->scale(1./m_time->dt());
      }
      else // Zero boundary condition after first pass. Modified for the ABL implementation.
      {
        velocity_bc->execute();
        u_lss->dirichlet_apply(true, true);
      }

      inner_bc->execute(); // Inner Boundary Condition for the RHS implementation of ABL.
      // u_lss->matrix()->print(std::cout); // Print m_velocity_assembly

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
      if(i == 0 && is_not_null(m_p_strategy_first))
      {
        p_lss->options().set("solution_strategy_component", m_p_strategy_first);
      }
      else if (i > 0 && is_not_null(m_p_strategy_second))
      {
        p_lss->options().set("solution_strategy_component", m_p_strategy_second);
      }
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
  
  Handle<math::LSS::Vector> lumped_m_diag;

  Handle<common::Action> m_velocity_assembly;
  
  Handle<common::Action> pressure_bc;
  Handle<common::Action> velocity_bc;
  Handle<common::Action> inner_bc;

  int nb_iterations;

  Teuchos::RCP<const Thyra::LinearOpBase<Real> > lumped_m_op;

  Handle< math::LSS::Vector > u;
  Handle< math::LSS::Vector > a;
  Handle< math::LSS::Vector > p;
  Teuchos::RCP<Thyra::VectorBase<Real> > delta_a;
  Teuchos::RCP<Thyra::VectorBase<Real> > aup_delta_p; // This is actually u_lss->rhs()
  Handle< math::LSS::Vector > delta_p_sum;
  
  Handle<solver::Time> m_time;
  Real theta;

  // These are used when alternating the solution strategies between predictor and corrector steps
  Handle<math::LSS::SolutionStrategy> m_p_strategy_first;
  Handle<math::LSS::SolutionStrategy> m_p_strategy_second;

private:
  Handle<solver::ActionDirector> m_u_rhs_assembly;
  Handle<solver::ActionDirector> m_p_rhs_assembly;
  Handle<solver::ActionDirector> m_apply_aup;
};

ComponentBuilder < InnerLoop, common::Action, LibUFEM > InnerLoop_builder;

/// Initialize inner loop data
struct SetupInnerLoopData : solver::Action
{
  SetupInnerLoopData ( const std::string& name ) : solver::Action(name)
  {
  }

  static std::string type_name() { return "SetupInnerLoopData"; }

  void execute()
  {
    // We use the velocity RHS to constrct the lumped mass matrix during assembly
    inner_loop->lumped_m_diag->assign(*inner_loop->u_lss->rhs());
    Handle<math::LSS::ThyraVector> lumped_diag(inner_loop->lumped_m_diag);
    Teuchos::RCP< Thyra::VectorBase<Real> > lumped_inv_diag = lumped_diag->thyra_vector();
    Thyra::reciprocal(*lumped_inv_diag, lumped_inv_diag.ptr());
    // Construct the diagonal op
    inner_loop->lumped_m_op = Thyra::diagonal(lumped_inv_diag);
    inner_loop->lumped_m_diag->sync();
    
    // Set up pressure RCG, if needed
    if(is_not_null(inner_loop->m_p_strategy_first) && is_not_null(inner_loop->m_p_strategy_second))
    {
      inner_loop->m_p_strategy_first->set_matrix(inner_loop->p_lss->matrix());
      inner_loop->m_p_strategy_first->set_solution(inner_loop->p_lss->solution());
      inner_loop->m_p_strategy_first->set_rhs(inner_loop->p_lss->rhs());
      inner_loop->m_p_strategy_first->mark_basic();

      inner_loop->m_p_strategy_second->set_matrix(inner_loop->p_lss->matrix());
      inner_loop->m_p_strategy_second->set_solution(inner_loop->p_lss->solution());
      inner_loop->m_p_strategy_second->set_rhs(inner_loop->p_lss->rhs());
      inner_loop->m_p_strategy_second->mark_basic();
    }
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
  nu("kinematic_viscosity")
{
  const std::vector<std::string> restart_field_tags = boost::assign::list_of("navier_stokes_u_solution")("navier_stokes_p_solution")("linearized_velocity")("navier_stokes_viscosity");
  properties().add("restart_field_tags", restart_field_tags);
  
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

  options().add("pressure_rcg_solve", false)
    .pretty_name("Pressure RCG Solve")
    .description("Use alternating Recycling Conjugate Gradients for the pressure system solution");

  options().add("enable_body_force", false)
    .pretty_name("Enable Force Term")
    .description("Activate the volume force term")
    .attach_trigger(boost::bind(&NavierStokesSemiImplicit::trigger_reset_assembly, this))
    .mark_basic();

  options().add("enable_boussinesq", false)
    .pretty_name("Enable Boussinesq")
    .description("Activate the Boussinesq bouyancy approximation")
    .attach_trigger(boost::bind(&NavierStokesSemiImplicit::trigger_reset_assembly, this))
    .mark_basic();
    
  options().add("alpha_ps", compute_tau.data.op.alpha_ps)
    .pretty_name("alpha_ps")
    .description("Constant to multiply the PSPG parameter with.")
    .link_to(&(compute_tau.data.op.alpha_ps));
      
  options().add("alpha_su", compute_tau.data.op.alpha_su)
    .pretty_name("alpha_su")
    .description("Constant to multiply the SUPG parameter with.")
    .link_to(&(compute_tau.data.op.alpha_su));
      
  options().add("alpha_bu", compute_tau.data.op.alpha_bu)
    .pretty_name("alpha_bu")
    .description("Constant to multiply the Bulk parameter with.")
    .link_to(&(compute_tau.data.op.alpha_bu));
    
  options().add("supg_type", compute_tau.data.op.supg_type_str)
    .pretty_name("SUPG Type")
    .description("Type of computation for the stabilization coefficients.")
    .link_to(&(compute_tau.data.op.supg_type_str))
    .attach_trigger(boost::bind(&ComputeTauImpl::trigger_supg_type, &compute_tau.data.op));
    
  options().add("c1", compute_tau.data.op.c1)
    .pretty_name("c1")
    .description("Constant adjusting the time part of SUPG in the metric tensor formulation")
    .link_to(&(compute_tau.data.op.c1));
    
  options().add("c2", compute_tau.data.op.c2)
    .pretty_name("c2")
    .description("Constant adjusting the time part of SUPG in the metric tensor formulation")
    .link_to(&(compute_tau.data.op.c2));
    
  options().add("u_ref", compute_tau.data.op.u_ref)
    .pretty_name("Reference velocity")
    .description("Reference velocity for the CF2 SUPG method")
    .link_to(&(compute_tau.data.op.u_ref));

  add_component(create_proto_action("LinearizeU", nodes_expression(u_adv = 2.1875*u - 2.1875*u1 + 1.3125*u2 - 0.3125*u3)));
  get_child("LinearizeU")->add_tag(detail::my_tag());

  m_p_lss = create_component<PressureSystem>("PressureLSS");
  m_p_lss->mark_basic();
  m_p_lss->set_solution_tag("navier_stokes_p_solution");
  m_p_lss->add_tag(detail::my_tag());
  m_p_lss->options().set("matrix_builder", std::string("cf3.math.LSS.TrilinosCrsMatrix"));

  // Assembly of the velocity system matrices
  m_u_lss = create_component<LSSAction>("VelocityLSS");
  m_u_lss->mark_basic();
  m_velocity_assembly = m_u_lss->create_component<solver::ActionDirectorWithSkip>("Assembly");
  m_velocity_assembly->add_tag(detail::my_tag());
  m_velocity_assembly->mark_basic();
  m_u_lss->set_solution_tag("navier_stokes_u_solution");
  m_u_lss->add_tag(detail::my_tag());
  m_u_lss->options().set("matrix_builder", std::string("cf3.math.LSS.TrilinosCrsMatrix"));

  // Boundary conditions
  Handle<BoundaryConditions> pressure_bc =  m_p_lss->create_component<BoundaryConditions>("BC");
  pressure_bc->mark_basic();
  pressure_bc->set_solution_tag("navier_stokes_p_solution");
  m_p_lss->options().set("disabled_actions", std::vector<std::string>(1, "BC")); // Disabled because we execute it from the inner loop
  
  Handle<BoundaryConditions> velocity_bc =  m_u_lss->create_component<BoundaryConditions>("BC");
  velocity_bc->mark_basic();
  velocity_bc->set_solution_tag("navier_stokes_u_solution");
  Handle<BoundaryConditions> inner_bc =  m_u_lss->create_component<BoundaryConditions>("InnerBC");
  inner_bc->mark_basic();
  inner_bc->set_solution_tag("navier_stokes_u_solution");
  m_u_lss->options().set("disabled_actions", std::vector<std::string>({"BC", "InnerBC", "Assembly"})); // Assembly disabled because we execute it from the inner loop

  // Copy the current solution to the solution vectors
  create_component<ProtoAction>("SetSolution")->set_expression(nodes_expression(group(m_u_lss->solution(u) = u, m_p_lss->solution(p) = p)));
  get_child("SetSolution")->add_tag(detail::my_tag());
  
  // Solve the systems iteratively
  m_inner_loop = create_component<InnerLoop>("InnerLoop");
  m_inner_loop->mark_basic();
  m_inner_loop->add_tag(detail::my_tag());
  Handle<InnerLoop>(m_inner_loop)->pressure_bc = pressure_bc;
  Handle<InnerLoop>(m_inner_loop)->velocity_bc = velocity_bc;
  Handle<InnerLoop>(m_inner_loop)->inner_bc = inner_bc;
  Handle<InnerLoop>(m_inner_loop)->m_velocity_assembly = Handle<common::Action>(m_velocity_assembly); // Give m_velocity_assembly the properties of an Action.

  // Update the solution
  create_component<ProtoAction>("Update")->set_expression(nodes_expression(group(u3 = u2, u2 = u1, u1 = u, u = m_u_lss->solution(u), p = m_p_lss->solution(p))));
  get_child("Update")->add_tag(detail::my_tag());

  create_component("ComputeCFL", "cf3.UFEM.ComputeCFL")->add_tag(detail::my_tag());;
  
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

  // Component where the user can add his own initial conditions
  Handle<InitialConditions> ns_ic(m_initial_conditions->create_initial_condition("NavierStokes", "cf3.UFEM.InitialConditions"));
  ns_ic->mark_basic();
  ns_ic->add_tag(detail::my_tag());

  // Use a proto action to set the linearized_velocity easily
  Handle<ProtoAction> lin_vel_ic (m_initial_conditions->create_initial_condition("linearized_velocity", "cf3.solver.ProtoAction"));
  lin_vel_ic->set_expression(nodes_expression(group(u_adv = u, u1 = u, u2 = u, u3 = u)));
  lin_vel_ic->add_tag(detail::my_tag());

  // This is the place to add any readers for restarting
  Handle<InitialConditions> restart_ic(m_initial_conditions->create_initial_condition("Restarts", "cf3.UFEM.InitialConditions"));
  restart_ic->mark_basic();
  restart_ic->add_tag(detail::my_tag());

  Handle<math::LSS::System> u_lss(m_u_lss->get_child("LSS"));
  
  if(is_not_null(m_pressure_assembly))
    m_initial_conditions->remove_component("PressureAssembly");
  m_pressure_assembly = m_initial_conditions->create_initial_condition("PressureAssembly", "cf3.UFEM.PressureSystemAssembly");
  m_pressure_assembly->options().set("pressure_lss_action", m_p_lss);
  m_pressure_assembly->add_tag(detail::my_tag());
  m_pressure_assembly->options().set("theta", theta);
  
  m_pressure_assembly->options().set("alpha_ps", options().value<Real>("alpha_ps"));
  options().option("alpha_ps").link_option(m_pressure_assembly->options().option_ptr("alpha_ps"));
  m_pressure_assembly->options().set("supg_type", options().value<std::string>("supg_type"));
  options().option("supg_type").link_option(m_pressure_assembly->options().option_ptr("supg_type"));
  m_pressure_assembly->options().set("c1", options().value<Real>("c1"));
  options().option("c1").link_option(m_pressure_assembly->options().option_ptr("c1"));
  m_pressure_assembly->options().set("c2", options().value<Real>("c2"));
  options().option("c2").link_option(m_pressure_assembly->options().option_ptr("c2"));
  m_pressure_assembly->options().set("u_ref", options().value<Real>("u_ref"));
  options().option("u_ref").link_option(m_pressure_assembly->options().option_ptr("u_ref"));
  
  if(is_not_null(m_mass_matrix_assembly))
    m_initial_conditions->remove_component("MassMatrixAssembly");
  m_initial_conditions->create_component("ZeroVelocitySystem", "cf3.math.LSS.ZeroLSS")->options().set("lss", u_lss);
  m_mass_matrix_assembly = m_initial_conditions->create_initial_condition("MassMatrixAssembly", "cf3.solver.ActionDirector");
  m_mass_matrix_assembly->add_tag(detail::my_tag());
  
  Handle<InnerLoop> inner_loop(get_child("InnerLoop"));
  if(is_not_null(m_initial_conditions->get_child("SetupInnerLoopData")))
    m_initial_conditions->remove_component("SetupInnerLoopData");
  Handle<SetupInnerLoopData> setup_inner = m_initial_conditions->create_component<SetupInnerLoopData>("SetupInnerLoopData");
  setup_inner->inner_loop = inner_loop;
  
  trigger_reset_assembly();
}

void NavierStokesSemiImplicit::on_regions_set()
{
  BOOST_FOREACH(Component& comp, find_components_recursively_with_tag(*this, detail::my_tag()))
  {
    comp.configure_option_recursively(solver::Tags::regions(), options().option(solver::Tags::regions()).value());
    comp.configure_option_recursively(solver::Tags::physical_model(), options().option(solver::Tags::physical_model()).value());
  }
  
  Handle<math::LSS::System> p_lss(m_p_lss->get_child("LSS"));
  Handle<math::LSS::System> u_lss(m_u_lss->get_child("LSS"));
  
  cf3_assert(p_lss->is_created());
  cf3_assert(u_lss->is_created());
  
  if(is_not_null(m_initial_conditions))
  {
    BOOST_FOREACH(Component& comp, find_components_recursively_with_tag(*m_initial_conditions, detail::my_tag()))
    {
      comp.configure_option_recursively(solver::Tags::regions(), options().option(solver::Tags::regions()).value());
      comp.configure_option_recursively(solver::Tags::physical_model(), options().option(solver::Tags::physical_model()).value());
    }
    
    m_initial_conditions->get_child("ZeroVelocitySystem")->options().set("lss", u_lss);
  }
  
  Handle<InnerLoop> inner_loop(get_child("InnerLoop"));
  inner_loop->p_lss = p_lss;
  inner_loop->solve_p_lss->options().set("lss", p_lss);
  inner_loop->u_lss = u_lss;
  inner_loop->solve_u_lss->options().set("lss", u_lss);
  inner_loop->lumped_m_diag = detail::create_vector(*u_lss, "LumpedDiag");
  
  // Create vectors for temporary data
  inner_loop->u = detail::create_vector(*u_lss, "U");
  inner_loop->a = detail::create_vector(*u_lss, "A");
  inner_loop->p = detail::create_vector(*p_lss, "P");
  inner_loop->delta_a = u_lss->solution()->handle<math::LSS::ThyraVector>()->thyra_vector();
  inner_loop->delta_p_sum = detail::create_vector(*p_lss, "DeltaPSum");
  inner_loop->aup_delta_p = u_lss->rhs()->handle<math::LSS::ThyraVector>()->thyra_vector();
  
  mesh::Dictionary& dict = common::find_parent_component<mesh::Mesh>(*m_loop_regions.front()).geometry_fields();
  Handle< common::Table<Real> > coordinates(dict.coordinates().handle());
  Handle< common::List<Uint> > used_nodes(m_p_lss->get_child(mesh::Tags::nodes_used()));
  
  cf3_assert(is_not_null(coordinates));
  cf3_assert(is_not_null(used_nodes));
  
  std::vector<bool> periodic_links_active_vec;

  Handle< List<bool> > periodic_links_active_h(dict.get_child("periodic_links_active"));
  if(is_not_null(periodic_links_active_h))
  {
    const List<bool>& periodic_links_active = *periodic_links_active_h;
    const List<Uint>& used_nodes_list = *used_nodes;
    const Uint nb_used_nodes = used_nodes_list.size();
    periodic_links_active_vec.resize(nb_used_nodes, false);
    for(Uint i = 0; i != nb_used_nodes; ++i)
    {
      if(periodic_links_active[used_nodes_list[i]])
      {
        periodic_links_active_vec[i] = true;
      }
    }
  }
  
  if(options().value<bool>("pressure_rcg_solve"))
  {
    inner_loop->m_p_strategy_first = Handle<math::LSS::SolutionStrategy>(inner_loop->parent()->create_component("FirstPressureStrategy", "cf3.math.LSS.RCGStrategy"));
    inner_loop->m_p_strategy_first->set_coordinates(*Handle<PE::CommPattern>(m_p_lss->get_child("CommPattern")), *coordinates, *used_nodes, periodic_links_active_vec);
    inner_loop->m_p_strategy_second = Handle<math::LSS::SolutionStrategy>(inner_loop->parent()->create_component("SecondPressureStrategy", "cf3.math.LSS.RCGStrategy"));
    inner_loop->m_p_strategy_second->set_coordinates(*Handle<PE::CommPattern>(m_p_lss->get_child("CommPattern")), *coordinates, *used_nodes, periodic_links_active_vec);
  }
  
  if(p_lss->solution_strategy()->options().check("coordinates") && p_lss->solution_strategy()->options().check("used_nodes"))
  {
    p_lss->solution_strategy()->options().set("coordinates", coordinates);
    p_lss->solution_strategy()->options().set("used_nodes", used_nodes);
  }
  
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
  if(is_not_null(m_pressure_assembly))
    m_pressure_assembly->options().set("theta", theta);
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

void NavierStokesSemiImplicit::trigger_reset_assembly()
{
  m_mass_matrix_assembly->clear();
  m_velocity_assembly->clear();
  m_inner_loop->get_child("URHSAssembly")->clear();
  m_inner_loop->get_child("PRHSAssembly")->clear();
  m_inner_loop->get_child("ApplyAup")->clear();
  
  set_elements_expressions_quad();
  set_elements_expressions_triag();
  set_elements_expressions_hexa();
  set_elements_expressions_prism();
  set_elements_expressions_tetra();
}

} // UFEM
} // cf3
