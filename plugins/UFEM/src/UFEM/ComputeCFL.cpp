// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

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
#include <mesh/LagrangeP0/Hexa.hpp>
#include <mesh/LagrangeP0/Tetra.hpp>

#include "ComputeCFL.hpp"
#include "AdjacentCellToFace.hpp"
#include "Tags.hpp"

#include "solver/Time.hpp"

#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"
#include "solver/Tags.hpp"

namespace cf3
{

namespace UFEM
{

using namespace solver::actions::Proto;

common::ComponentBuilder < ComputeCFL, common::Action, LibUFEM > ComputeCFL_Builder;

ComputeCFL::ComputeCFL(const std::string& name) :
  ProtoAction(name),
  m_max_cfl(0.8),
  m_dt(0.1)
{
  options().add("velocity_field_tag", "navier_stokes_u_solution")
    .pretty_name("Velocity Field Tag")
    .description("Tag for the field containing the velocity")
    .attach_trigger(boost::bind(&ComputeCFL::trigger_variable, this));

  options().add("velocity_variable_name", "Velocity")
    .pretty_name("Velocity Variable Name")
    .description("Name of the velocity to use")
    .attach_trigger(boost::bind(&ComputeCFL::trigger_variable, this));

  options().add("max_cfl", m_max_cfl)
    .pretty_name("Max CFL")
    .description("Maximum allowed CFL number")
    .link_to(&m_max_cfl);

  options().add("time_step", m_dt)
    .pretty_name("Time Step")
    .description("Time step to use as reference for CFL calculation")
    .link_to(&m_dt)
    .link_to(&m_min_dt);

  options().add(solver::Tags::time(), m_time)
    .pretty_name("Time")
    .description("Time component for the simulation")
    .link_to(&m_time);

  trigger_variable();
}

ComputeCFL::~ComputeCFL()
{
}

struct CFLOp
{
  typedef void result_type;

  template<typename T>
  struct ShapeType
  {
    typedef boost::mpl::int_<T::shape> type;
  };

  template<Uint Dim>
  struct ShapeType< ElementBased<Dim> >
  {
    typedef boost::mpl::void_ type;
  };

  template<typename UT>
  void operator()(const UT& u, Real& cfl_scale)
  {
    typedef typename UT::EtypeT ElementT;
    get_scale(typename ShapeType<ElementT>::type(), u, cfl_scale);
  }

  template<typename UT>
  void get_scale(boost::mpl::int_<cf3::mesh::GeoShape::TRIAG>, const UT& u, Real& cfl_scale)
  {
    static const RealVector2 center(0.5, 0.5);
    cfl_scale = sqrt(u.eval(center).squaredNorm() / (4./3.141592654*u.support().volume()));
  }
  
  template<typename UT>
  void get_scale(boost::mpl::int_<cf3::mesh::GeoShape::TETRA>, const UT& u, Real& cfl_scale)
  {
    static const RealVector3 center(0.5, 0.5, 0.5);
    cfl_scale = u.eval(center).norm() / ::pow(3./4./3.141592654*u.support().volume(),1./3.);
  }

  template<typename UT>
  void get_scale(boost::mpl::int_<cf3::mesh::GeoShape::QUAD>, const UT& u, Real& cfl_scale)
  {
    static const RealVector2 center(0., 0.);
    const RealVector2 e_xi = u.support().nodes().row(1) - u.support().nodes().row(0);
    const RealVector2 e_eta = u.support().nodes().row(3) - u.support().nodes().row(0);
    const Real u_xi = e_xi.dot(u.eval(center));
    const Real u_eta = e_eta.dot(u.eval()); // center omitted here, so it reuses the previously calculated value
    cfl_scale = sqrt(u_xi*u_xi/e_xi.squaredNorm() + u_eta*u_eta/e_eta.squaredNorm());
  }
  
  template<typename UT>
  void get_scale(boost::mpl::int_<cf3::mesh::GeoShape::HEXA>, const UT& u, Real& cfl_scale)
  {
    static const RealVector3 center(0., 0., 0.);
    const RealVector3 e_xi = u.support().nodes().row(1) - u.support().nodes().row(0);
    const RealVector3 e_eta = u.support().nodes().row(3) - u.support().nodes().row(0);
    const RealVector3 e_zta = u.support().nodes().row(4) - u.support().nodes().row(0);
    const Real u_xi = e_xi.dot(u.eval(center));
    const Real u_eta = e_eta.dot(u.eval()); // center omitted here, so it reuses the previously calculated value
    const Real u_zta = e_zta.dot(u.eval());
    cfl_scale = sqrt(u_xi*u_xi/e_xi.squaredNorm() + u_eta*u_eta/e_eta.squaredNorm() + u_zta*u_zta/e_zta.squaredNorm());
  }

  template<typename UT>
  inline void get_scale(boost::mpl::void_, const UT& u, Real& cfl_scale)
  {
  }
};

static solver::actions::Proto::MakeSFOp<CFLOp>::type const compute_cfl = {};

void ComputeCFL::trigger_variable()
{
  using boost::proto::lit;

  const std::string tag = options().option("velocity_field_tag").value<std::string>();
  const std::string var = options().option("velocity_variable_name").value<std::string>();

  FieldVariable<0, VectorField> u(var, tag);
  FieldVariable<1, ScalarField> cfl("CFL", "cfl", mesh::LagrangeP0::LibLagrangeP0::library_namespace());
  FieldVariable<2, ScalarField> dt_max("DtMax", "cfl", mesh::LagrangeP0::LibLagrangeP0::library_namespace());

  set_expression(elements_expression
  (
    boost::mpl::vector8<mesh::LagrangeP0::Triag, mesh::LagrangeP1::Triag2D, mesh::LagrangeP0::Quad, mesh::LagrangeP1::Quad2D, mesh::LagrangeP0::Hexa, mesh::LagrangeP1::Hexa3D, mesh::LagrangeP0::Tetra, mesh::LagrangeP1::Tetra3D>(),
    group
    (
      compute_cfl(u, lit(m_cfl_scaling)),
      cfl = lit(m_dt) * lit(m_cfl_scaling),
      dt_max = lit(m_max_cfl) / lit(m_cfl_scaling),
      lit(m_min_dt) = _min(lit(m_min_dt), dt_max)
    )
  ));
}

void ComputeCFL::execute()
{
  CFwarn << "executing CFL computation with dt " << m_dt << " and min_dt " << m_min_dt << CFendl;
  ProtoAction::execute();

  Real global_min_dt = m_min_dt;
  if(common::PE::Comm::instance().is_active())
  {
    common::PE::Comm::instance().all_reduce(common::PE::min(), &m_min_dt, 1, &global_min_dt);
  }

  m_min_dt = global_min_dt;

  if(m_min_dt < m_dt)
  {
    CFwarn << "Time step " << m_dt << "s is too big to honour Courant number " << m_max_cfl << ", adjusting to " << m_min_dt << "s" << CFendl;
    m_time->options().set("time_step", m_min_dt);
  }
}



} // namespace UFEM

} // namespace cf3
