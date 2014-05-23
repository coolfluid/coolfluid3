// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <cmath>

#include "Polydisperse.hpp"

#include <boost/bind.hpp>
#include <boost/function.hpp>


#include "common/Builder.hpp"
#include "common/Component.hpp"
#include "common/Group.hpp"
#include "common/OptionArray.hpp"
#include "common/OptionList.hpp"
#include "common/OptionT.hpp"
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

struct UnityCollisionKernel
{
  UnityCollisionKernel(const mesh::Field& particle_velocity_field, const std::vector<mesh::Field*>& concentration_fields, const std::vector<mesh::Field*>& weighted_volume_fields, const std::vector<mesh::Field*>& gradient_fields, const Real reference_volume, const physics::PhysModel& physical_model)
  {
  }

  Real apply(const Uint node_idx, const Uint alpha, const Uint gamma)
  {
    return 1.;
  }
};

struct NoCollisionKernel
{
  NoCollisionKernel(const mesh::Field& particle_velocity_field, const std::vector<mesh::Field*>& concentration_fields, const std::vector<mesh::Field*>& weighted_volume_fields, const std::vector<mesh::Field*>& gradient_fields, const Real reference_volume, const physics::PhysModel& physical_model)
  {
  }

  Real apply(const Uint node_idx, const Uint alpha, const Uint gamma)
  {
    return 0.;
  }
};

struct BrownianCollisionKernel
{
  BrownianCollisionKernel(const mesh::Field& particle_velocity_field, const std::vector<mesh::Field*>& concentration_fields, const std::vector<mesh::Field*>& weighted_volume_fields, const std::vector<mesh::Field*>& gradient_fields, const Real reference_volume, const physics::PhysModel& physical_model) :
    m_concentration_fields(concentration_fields),
    m_weighted_volume_fields(weighted_volume_fields),
    m_reference_volume(reference_volume),
    m_mu(physical_model.options().value<Real>("dynamic_viscosity"))
  {
  }

  Real apply(const Uint node_idx, const Uint alpha, const Uint gamma)
  {
    const Real vol_alpha = (*m_weighted_volume_fields[alpha])[node_idx][0] / (*m_concentration_fields[alpha])[node_idx][0] * m_reference_volume;
    const Real vol_gamma = (*m_weighted_volume_fields[gamma])[node_idx][0] / (*m_concentration_fields[gamma])[node_idx][0] * m_reference_volume;
    const Real r_alpha = ::pow(3./(4.*pi())*vol_alpha, 1./3.);
    const Real r_gamma = ::pow(3./(4.*pi())*vol_gamma,1./3.);
    
    // TODO: Mean free path and temperature in physics
    const Real lambda = 68e-9;
    const Real T = 293.;
    const Real A = 1.591; // TODO: Configuration for Cunningham correction
    const Real kb = 1.3806488e-23;
    const Real C_alpha = 1. + A*lambda/r_alpha;
    const Real C_gamma = 1. + A*lambda/r_gamma;
    return 2.*kb*T/(3.*m_mu) * (C_alpha/r_alpha + C_gamma/r_gamma) * (r_alpha + r_gamma);
  }
  
  const std::vector<mesh::Field*>& m_concentration_fields;
  const std::vector<mesh::Field*>& m_weighted_volume_fields;
  const Real m_reference_volume;
  const Real m_mu;
};

// Evaluation of the collision kernel based on the knowledge of the particle rate-of-strain tensor
template<Uint dim>
struct DNSCollisionKernel
{
  typedef Eigen::Matrix<Real, dim, 1> VectorT;
  typedef Eigen::Matrix<Real, dim, dim> MatrixT;

  DNSCollisionKernel(const mesh::Field& particle_velocity_field, const std::vector<mesh::Field*>& concentration_fields, const std::vector<mesh::Field*>& weighted_volume_fields, const std::vector<mesh::Field*>& gradient_fields, const Real reference_volume, const physics::PhysModel& physical_model) :
    m_particle_velocity_field(particle_velocity_field),
    m_concentration_fields(concentration_fields),
    m_weighted_volume_fields(weighted_volume_fields),
    m_gradient_fields(gradient_fields),
    m_reference_volume(reference_volume)
  {
  }

  Real apply(const Uint node_idx, const Uint alpha, const Uint gamma)
  {
    const Real vol_alpha = (*m_weighted_volume_fields[alpha])[node_idx][0] / (*m_concentration_fields[alpha])[node_idx][0] * m_reference_volume;
    const Real vol_gamma = (*m_weighted_volume_fields[gamma])[node_idx][0] / (*m_concentration_fields[gamma])[node_idx][0] * m_reference_volume;
    const Eigen::Map<VectorT const> v_alpha(&m_particle_velocity_field[node_idx][alpha*dim]);
    const Eigen::Map<VectorT const> v_gamma(&m_particle_velocity_field[node_idx][gamma*dim]);
    const Real r_alpha = ::pow(3./(4.*pi())*vol_alpha, 1./3.);
    const Real r_gamma = ::pow(3./(4.*pi())*vol_gamma,1./3.);
    const Real r_col_3 = pow_int(r_alpha + r_gamma, 3); // Collision radius ^3

    // Effect of different particle size:
    const Real beta1 = (v_alpha - v_gamma).norm() * pi() * pow_int(r_alpha + r_gamma, 2);

    // Velocity gradient tensor (transposed, but not important here)
    const Eigen::Map<MatrixT> g(&((*m_gradient_fields[gamma])[node_idx][0]));
    // Rate of strain tensor
    const MatrixT s = (g + g.transpose())/2.;
    std::vector<Real> ev(3, 0.); // vector so we can use std::sort
    Eigen::Map<VectorT>(ev.data()) = s.template selfadjointView<Eigen::Upper>().eigenvalues();
    std::sort(ev.begin(), ev.end(), std::greater<double>()); // Sort from largest to smallest
    Real beta2 = 0;
    const Real approx_zero = 1e-8;
    if(ev[2] >= -approx_zero) // All >= 0
    {
      beta2 = 0.;
    }
    else if(ev[0] < approx_zero) // All < 0
    {
      beta2 = -4./3.*pi()*r_col_3*(ev[0]+ev[1]+ev[2]);
    }
    // The above conditions ensure that always ev[0] > 0 and ev[2] < 0
    else if(::fabs(ev[1]) < approx_zero)
    {
      cf3_assert(ev[2]<0);
      const Real k1 = -ev[0]/ev[2];
      const Real sqrt_k1 = ::sqrt(k1);
      beta2 = 8.*(-ev[2])*r_col_3*( 1./3*( sqrt_k1 + (k1-1)*::atan(sqrt_k1) ) - pi()/6.*(k1-1.) );
    }
    else if(ev[1] >= 0.)
    {
      const Real k1 = -ev[0]/ev[2];
      const Real k2 = -ev[1]/ev[2];
      beta2 = 8.*(-ev[2])*r_col_3*(collision_integral(k1, k2) - pi()/6.*(k1+k2-1.));
    }
    else if(ev[1] < 0.)
    {
      cf3_assert(ev[0] > 0);
      const Real k1 = -ev[1]/ev[0];
      const Real k2 = -ev[2]/ev[0];
      beta2 = 8.*ev[0]*r_col_3*(collision_integral(k1, k2));
    }
    else
    {
      std::stringstream msg;
      msg << "Unknown combination of eigenvalues: " << ev[0] << ", " << ev[1] << ", " << ev[2];
      CFdebug << msg.str() << CFendl;
      beta2 = 0.;
    }

    return beta1 + beta2;
  }

  // Numerical integration needed for the general case
  Real collision_integral(const Real k1, const Real k2)
  {
    // The gauss points and weights
    static const Real gauss[6][2] = {
      {0.053038319518181845,0.1345579416596007},
      {0.26608552564569055,0.28334147689638234},
      {0.5979870928963115,0.36749874484146516},
      {0.9728092338985851,0.36749874484146516},
      {1.304710801149206,0.28334147689638234},
      {1.5177580072767147,0.1345579416596007}
    };

    Real result = 0;
    for(int i = 0; i != 6; ++i)
    {
      const Real phi = gauss[i][0];
      const Real cos_sin = k1*pow_int(::cos(phi),2) + k2*pow_int(::sin(phi),2);
      result += gauss[i][1] * 2./3. * ::pow(cos_sin, 1.5)/::sqrt(1 + cos_sin);
    }
    return result;
  }

  const mesh::Field& m_particle_velocity_field;
  const std::vector<mesh::Field*>& m_concentration_fields;
  const std::vector<mesh::Field*>& m_weighted_volume_fields;
  const std::vector<mesh::Field*>& m_gradient_fields;
  const Real m_reference_volume;
};

/// Combined DNS and Brownian coagulation
template<Uint dim>
struct DNSBrownianCollisionKernel
{
  DNSBrownianCollisionKernel(const mesh::Field& particle_velocity_field, const std::vector<mesh::Field*>& concentration_fields, const std::vector<mesh::Field*>& weighted_volume_fields, const std::vector<mesh::Field*>& gradient_fields, const Real reference_volume, const physics::PhysModel& physical_model) :
    m_brown(particle_velocity_field, concentration_fields, weighted_volume_fields, gradient_fields, reference_volume, physical_model),
    m_dns(particle_velocity_field, concentration_fields, weighted_volume_fields, gradient_fields, reference_volume, physical_model)
  {
  }

  Real apply(const Uint node_idx, const Uint alpha, const Uint gamma)
  {
    return ::sqrt(pow_int(m_brown.apply(node_idx, alpha, gamma), 2) + pow_int(m_dns.apply(node_idx, alpha, gamma), 2));
  }
  
  BrownianCollisionKernel m_brown;
  DNSCollisionKernel<dim> m_dns;
};

/// Functor to compute the moment source terms
template<typename CollisionKernelT>
struct MomentSourceFunctor : FunctionBase
{
  typedef void result_type;

  MomentSourceFunctor(mesh::Region& region, const std::vector<std::string>& concentration_tags, const std::vector<std::string>& weighted_volume_tags, const std::vector<std::string>& gradient_tags, const Real reference_volume, const physics::PhysModel& physical_model) :
    m_nb_phases(concentration_tags.size()),
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
      m_concentration_fields.push_back(&(dict.field(concentration_tags[i])));
      m_weighted_volume_fields.push_back(&(dict.field(weighted_volume_tags[i])));
      m_gradient_fields.push_back(&(dict.field(gradient_tags[i])));
    }
    m_beta.reset(new CollisionKernelT(dict.field("ufem_particle_velocity"), m_concentration_fields, m_weighted_volume_fields, m_gradient_fields, reference_volume, physical_model));
    m_collision_rate_field = Handle<mesh::Field>(dict.get_child("collision_rate")).get();
  }

  ~MomentSourceFunctor()
  {
    m_source_field->synchronize();
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
        const Real beta = m_beta->apply(node_idx, alpha, gamma);
//         std::cout << "computing for vol_alpha " << vol_alpha << ", vol_gamma: " << vol_gamma << ", n_alpha " << n_alpha << ", n_gamma " << n_gamma << ", beta " << beta << std::endl;
//         std::cout << "collision value: " << (beta*n_alpha*n_gamma) << std::endl;
        for(int k = 0; k != nb_moments; ++k ) // For all moments
        {
          m_rhs[k] += (0.5*pow_int(vol_alpha + vol_gamma, k ) - pow_int(vol_alpha, k )) * (beta*n_alpha*n_gamma);
        }
        if(is_not_null(m_collision_rate_field))
        {
          const Uint var_idx = alpha*m_nb_phases + gamma;
          cf3_assert(var_idx < m_collision_rate_field->row_size());
          (*m_collision_rate_field)[node_idx][var_idx] = n_alpha*n_gamma*beta;
        }
      }
    }

    Eigen::Map<RealVector> x(&(*m_source_field)[node_idx][0], nb_moments);
    
    // Check for NaN
    for(Uint k = 0; k != nb_moments; ++k)
    {
      if(!std::isfinite(m_rhs[k]))
      {
        x.setZero();
        return;
      }
    }
    
    Eigen::FullPivLU<RealMatrix> lu(m_mat);
    if(!lu.isInvertible())
    {
      Eigen::JacobiSVD<RealMatrix> svd(m_mat, Eigen::ComputeThinU | Eigen::ComputeThinV);
      x = svd.solve(m_rhs);
    }
    else
    {
      x = lu.solve(m_rhs);
    }

//    std::cout << "matrix:\n" << m_mat << std::endl;
//    std::cout << "rhs: " << m_rhs.transpose() << std::endl;
//    std::cout << "x: " << x.transpose() << std::endl;
  }

  mesh::Field* m_source_field;
  std::vector<mesh::Field*> m_concentration_fields;
  std::vector<mesh::Field*> m_weighted_volume_fields;
  std::vector<mesh::Field*> m_gradient_fields;

  const Uint m_nb_phases;
  RealMatrix m_mat;
  RealVector m_rhs;

  boost::scoped_ptr<CollisionKernelT> m_beta;
  mesh::Field* m_collision_rate_field;
};

template<typename CollisionKernelT>
void source_term_loop(mesh::Region& region, const std::vector<std::string>& concentration_tags, const std::vector<std::string>& weighted_volume_tags, const std::vector<std::string>& gradient_tags, const Real reference_volume, const physics::PhysModel& physical_model)
{
  MomentSourceFunctor<CollisionKernelT> compute_source_terms(region, concentration_tags, weighted_volume_tags, gradient_tags, reference_volume, physical_model);
  for_each_node(region, lit(compute_source_terms)(node_index));
}

}

Polydisperse::Polydisperse(const std::string& name) :
  solver::Action(name),
  m_nb_phases(0),
  m_reference_volume(1.)
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

  options().add("collision_kernel_type", "DNSCollisionKernel")
    .pretty_name("Collision Kernel Type")
    .description("Type of the collision kernel to use");

  options().add("save_collision_rate", false)
    .pretty_name("Save Collision Rate")
    .description("Save the collision rate values in a field");

  options().add("reference_volume", m_reference_volume)
    .pretty_name("Reference Volume")
    .description(("Reference volume for the particles. All particle volumes will be divided by this value, all concentrations multiplied by it"))
    .link_to(&m_reference_volume)
    .mark_basic();

  m_compute_velocities = create_static_component<solver::ActionDirector>("ComputeVelocities");
  m_concentration_solver = Handle<LSSAction>(create_component("ConcentrationSolver", "cf3.UFEM.particles.ParticleConcentration"));
  m_concentration_solver->set_solution_tag("particle_concentration_0");
  m_concentration_solver->options().set("concentration_variable", std::string("c_0"));
  m_concentration_solver->options().set("velocity_variable", std::string("vp_0"));
  m_concentration_solver->options().set("source_term_tag", std::string("moment_source_terms"));
  m_concentration_solver->options().set("source_term_variable", std::string("c_src_0"));

  m_boundary_conditions = create_static_component<common::Group>("BC");
  m_boundary_conditions->mark_basic();
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
  m_boundary_conditions->configure_option_recursively("physical_model", options()["physical_model"].value());
  m_boundary_conditions->configure_option_recursively("regions", options()["regions"].value());
  if(options().value<bool>("save_collision_rate") && !m_loop_regions.empty())
  {
    mesh::Mesh& mesh = common::find_parent_component<mesh::Mesh>(*m_loop_regions.front());
    mesh::Dictionary& dict = mesh.geometry_fields(); // TODO: Generalize to other dicts
    Handle<mesh::Field> rate_field(dict.get_child("collision_rate"));
    std::string description;
    for(Uint i = 0; i != m_nb_phases; ++i)
    {
      for(Uint j = 0; j != m_nb_phases; ++j)
      {
        if(i != 0 || j != 0)
          description += ",";
        description += "collision_rate_" + common::to_str(i) + "_" + common::to_str(j);
      }
    }
    if(is_null(rate_field))
    {
      dict.create_field("collision_rate", description);
    }
    else
    {
      cf3_always_assert(rate_field->descriptor().description() == description);
    }
  }
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
  m_gradient_tags.clear();
  
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
    m_gradient_tags.push_back("grad_vp_"+phase_label);

    const Real init_c = initial_concentrations[i];
    const Real dp = initial_diameters[i];
    const Real init_wv = init_c * (pi()/6.*dp*dp*dp);
    
    FieldVariable<0, ScalarField> c(m_concentration_variables.back(), m_concentration_tags.back());
    Handle<ProtoAction> c_action(m_ic_actions->create_initial_condition(m_concentration_tags.back(), "cf3.solver.ProtoAction"));
    c_action->set_expression(nodes_expression(c = lit(init_c)*lit(m_reference_volume)));
    
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
    tau_calc->options().set("reference_volume", m_reference_volume);
    options().option("reference_volume").link_option(tau_calc->options().option_ptr("reference_volume"));

    
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

    Handle<common::Component> vp_gradient = m_compute_velocities->create_component("ComputeGradient"+phase_label, "cf3.UFEM.VelocityGradient");
    vp_gradient->options().set("velocity_variable", m_velocity_variables.back());
    vp_gradient->options().set("velocity_tag", std::string("ufem_particle_velocity"));
    vp_gradient->options().set("gradient_name", m_velocity_variables.back());
    vp_gradient->options().set("gradient_tag", m_gradient_tags.back());

    Handle<UFEM::BoundaryConditions> c_bc = m_boundary_conditions->create_component<UFEM::BoundaryConditions>(m_concentration_variables.back());
    c_bc->mark_basic();
    c_bc->set_solution_tag(m_concentration_tags.back());

    Handle<UFEM::BoundaryConditions> wv_bc = m_boundary_conditions->create_component<UFEM::BoundaryConditions>(m_weighted_volume_variables.back());
    wv_bc->mark_basic();
    wv_bc->set_solution_tag(m_weighted_volume_tags.back());
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
  
  const std::string collision_kernel_type = options().value<std::string>("collision_kernel_type");

  BOOST_FOREACH(const Handle<mesh::Region>& region, m_loop_regions)
  {
    if(collision_kernel_type == "UnityCollisionKernel")
    {
      detail::source_term_loop<detail::UnityCollisionKernel>(*region, m_concentration_tags, m_weighted_volume_tags, m_gradient_tags, m_reference_volume, physical_model());
    }
    else if(collision_kernel_type == "DNSCollisionKernel")
    {
      if(physical_model().ndim() == 3)
      {
        detail::source_term_loop< detail::DNSCollisionKernel<3> >(*region, m_concentration_tags, m_weighted_volume_tags, m_gradient_tags, m_reference_volume, physical_model());
      }
      else if(physical_model().ndim() == 2)
      {
        detail::source_term_loop< detail::DNSCollisionKernel<2> >(*region, m_concentration_tags, m_weighted_volume_tags, m_gradient_tags, m_reference_volume, physical_model());
      }
      else
      {
        throw common::SetupError(FromHere(), "Unsupported dimension for collision kernel: " + common::to_str(physical_model().ndim()));
      }
    }
    else if(collision_kernel_type == "DNSBrownianCollisionKernel")
    {
      if(physical_model().ndim() == 3)
      {
        detail::source_term_loop< detail::DNSBrownianCollisionKernel<3> >(*region, m_concentration_tags, m_weighted_volume_tags, m_gradient_tags, m_reference_volume, physical_model());
      }
      else if(physical_model().ndim() == 2)
      {
        detail::source_term_loop< detail::DNSBrownianCollisionKernel<2> >(*region, m_concentration_tags, m_weighted_volume_tags, m_gradient_tags, m_reference_volume, physical_model());
      }
      else
      {
        throw common::SetupError(FromHere(), "Unsupported dimension for collision kernel: " + common::to_str(physical_model().ndim()));
      }
    }
    else if(collision_kernel_type == "NoCollisionKernel")
    {
      detail::source_term_loop<detail::NoCollisionKernel>(*region, m_concentration_tags, m_weighted_volume_tags, m_gradient_tags, m_reference_volume, physical_model());
    }
    else if(collision_kernel_type == "BrownianCollisionKernel")
    {
      detail::source_term_loop<detail::BrownianCollisionKernel>(*region, m_concentration_tags, m_weighted_volume_tags, m_gradient_tags, m_reference_volume, physical_model());
    }
    else
    {
      throw common::SetupError(FromHere(), "Unknown collision kernel type: " + collision_kernel_type);
    }
  }
  
  for(Uint i = 0; i != m_nb_phases; ++i)
  {
    Handle<UFEM::BoundaryConditions> bc(m_concentration_solver->get_child("BoundaryConditions"));

    bc->set_solution_tag(m_concentration_tags[i]);
    Handle<UFEM::BoundaryConditions> c_bc(m_boundary_conditions->get_child(m_concentration_variables[i]));
    c_bc->options().set("lss", m_concentration_solver->get_child("LSS"));
    bc->add_link(*c_bc);

    m_concentration_solver->set_solution_tag(m_concentration_tags[i]);
    m_concentration_solver->options().set("concentration_variable", m_concentration_variables[i]);
    m_concentration_solver->options().set("velocity_variable", m_velocity_variables[i]);
    m_concentration_solver->options().set("source_term_variable", m_concentration_src_variables[i]);
    m_concentration_solver->execute();
    bc->clear();
    
    bc->set_solution_tag(m_weighted_volume_tags[i]);
    Handle<UFEM::BoundaryConditions> zeta_bc(m_boundary_conditions->get_child(m_weighted_volume_variables[i]));
    zeta_bc->options().set("lss", m_concentration_solver->get_child("LSS"));
    bc->add_link(*zeta_bc);
    m_concentration_solver->set_solution_tag(m_weighted_volume_tags[i]);
    m_concentration_solver->options().set("concentration_variable", m_weighted_volume_variables[i]);
    m_concentration_solver->options().set("source_term_variable", m_weighted_volume_src_variables[i]);
    m_concentration_solver->execute();
    bc->clear();
  }
}


} // particles
} // UFEM
} // cf3
