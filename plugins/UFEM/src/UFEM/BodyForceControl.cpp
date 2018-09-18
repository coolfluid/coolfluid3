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

#include "BodyForceControl.hpp"
#include "AdjacentCellToFace.hpp"
#include "Tags.hpp"

#include "solver/Time.hpp"

#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"
#include "solver/actions/Proto/SurfaceIntegration.hpp"
#include "solver/Tags.hpp"

namespace cf3
{

namespace UFEM
{

using namespace solver::actions::Proto;
using boost::proto::lit;

////////////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < BodyForceControl, common::Action, LibUFEM > BodyForceControl_Builder;

////////////////////////////////////////////////////////////////////////////////////////////
/// Implementation of a body force limiter that depends on the velocity of
//reference. Reference: "Modeling a No-Slip Flow Boundary with an External Force
//Field", D. Goldstein, 1993, Journal of Computational Physics, 105, 354-366

BodyForceControl::BodyForceControl(const std::string& name) :
  ProtoAction(name)
{
  options().add("velocity_field_tag", "navier_stokes_u_solution")
    .pretty_name("Velocity Field Tag")
    .description("Tag for the field containing the velocity");

  options().add("surface_regions", m_surface_regions)
      .pretty_name("Regions")
      .description("Regions where uRef should be constant")
      .link_to(&m_surface_regions)
      .mark_basic();

  options().add("uRef", m_uRef)
      .pretty_name("Velocity of Reference")
      .description("Velocity of Reference at the top of the system")
      .link_to(&m_uRef)
      .mark_basic();

  options().add("aCoef", aCoef)
      .pretty_name("alpha coefficient")
      .description("alpha coefficient for feedback loop: Gain")
      .link_to(&aCoef)
      .mark_basic();

  options().add("bCoef", bCoef)
      .pretty_name("beta coefficient")
      .description("beta coefficient for feedback loop: Damping")
      .link_to(&bCoef)
      .mark_basic();

  options().add(solver::Tags::time(), m_time)
    .pretty_name("Time")
    .description("Time component for the simulation")
    .link_to(&m_time);

  FieldVariable<0, VectorField> bf("Force", "body_force");
  
  set_expression(nodes_expression
  (
   /* bf += lit(m_correction) * _norm(bf) /// basic correction */
   bf = lit(m_correction)
  ));
}

BodyForceControl::~BodyForceControl()
{
}

void BodyForceControl::execute()
{
  using boost::proto::lit;
  const std::string tag = options().option("velocity_field_tag").value<std::string>();
  Eigen::Map<RealVector> uRef(&m_uRef[0],m_uRef.size());
  FieldVariable<0, VectorField> u("Velocity", tag);

  /// uMean computation
  RealVector uMean(physical_model().ndim());
  uMean.setZero();
  /* std::cout << "yop: mean:" << uMean << std::endl; */
  surface_integral(uMean, m_surface_regions, u);
  Real surface = 0.0;
  surface_integral(surface, m_surface_regions, lit(1.0));
  uMean /= surface;

  /// uInteg computation
  if(uInteg.size() == 0)                    /// modify uInteg only at 1st iter
  {
    uInteg.resize(physical_model().ndim()); /// Resize to speed dim(x, y, z)
    uInteg.setZero();                       /// Initialize to zero
  } 
  /* std::cout << "yop: uInteg:" << uInteg << ", mean:" << uMean << ", uref:" << uRef << std::endl; */
  uInteg += (uMean - uRef) * m_time->dt();

  /// Correction factor
  m_correction.resize(physical_model().ndim());
  /* m_correction = ((uRef - uMean)/uRef.norm()); /// basic correction */
  m_correction = aCoef * uInteg + bCoef * (uMean - uRef); /// Goldstein p.356
  std::cout << "yop: uInteg:" << uInteg << ", mean:" << uMean << ", corr:" << m_correction << std::endl;
  /* m_correction(1) = 0.; */
  /* m_correction(2) = 0.; */
  /* std::cout << "yop: corr:" << m_correction << std::endl; */

  ProtoAction::execute();
}


} // namespace UFEM

} // namespace cf3
