// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Polydisperse.hpp"

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "common/Component.hpp"
#include "common/Builder.hpp"
#include "common/OptionList.hpp"
#include "common/OptionT.hpp"
#include "common/OptionArray.hpp"
#include "common/PropertyList.hpp"

#include "math/Consts.hpp"

#include "mesh/Field.hpp"

#include "physics/PhysModel.hpp"

#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"

#include "../Tags.hpp"

namespace cf3 {
namespace UFEM {
namespace particles {

using namespace solver::actions::Proto;
using boost::proto::lit;
using math::Consts::pi;

////////////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < Polydisperse, common::Action, LibUFEMParticles > Polydisperse_builder;

////////////////////////////////////////////////////////////////////////////////////////////

namespace detail
{

Real pow_int(const Real in, const int pow)
{
  Real result = 1.;
  for(int i = 0; i < pow; ++i)
    result *= in;
  return result;
}

/// Functor to compute the moment source terms
struct MomentSourceFunctor : FunctionBase
{
  typedef void result_type;

  MomentSourceFunctor(mesh::Region& region, const std::vector<std::string>& concentration_names, const std::vector<std::string>& weighted_volume_names) :
    m_nb_phases(concentration_names.size()),
    m_mat(2*m_nb_phases, 2*m_nb_phases),
    m_rhs(2*m_nb_phases)
  {
    mesh::Mesh& mesh = common::find_parent_component<mesh::Mesh>(region);
    mesh::Dictionary& dict = mesh.geometry_fields(); // TODO: Generalize to other dicts
    m_source_field = &(dict.field("moment_source_terms"));

    m_concentration_fields.reserve(m_nb_phases);
    m_weighted_volume_fields.reserve(m_nb_phases);
    for(Uint i = 0; i != m_nb_phases; ++i)
    {
      m_concentration_fields.push_back(&(dict.field(concentration_names[i])));
      m_weighted_volume_fields.push_back(&(dict.field(weighted_volume_names[i])));
    }
  }

  void operator()(const Uint node_idx)
  {
    const Uint nb_moments = 2*m_nb_phases;
    m_rhs.setZero();
    for(Uint alpha = 0; alpha != m_nb_phases; ++alpha) // For all phases
    {
      const Real n_alpha = (*m_concentration_fields[alpha])[node_idx][0];
      const Real vol_alpha = (*m_weighted_volume_fields[alpha])[node_idx][0] / n_alpha;
      for(int ki = 0; ki != nb_moments; ++ki) // For all moments
      {
        const Real k = static_cast<Real>(ki);
        m_mat(ki, alpha) = (1.-k) * pow_int(vol_alpha, ki);
        m_mat(ki, alpha+m_nb_phases) = k * pow_int(vol_alpha, ki-1);
      }
      for(Uint gamma = 0; gamma != m_nb_phases; ++gamma)
      {
        const Real n_gamma = (*m_concentration_fields[gamma])[node_idx][0];
        const Real vol_gamma = (*m_weighted_volume_fields[gamma])[node_idx][0] / n_gamma;
        for(int k = 0; k != nb_moments; ++k ) // For all moments
        {
          m_rhs[k] += (0.5*pow_int(vol_alpha + vol_gamma, k ) - pow_int(vol_alpha, k )) * (1.*n_alpha*n_gamma);
        }
      }
    }
    
//     std::cout << "source mat:\n" << m_mat << std::endl;
//     std::cout << "source rhs: " << m_rhs.transpose() << std::endl;
    
    Eigen::Map<RealVector> x(&(*m_source_field)[node_idx][0], nb_moments);
    Eigen::FullPivLU<RealMatrix> lu(m_mat);
    if(!lu.isInvertible())
      throw common::FloatingPointError(FromHere(), "Matrix is not invertible.");
    x = lu.solve(m_rhs);
//     std::cout << "source sol: " << x.transpose() << std::endl;
  }

  mesh::Field* m_source_field;
  std::vector<mesh::Field*> m_concentration_fields;
  std::vector<mesh::Field*> m_weighted_volume_fields;

  const Uint m_nb_phases;
  RealMatrix m_mat;
  RealVector m_rhs;
};

}

Polydisperse::Polydisperse(const std::string& name) :
  solver::Action(name),
  m_nb_phases(0)
{
  options().add("velocity_tag", "navier_stokes_solution")
    .pretty_name("Velocity Tag")
    .description("Tag for the field containing the velocity");

  options().add("velocity_variable", "Velocity")
    .pretty_name("Velocity Variable")
    .description("Name for the velocity variable");

  options().add("nb_phases", m_nb_phases)
    .pretty_name("Number of phases")
    .description("Number of particle sizes to transport")
    .link_to(&m_nb_phases)
    .attach_trigger(boost::bind(&Polydisperse::trigger_nb_phases, this))
    .mark_basic();

  options().add("initial_diameters", std::vector<Real>())
    .pretty_name("Initial Diameters")
    .description("Initial particle diameters for each particle size")
    .mark_basic();

  options().add("initial_concentrations", std::vector<Real>())
    .pretty_name("Initial Concentrations")
    .description("Initial concentrations for the particle phases")
    .mark_basic();

  options().add("initial_conditions", m_initial_conditions)
    .pretty_name("Initial Conditions")
    .description("The component that is used to manage the initial conditions in the solver this action belongs to")
    .link_to(&m_initial_conditions)
    .attach_trigger(boost::bind(&Polydisperse::trigger_initial_conditions, this));

  m_compute_velocities = create_static_component<solver::ActionDirector>("ComputeVelocities");
  m_concentration_solver = Handle<LSSAction>(create_component("ConcentrationSolver", "cf3.UFEM.particles.ParticleConcentration"));
  m_concentration_solver->set_solution_tag("particle_concentration_0");
  m_concentration_solver->options().set("concentration_variable", std::string("c_0"));
  m_concentration_solver->options().set("velocity_variable", std::string("vp_0"));
  m_concentration_solver->options().set("source_term_tag", std::string("moment_source_terms"));
  m_concentration_solver->options().set("source_term_variable", std::string("c_src_0"));
}

Polydisperse::~Polydisperse()
{
}


void Polydisperse::on_regions_set()
{
  m_compute_velocities->configure_option_recursively("physical_model", options()["physical_model"].value());
  m_compute_velocities->configure_option_recursively("regions", options()["regions"].value());
  m_ic_actions->configure_option_recursively("physical_model", options()["physical_model"].value());
  m_ic_actions->configure_option_recursively("regions", options()["regions"].value());
  m_compute_velocities->configure_option_recursively("initial_conditions", options()["initial_conditions"].value());
  m_concentration_solver->configure_option_recursively("physical_model", options()["physical_model"].value());
  m_concentration_solver->configure_option_recursively("regions", options()["regions"].value());
}

void Polydisperse::trigger_nb_phases()
{
  const std::vector<Real> initial_diameters = options().value< std::vector<Real> >("initial_diameters");
  if(initial_diameters.size() != m_nb_phases)
    throw common::SetupError(FromHere(), "Wrong number of initial diameters");

  const std::vector<Real> initial_concentrations = options().value< std::vector<Real> >("initial_concentrations");
  if(initial_concentrations.size() != m_nb_phases)
    throw common::SetupError(FromHere(), "Wrong number of initial concentrations");

  m_concentration_tags.clear();
  m_concentration_variables.clear();
  m_weighted_volume_tags.clear();
  m_weighted_volume_variables.clear();
  m_tau_variables.clear();
  m_velocity_variables.clear();
  m_concentration_src_variables.clear();
  m_weighted_volume_src_variables.clear();
  
  m_ic_actions->clear();

  for(Uint i = 0; i != m_nb_phases; ++i)
  {
    const std::string phase_label = common::to_str(i);
    m_concentration_tags.push_back("particle_concentration_"+phase_label);
    m_concentration_variables.push_back("c_"+phase_label);
    m_weighted_volume_tags.push_back("weighted_particle_volume_"+phase_label);
    m_weighted_volume_variables.push_back("zeta_"+phase_label);
    m_tau_variables.push_back("tau_"+phase_label);
    m_velocity_variables.push_back("vp_"+phase_label);
    m_concentration_src_variables.push_back("c_src_"+phase_label);
    m_weighted_volume_src_variables.push_back("zeta_src_"+phase_label);

    const Real init_c = initial_concentrations[i];
    const Real dp = initial_diameters[i];
    const Real init_wv = init_c * (pi()/6.*dp*dp*dp);
    
    FieldVariable<0, ScalarField> c(m_concentration_variables.back(), m_concentration_tags.back());
    Handle<ProtoAction> c_action(m_ic_actions->create_initial_condition(m_concentration_tags.back(), "cf3.solver.ProtoAction"));
    c_action->set_expression(nodes_expression(c = lit(init_c)));
    
    FieldVariable<1, ScalarField> wv(m_weighted_volume_variables.back(), m_weighted_volume_tags.back());
    Handle<ProtoAction> wv_action(m_ic_actions->create_initial_condition(m_weighted_volume_tags.back(), "cf3.solver.ProtoAction"));
    wv_action->set_expression(nodes_expression(wv = lit(init_wv)));
    
    FieldVariable<2, ScalarField> c_src(m_concentration_src_variables.back(), "moment_source_terms");
    Handle<ProtoAction> c_src_action(m_ic_actions->create_initial_condition(m_concentration_tags.back(), "cf3.solver.ProtoAction"));
    c_src_action->set_expression(nodes_expression(c_src = lit(0.)));
    
    Handle<common::Component> tau_calc = m_compute_velocities->create_component("ComputeTau"+phase_label, "cf3.UFEM.particles.RelaxationTime");
    tau_calc->options().set("concentration_tag", m_concentration_tags.back());
    tau_calc->options().set("concentration_variable", m_concentration_variables.back());
    tau_calc->options().set("weighted_volume_tag", m_weighted_volume_tags.back());
    tau_calc->options().set("weighted_volume_variable", m_weighted_volume_variables.back());
    tau_calc->options().set("tau_variable", m_tau_variables.back());
    
    Handle<common::Component> eq_euler = m_compute_velocities->create_component("ComputeVelocity"+phase_label, "cf3.UFEM.particles.EquilibriumEuler");
    options().option("velocity_tag").link_option(eq_euler->options().option_ptr("velocity_tag"));
    eq_euler->options().set("velocity_tag", options().value<std::string>("velocity_tag"));
    options().option("velocity_variable").link_option(eq_euler->options().option_ptr("velocity_variable"));
    eq_euler->options().set("velocity_variable", options().value<std::string>("velocity_variable"));
    eq_euler->options().set("particle_velocity_variable", m_velocity_variables.back());
    eq_euler->options().set("tau_variable", m_tau_variables.back());
    if(i != 0)
    {
      eq_euler->options().set("compute_gradient", false);
    }
  }
  // Outside the loop for per-block ordering
  for(Uint i = 0; i != m_nb_phases; ++i)
  {
    FieldVariable<3, ScalarField> wv_src(m_weighted_volume_src_variables[i], "moment_source_terms");
    Handle<ProtoAction> wv_src_action(m_ic_actions->create_initial_condition(m_weighted_volume_tags[i], "cf3.solver.ProtoAction"));
    wv_src_action->set_expression(nodes_expression(wv_src = lit(0.)));
  }
}

void Polydisperse::trigger_initial_conditions()
{
  m_ic_actions = Handle<InitialConditions>(m_initial_conditions->create_initial_condition("Polydisperse", "cf3.UFEM.InitialConditions"));
  m_compute_velocities->configure_option_recursively("initial_conditions", options()["initial_conditions"].value());
}

void Polydisperse::execute()
{
  m_compute_velocities->execute();
  
  BOOST_FOREACH(const Handle<mesh::Region>& region, m_loop_regions)
  {
    detail::MomentSourceFunctor compute_source_terms(*region, m_concentration_tags, m_weighted_volume_tags);
    for_each_node(*region, lit(compute_source_terms)(node_index));
  }
  
  for(Uint i = 0; i != m_nb_phases; ++i)
  {
    m_concentration_solver->set_solution_tag(m_concentration_tags[i]);
    m_concentration_solver->options().set("concentration_variable", m_concentration_variables[i]);
    m_concentration_solver->options().set("velocity_variable", m_velocity_variables[i]);
    m_concentration_solver->options().set("source_term_variable", m_concentration_src_variables[i]);
    m_concentration_solver->execute();
//     std::cout << "concentration system for phase " << i << ":" << std::endl;
//     Handle<math::LSS::System>(m_concentration_solver->get_child("LSS"))->print(std::cout);
    
    m_concentration_solver->set_solution_tag(m_weighted_volume_tags[i]);
    m_concentration_solver->options().set("concentration_variable", m_weighted_volume_variables[i]);
    m_concentration_solver->options().set("source_term_variable", m_weighted_volume_src_variables[i]);
    m_concentration_solver->execute();
//     std::cout << "weighted volume system for phase " << i << ":" << std::endl;
//     Handle<math::LSS::System>(m_concentration_solver->get_child("LSS"))->print(std::cout);
  }
}


} // particles
} // UFEM
} // cf3
