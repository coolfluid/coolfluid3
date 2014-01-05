// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "common/Core.hpp"
#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"
#include "common/Log.hpp"
#include "common/OptionList.hpp"
#include "common/PE/Comm.hpp"
#include "common/Signal.hpp"
#include "common/Builder.hpp"
#include "common/OptionT.hpp"
#include "common/EventHandler.hpp"

#include "math/LSS/System.hpp"

#include "mesh/Region.hpp"
#include "mesh/LagrangeP0/LibLagrangeP0.hpp"
#include "mesh/LagrangeP0/Quad.hpp"
#include "mesh/LagrangeP0/Line.hpp"
#include "mesh/LagrangeP0/Triag.hpp"
#include "mesh/LagrangeP0/Hexa.hpp"
#include "mesh/LagrangeP0/Tetra.hpp"
#include "mesh/LagrangeP0/Prism.hpp"

#include "SUPG.hpp"
#include "SUPGFields.hpp"
#include "AdjacentCellToFace.hpp"
#include "Tags.hpp"

#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"
#include "solver/Tags.hpp"

namespace cf3
{

namespace UFEM
{

using namespace solver::actions::Proto;
using boost::proto::lit;

common::ComponentBuilder < SUPGFields, common::Action, LibUFEM > SUPGFields_Builder;

SUPGFields::SUPGFields(const std::string& name) :
  ProtoAction(name)
{
  options().add(solver::Tags::time(), m_time)
    .pretty_name("Time")
    .description("Component that keeps track of time for this simulation")
    .attach_trigger(boost::bind(&SUPGFields::trigger_time, this))
    .link_to(&m_time);
    
  FieldVariable<0, VectorField> u_adv("AdvectionVelocity", "linearized_velocity");
  FieldVariable<1, ScalarField> nu_eff("EffectiveViscosity", "navier_stokes_viscosity");
  FieldVariable<2, ScalarField> su_fd("tau_su", "supg", mesh::LagrangeP0::LibLagrangeP0::library_namespace());
  FieldVariable<3, ScalarField> bulk_fd("tau_bu", "supg", mesh::LagrangeP0::LibLagrangeP0::library_namespace());
  
  set_expression(elements_expression
  (
    boost::mpl::vector10<mesh::LagrangeP0::Triag, mesh::LagrangeP1::Triag2D, mesh::LagrangeP0::Quad, mesh::LagrangeP1::Quad2D, mesh::LagrangeP0::Hexa, mesh::LagrangeP1::Hexa3D, mesh::LagrangeP0::Tetra, mesh::LagrangeP1::Tetra3D, mesh::LagrangeP0::Prism, mesh::LagrangeP1::Prism3D>(),
    group
    (
      compute_tau.apply(u_adv, nu_eff, lit(m_dt), lit(tau_ps), lit(tau_su), lit(tau_bu)),
      su_fd = lit(tau_su),
      bulk_fd = lit(tau_bu)
    )
  ));
}

SUPGFields::~SUPGFields()
{
}

void SUPGFields::trigger_time()
{
  if(is_null(m_time))
      return;

  m_time->options().option("time_step").attach_trigger(boost::bind(&SUPGFields::trigger_timestep, this));
  trigger_timestep();
}

void SUPGFields::trigger_timestep()
{
  m_dt = m_time->dt();
  CFdebug << "Set SUPGFields time step to " << m_dt << CFendl;
}

} // namespace UFEM

} // namespace cf3
