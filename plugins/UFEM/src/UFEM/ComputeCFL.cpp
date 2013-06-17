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
  m_dt(0.)
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
  
  template<typename UT, typename NodesT, unsigned long N>
  Real max_edge_projection(const UT& u, const NodesT& nodes, const boost::array<int, N>& a_nodes, const boost::array<int, N>& b_nodes)
  {
    Real result = 0.;
    for(int i = 0; i != N; ++i)
    {
      const UT diff = nodes.row(a_nodes[i]) - nodes.row(b_nodes[i]);
      const Real projection = fabs(u.dot(diff) / diff.squaredNorm());
        result = projection > result ? projection : result;
    }
    
    return result;
  }

  template<typename UT>
  void get_scale(boost::mpl::int_<cf3::mesh::GeoShape::TRIAG>, const UT& u, Real& cfl_scale)
  {
    static const RealVector2 center(1./3., 1./3.);
    const boost::array<int, 3> a_nodes = { 0, 1, 2 };
    const boost::array<int, 3> b_nodes = { 1, 2, 0 };
    cfl_scale = max_edge_projection(u.eval(center), u.support().nodes(), a_nodes, b_nodes);
  }
  
  template<typename UT>
  void get_scale(boost::mpl::int_<cf3::mesh::GeoShape::TETRA>, const UT& u, Real& cfl_scale)
  {
    static const RealVector3 center(1./3., 1./3., 1./3.);
    const boost::array<int, 6> a_nodes = { 0, 1, 2, 0, 1, 2 };
    const boost::array<int, 6> b_nodes = { 1, 2, 0, 3, 3, 3 };
    cfl_scale = max_edge_projection(u.eval(center), u.support().nodes(), a_nodes, b_nodes);
  }

  template<typename UT>
  void get_scale(boost::mpl::int_<cf3::mesh::GeoShape::PRISM>, const UT& u, Real& cfl_scale)
  {
    static const RealVector3 center(1./3., 1./3., 0.);
    const boost::array<int, 9> a_nodes = { 0, 1, 2, 3, 4, 5, 0, 1, 2 };
    const boost::array<int, 9> b_nodes = { 1, 2, 0, 4, 5, 3, 3, 4, 5 };
    cfl_scale = max_edge_projection(u.eval(center), u.support().nodes(), a_nodes, b_nodes);
  }

  template<typename UT>
  void get_scale(boost::mpl::int_<cf3::mesh::GeoShape::QUAD>, const UT& u, Real& cfl_scale)
  {
    static const RealVector2 center(0., 0.);
    const boost::array<int, 4> a_nodes = { 0, 1, 2, 3 };
    const boost::array<int, 4> b_nodes = { 1, 2, 3, 0 };
    cfl_scale = max_edge_projection(u.eval(center), u.support().nodes(), a_nodes, b_nodes);
  }
  
  template<typename UT>
  void get_scale(boost::mpl::int_<cf3::mesh::GeoShape::HEXA>, const UT& u, Real& cfl_scale)
  {
    static const RealVector3 center(0., 0., 0.);
    const boost::array<int, 12> a_nodes = { 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3 };
    const boost::array<int, 12> b_nodes = { 1, 2, 3, 0, 5, 6, 7, 4, 4, 5, 6, 7 };
    cfl_scale = max_edge_projection(u.eval(center), u.support().nodes(), a_nodes, b_nodes);
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

  set_expression(elements_expression
  (
    boost::mpl::vector10<mesh::LagrangeP0::Triag, mesh::LagrangeP1::Triag2D, mesh::LagrangeP0::Quad, mesh::LagrangeP1::Quad2D, mesh::LagrangeP0::Hexa, mesh::LagrangeP1::Hexa3D, mesh::LagrangeP0::Tetra, mesh::LagrangeP1::Tetra3D, mesh::LagrangeP0::Prism, mesh::LagrangeP1::Prism3D>(),
    group
    (
      compute_cfl(u, lit(m_cfl_scaling)),
      cfl = lit(m_dt) * lit(m_cfl_scaling),
      lit(m_max_computed_cfl) = _max(lit(m_max_computed_cfl), cfl)
    )
  ));
}

void ComputeCFL::execute()
{
  if(is_null(m_time))
    throw common::SetupError(FromHere(), "No time component configured for ComputeCFL");
  
  m_dt = m_time->dt();

  m_max_computed_cfl = 0.;
  ProtoAction::execute();

  Real global_max_cfl = m_max_computed_cfl;
  if(common::PE::Comm::instance().is_active())
  {
    common::PE::Comm::instance().all_reduce(common::PE::max(), &m_max_computed_cfl, 1, &global_max_cfl);
  }

  CFinfo << "CFL for time step " << m_dt << " is " << global_max_cfl << CFendl;
}



} // namespace UFEM

} // namespace cf3
