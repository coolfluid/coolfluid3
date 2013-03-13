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

#include <Teko_EpetraHelpers.hpp>
#include <Teko_Utilities.hpp>

#include <Thyra_EpetraLinearOp.hpp>
#include <Thyra_VectorStdOps.hpp>

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

void write_mat(const Teuchos::RCP<const Thyra::LinearOpBase<Real> >& mat, const std::string& filename)
{
// Teuchos::RCP<std::ofstream> mat_out = Teuchos::rcp(new  std::ofstream(filename.c_str(), std::ios::out));
// Teuchos::RCP<Teuchos::FancyOStream> mat_fancy_out = Teuchos::fancyOStream(mat_out);
// Thyra::describeLinearOp(*mat, *mat_fancy_out, Teuchos::VERB_EXTREME);
}

/// Action that builds the pressure matrix, starting from blocks defined in the global system
class BuildPressureMatrix : public solver::Action
{
public:
  BuildPressureMatrix(const std::string& name) : solver::Action(name)
  {
    options().add("global_lss", m_global_lss)
      .pretty_name("Global LSS")
      .description("Global LSS with the blocks that will combine into the pressure matrix")
      .link_to(&m_global_lss);

    options().add("pressure_lss", m_pressure_lss)
      .pretty_name("Pressure LSS")
      .description("LSS for the pressure system")
      .link_to(&m_pressure_lss);
  }

  void execute()
  {
    if(is_null(m_global_lss))
      throw common::SetupError(FromHere(), "Option global_lss not set for " + uri().string());

    if(is_null(m_pressure_lss))
      throw common::SetupError(FromHere(), "Option pressure_lss not set for " + uri().string());

    Handle<math::LSS::System> global_lss(m_global_lss->get_child("LSS"));
    Handle<math::LSS::System> pressure_lss(m_pressure_lss->get_child("LSS"));

    if(is_null(global_lss))
      throw common::SetupError(FromHere(), "No LSS child for global LSS " + m_global_lss->uri().string());

    if(is_null(pressure_lss))
      throw common::SetupError(FromHere(), "No LSS child for pressure LSS " + m_pressure_lss->uri().string());

    if(!global_lss->is_created())
      throw common::SetupError(FromHere(), "Global LSS not created for " + uri().string());

    if(!pressure_lss->is_created())
      throw common::SetupError(FromHere(), "Pressure LSS not created for " + uri().string());

    const math::VariablesDescriptor& descriptor = common::find_component_with_tag<math::VariablesDescriptor>(physical_model().variable_manager(), "navier_stokes_solution");
    const Uint p_idx = descriptor.var_number("Pressure");
    const Uint u_idx = descriptor.var_number("Velocity");
    const Uint dim = descriptor.dimensionality("Velocity");
    cf3_assert(p_idx == 0 || p_idx == 1);
    cf3_assert(u_idx == 0 || u_idx == 1);
    cf3_assert(descriptor.nb_vars() == 2);
    cf3_assert(descriptor.offset(u_idx) == u_idx);

    Handle<math::LSS::TrilinosCrsMatrix> full_matrix(global_lss->matrix());
    cf3_assert(is_not_null(full_matrix));

    // Get the different blocks from the full matrix
    Teuchos::RCP<Teko::Epetra::BlockedEpetraOperator> blocked_op = math::LSS::create_teko_blocked_operator(*full_matrix, descriptor);
    Teuchos::RCP<const Thyra::LinearOpBase<Real> > Aup = Thyra::epetraLinearOp(blocked_op->GetBlock(u_idx,p_idx));
    Teuchos::RCP<const Thyra::LinearOpBase<Real> > Apu = Thyra::epetraLinearOp(blocked_op->GetBlock(p_idx,u_idx));
    Teuchos::RCP<const Thyra::LinearOpBase<Real> > App = Thyra::epetraLinearOp(blocked_op->GetBlock(p_idx,p_idx));
    Teuchos::RCP< Thyra::LinearOpBase<Real> > Ml_inv = Teko::getInvDiagonalOp(Thyra::epetraLinearOp(blocked_op->GetBlock(u_idx,u_idx))); // This is the inverse lumped mass matrix

    write_mat(Aup, "aup.txt");
    write_mat(Apu, "apu.txt");
    write_mat(App, "app.txt");
    write_mat(Ml_inv, "ml_inv.txt");

    // Form the pressure matrix
    Handle<math::LSS::TrilinosCrsMatrix> pressure_matrix(pressure_lss->matrix());
    Teuchos::RCP<const Thyra::LinearOpBase<Real> > result = Teko::explicitMultiply(Apu, Ml_inv, Aup);
    Teuchos::RCP<Epetra_CrsMatrix> crs_result = Teuchos::rcp_const_cast<Epetra_CrsMatrix>(Teuchos::rcp_dynamic_cast<const Epetra_CrsMatrix>(Teuchos::rcp_dynamic_cast<const Thyra::EpetraLinearOp>(result)->epetra_op()));
    cf3_assert(!crs_result.is_null());
    pressure_matrix->replace_epetra_matrix(crs_result); // Overwrite the result
    Teko::explicitSum(App, pressure_matrix->thyra_operator());
    write_mat(pressure_matrix->thyra_operator(), "result.txt");
  }

  static std::string type_name () { return "BuildPressureMatrix"; }

private:
  Handle<LSSActionUnsteady> m_global_lss;
  Handle<LSSActionUnsteady> m_pressure_lss;


};

ComponentBuilder < BuildPressureMatrix, common::Action, LibUFEM > BuildPressureMatrix_builder;

NavierStokesSemiImplicit::NavierStokesSemiImplicit(const std::string& name) :
  solver::ActionDirector(name),
  u("Velocity", "navier_stokes_solution"),
  p("Pressure", "navier_stokes_solution"),
  nu_eff("EffectiveViscosity", "navier_stokes_viscosity"),
  u_adv("AdvectionVelocity", "linearized_velocity"),
  a("a", "navier_stokes_acceleration"),
  delta_a_star("delta_a_star", "navier_stokes_semi_implicit"),
  delta_a("delta_a", "navier_stokes_semi_implicit"),
  dp("delta_p", "navier_stokes_delta_p"),
  u_ref("reference_velocity"),
  nu("kinematic_viscosity"),
  m_recursing(false)
{
  options().add("theta", 1.)
    .pretty_name("Theta")
    .description("Theta coefficient for the theta-method.")
    .attach_trigger(boost::bind(&NavierStokesSemiImplicit::trigger_assembly, this));

  options().add("initial_conditions", m_initial_conditions)
    .pretty_name("Initial Conditions")
    .description("The component that is used to manage the initial conditions in the solver this action belongs to")
    .link_to(&m_initial_conditions)
    .attach_trigger(boost::bind(&NavierStokesSemiImplicit::trigger_initial_conditions, this));

  m_pressure_matrix_assembly = create_component<solver::ActionDirector>("PressureMatrixAssembly");
  
  create_component<ProtoAction>("InitializeInnerLoop")->set_expression(nodes_expression(a[_i] = 0.));
  
  m_inner_loop = create_component<solver::actions::Iterate>("InnerLoop");
  m_inner_loop->options().set("max_iter", 2u);
  m_inner_loop->mark_basic();

  m_actions_to_disable.push_back(m_pressure_matrix_assembly->name());

  trigger_assembly();
}

NavierStokesSemiImplicit::~NavierStokesSemiImplicit()
{
}

void NavierStokesSemiImplicit::execute()
{
  Handle<math::LSS::System> velocity_lss(m_inner_loop->get_child("VelocitySystem")->get_child("LSS"));
  Handle<math::LSS::ZeroLSS> zero_rhs(m_inner_loop->get_child("ZeroVelocityRHS"));
  zero_rhs->options().set("lss", velocity_lss);
  solver::ActionDirector::execute();
  options().set("disabled_actions", m_actions_to_disable);
}

void NavierStokesSemiImplicit::trigger_assembly()
{
  m_pressure_matrix_assembly->clear();
  m_inner_loop->clear();
  Handle<LSSActionUnsteady> global_lss = m_pressure_matrix_assembly->create_component<LSSActionUnsteady>("GlobalLSS");
  global_lss->set_solution_tag("navier_stokes_solution");
  global_lss->options().set("matrix_builder", std::string("cf3.math.LSS.TrilinosCrsMatrix"));
  
  set_pressure_matrix_assembly_quad(*global_lss);
  
  Handle<BuildPressureMatrix> build_pressure_matrix = m_pressure_matrix_assembly->create_component<BuildPressureMatrix>("BuildPressureMatrix");
  
  // Initialize variables for this iteration
  m_inner_loop->create_component<ProtoAction>("InitializeIteration")->set_expression(nodes_expression(u_adv = u));

  // Assemble the velocity system
  Handle<LSSActionUnsteady> velocity_lss = m_inner_loop->create_component<LSSActionUnsteady>("VelocitySystem");
  velocity_lss->set_solution_tag("navier_stokes_acceleration");
  velocity_lss->create_component<math::LSS::ZeroLSS>("ZeroLSS");
  velocity_lss->mark_basic();
  velocity_lss->options().set("matrix_builder", std::string("cf3.math.LSS.TrilinosCrsMatrix"));

  set_velocity_matrix_assembly_quad(*velocity_lss);

  // Apply acceleration BC
  Handle<BoundaryConditions> u_bc =  velocity_lss->create_component<BoundaryConditions>("BC");
  u_bc->mark_basic();
  u_bc->set_solution_tag(velocity_lss->solution_tag());

  // Solve for Delta a*
  Handle<math::LSS::SolveLSS> solve_velocity_lss = velocity_lss->create_component<math::LSS::SolveLSS>("SolveLSS");

  // Set the Delta a* field
  velocity_lss->create_component<ProtoAction>("UpdateDeltaAStar")->set_expression(nodes_expression(delta_a_star = velocity_lss->solution(a)));

  // Assemble pressure RHS
  Handle<LSSActionUnsteady> pressure_lss = m_inner_loop->create_component<LSSActionUnsteady>("PressureSystem");
  pressure_lss->options().set("matrix_builder", std::string("cf3.math.LSS.TrilinosCrsMatrix"));
  //pressure_lss->options().set("solution_strategy", std::string("cf3.math.LSS.ConstantPoissonStrategy"));
  pressure_lss->set_solution_tag("navier_stokes_delta_p");
  pressure_lss->mark_basic();
  build_pressure_matrix->options().set("global_lss", global_lss);
  build_pressure_matrix->options().set("pressure_lss", pressure_lss);
  Handle<math::LSS::ZeroLSS> zero_pressure_rhs = pressure_lss->create_component<math::LSS::ZeroLSS>("ZeroPressureRHS");
  zero_pressure_rhs->options().set("reset_matrix", false);

  set_pressure_rhs_assembly_quad(*pressure_lss);

  // Apply pressure BC
  Handle<BoundaryConditions> dp_bc =  pressure_lss->create_component<BoundaryConditions>("BC");
  dp_bc->mark_basic();
  dp_bc->set_solution_tag(pressure_lss->solution_tag());

  // Solve for Delta p
  pressure_lss->create_component<math::LSS::SolveLSS>("SolveLSS");

  // Update delta_p value
  pressure_lss->create_component<ProtoAction>("Update")->set_expression(nodes_expression(dp = pressure_lss->solution(dp)));

  // Assemble new velocity RHS
  Handle<math::LSS::ZeroLSS> zero_velocity_rhs = m_inner_loop->create_component<math::LSS::ZeroLSS>("ZeroVelocityRHS");
  zero_velocity_rhs->options().set("reset_matrix", false);

  set_pressure_gradient_apply_quad(*velocity_lss);

  // Apply velocity BC
  m_inner_loop->add_link(*u_bc);

  // Solve system for Delta a - Delta a*
  m_inner_loop->add_link(*solve_velocity_lss);

  // Update solution variables
  m_inner_loop->create_component<ProtoAction>("Update")->set_expression(nodes_expression(group
  (
    delta_a = delta_a_star - velocity_lss->solution(a),
    a += delta_a,
    u += delta_a * lit(velocity_lss->dt()),
    p += dp
  )));

  
  if(is_not_null(m_physical_model))
    configure_option_recursively(solver::Tags::physical_model(), m_physical_model);

  configure_option_recursively(solver::Tags::regions(), options().option(solver::Tags::regions()).value());
}

void NavierStokesSemiImplicit::trigger_initial_conditions()
{
  // Initial condition for the viscosity, defaulting to the molecular viscosity
  Handle<ProtoAction> visc_ic(m_initial_conditions->create_initial_condition("navier_stokes_viscosity", "cf3.solver.ProtoAction"));
  visc_ic->set_expression(nodes_expression(nu_eff = nu));

  visc_ic->options().set("regions", options().option("regions").value());
}

void NavierStokesSemiImplicit::on_regions_set()
{
  if(m_recursing)
    return;

  m_recursing = true;
  options().set("disabled_actions", std::vector<std::string>());

  configure_option_recursively("regions", options().option("regions").value());
  if(is_not_null(m_initial_conditions))
  {
    Handle<ProtoAction> visc_ic(m_initial_conditions->get_child("navier_stokes_viscosity"));
    if(is_not_null(visc_ic))
      visc_ic->options().set("regions", options().option("regions").value());
  }

  m_recursing = false;
}

} // UFEM
} // cf3
