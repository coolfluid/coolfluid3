// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <cmath>

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "common/Component.hpp"
#include "common/Builder.hpp"
#include "common/OptionT.hpp"
#include "common/OptionArray.hpp"
#include "common/PropertyList.hpp"

#include "math/LSS/SolveLSS.hpp"
#include "math/LSS/ZeroLSS.hpp"
#include "mesh/LagrangeP1/ElementTypes.hpp"

#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"
#include "solver/actions/Iterate.hpp"
#include "solver/actions/NodeValence.hpp"
#include "solver/CriterionTime.hpp"
#include "solver/actions/AdvanceTime.hpp"
#include "solver/Time.hpp"
#include "solver/Tags.hpp"

#include "WALE.hpp"
#include "../Tags.hpp"

namespace cf3 {
namespace UFEM {
namespace les {

using namespace solver::actions::Proto;
using boost::proto::lit;

////////////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < WALE, common::Action, LibUFEMLES > WALE_builder;

////////////////////////////////////////////////////////////////////////////////////////////

namespace detail
{

inline void cell_sizes(const mesh::LagrangeP1::Hexa3D::NodesT& nodes, Real& d1, Real& d2, Real& d3)
{
  d1 = (nodes.row(1) - nodes.row(0)).norm();
  d2 = (nodes.row(3) - nodes.row(0)).norm();
  d3 = (nodes.row(4) - nodes.row(0)).norm();
}

inline void cell_sizes(const mesh::LagrangeP1::Prism3D::NodesT& nodes, Real& d1, Real& d2, Real& d3)
{
  d1 = (nodes.row(1) - nodes.row(0)).norm();
  d2 = (nodes.row(2) - nodes.row(0)).norm();
  d3 = (nodes.row(3) - nodes.row(0)).norm();
}

inline void cell_sizes(const mesh::LagrangeP1::Tetra3D::NodesT& nodes, Real& d1, Real& d2, Real& d3)
{
  d1 = (nodes.row(1) - nodes.row(0)).norm();
  d2 = (nodes.row(2) - nodes.row(0)).norm();
  d3 = (nodes.row(3) - nodes.row(0)).norm();
}

inline void aspect_ratios(const Real d1, const Real d2, const Real d3, Real& a1, Real& a2)
{
  if (d1 >= d2 && d1 >= d3)
  {
    a1 = d2/d1;
    a2 = d3/d1;
  }
  else if (d2 >= d1 && d2 >= d3)
  {
    a1 = d1/d2;
    a2 = d3/d2;
  }
  else
  {
    a1 = d1/d3;
    a2 = d2/d3;
  }
}
  
/// Compute the turbulent viscosity
struct ComputeNuWALE
{
  typedef void result_type;

  template<typename UT, typename NUT, typename ValenceT>
  void operator()(const UT& u, NUT& nu, const ValenceT& valence, const Real nu_visc, const Real cw) const
  {
    typedef typename UT::EtypeT ElementT;
    static const Uint dim = ElementT::dimension;
    
    typedef mesh::Integrators::GaussMappedCoords<1, ElementT::shape> GaussT;
    typedef Eigen::Matrix<Real, dim, dim> SMatT;
        
    const SMatT grad_u = u.nabla(GaussT::instance().coords.col(0))*u.value();
    const SMatT S = 0.5*(grad_u + grad_u.transpose());
    const Real S_norm2 = S.squaredNorm();
    const SMatT grad_u2 = grad_u*grad_u;

    SMatT Sd = 0.5*(grad_u2 + grad_u2.transpose());
    Sd.diagonal().array() -= grad_u2.trace()/3.;
    const Real Sd_norm2 = Sd.squaredNorm();

    // Compute the anisotropic cell size adjustment using the method of Scotti et al.
    Real d1, d2, d3, a1, a2;
    cell_sizes(u.support().nodes(), d1, d2, d3);
    aspect_ratios(d1, d2, d3, a1, a2);
    const Real log_a1 = ::log(a1);
    const Real log_a2 = ::log(a2);
    const Real f = ::cosh(::sqrt(4./27.*(log_a1*log_a1 - log_a1*log_a2 + log_a2*log_a2)));
    
    // Get the isotropic length scale
    const Real delta_iso = ::pow(u.support().volume(), 2./3.);

    Real nu_t = cw*cw*f*f*delta_iso * ::pow(Sd_norm2, 1.5) / (::pow(S_norm2, 2.5) + ::pow(Sd_norm2, 1.25));
    if(nu_t < 0. || !std::isfinite(nu_t))
      nu_t = 0.;
    
    const Eigen::Matrix<Real, ElementT::nb_nodes, 1> nodal_vals = (nu_t + nu_visc)*valence.value().array().inverse();
    nu.add_nodal_values(nodal_vals);
  }
};

static MakeSFOp<ComputeNuWALE>::type const compute_nu = {};
}

WALE::WALE(const std::string& name) :
  ProtoAction(name),
  m_cw(0.55)
{
  options().add("cw", m_cw)
    .pretty_name("Cw")
    .description("Coefficient for the WALE model.")
    .link_to(&m_cw)
    .mark_basic();

  options().add("initial_conditions", m_initial_conditions)
    .pretty_name("Initial Conditions")
    .description("The component that is used to manage the initial conditions in the solver this action belongs to")
    .link_to(&m_initial_conditions)
    .attach_trigger(boost::bind(&WALE::trigger_initial_conditions, this));
    
  options().add("velocity_tag", "navier_stokes_u_solution")
    .pretty_name("Velocity Tag")
    .description("Tag for the field containing the velocity")
    .attach_trigger(boost::bind(&WALE::trigger_set_expression, this));
    
  m_reset_viscosity = create_component<ProtoAction>("ResetViscosity");
    
  trigger_set_expression();
}

void WALE::trigger_set_expression()
{
  FieldVariable<0, VectorField> u("Velocity", options().value<std::string>("velocity_tag"));
  FieldVariable<1, ScalarField> nu_eff("EffectiveViscosity", "navier_stokes_viscosity");
  FieldVariable<2, ScalarField> valence("Valence", "node_valence");
  PhysicsConstant nu_visc("kinematic_viscosity");

  // List of applicable elements
  typedef boost::mpl::vector3<
    mesh::LagrangeP1::Hexa3D,
    mesh::LagrangeP1::Tetra3D,
    mesh::LagrangeP1::Prism3D
  > AllowedElementTypesT;

  m_reset_viscosity->set_expression(nodes_expression(nu_eff = 0.));
  set_expression(elements_expression(AllowedElementTypesT(), detail::compute_nu(u, nu_eff, valence, nu_visc, lit(m_cw))));
}

void WALE::execute()
{
  m_reset_viscosity->execute();
  ProtoAction::execute();
}


void WALE::on_regions_set()
{
  if(is_not_null(m_node_valence))
  {
    m_node_valence->options().set("regions", options().option("regions").value());
  }
  m_reset_viscosity->options().set("regions", options().option("regions").value());
}


void WALE::trigger_initial_conditions()
{
  if(is_null(m_initial_conditions))
    return;

  if(is_null(m_node_valence))
  {
    m_node_valence = m_initial_conditions->create_initial_condition("node_valence", "cf3.solver.actions.NodeValence");
    on_regions_set();
  }
}

} // les
} // UFEM
} // cf3
