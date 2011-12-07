// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include "common/Log.hpp"
#include "common/FindComponents.hpp"
#include "common/OptionComponent.hpp"

#include "mesh/Field.hpp"
#include "mesh/SpaceFields.hpp"
#include "mesh/FieldManager.hpp"
#include "mesh/Space.hpp"
#include "mesh/FaceCellConnectivity.hpp"

#include "physics/PhysModel.hpp"
#include "physics/Variables.hpp"

#include "RiemannSolvers/RiemannSolvers/RiemannSolver.hpp"

#include "SFDM/Tags.hpp"
#include "SFDM/Term.hpp"
#include "SFDM/SFDSolver.hpp"
#include "SFDM/ShapeFunction.hpp"

using namespace cf3::common;
using namespace cf3::mesh;
using namespace cf3::solver;
using namespace cf3::solver::actions;

namespace cf3 {
namespace SFDM {

/////////////////////////////////////////////////////////////////////////////////////

Term::Term ( const std::string& name ) :
  cf3::solver::Action(name),
  m_compute_wave_speed(true)
{
  mark_basic();

  options().add_option(OptionComponent<Field>::create( SFDM::Tags::solution(), &m_solution))
      ->pretty_name("Solution Field");

  options().add_option(OptionComponent<Field>::create( SFDM::Tags::wave_speed(), &m_wave_speed))
      ->pretty_name("Wave Speed Field");

  options().add_option(OptionComponent<Field>::create( SFDM::Tags::residual(), &m_residual))
      ->pretty_name("Residual Field");

  options().add_option(OptionComponent<Field>::create( SFDM::Tags::jacob_det(), &m_jacob_det))
      ->pretty_name("Jacobian Determinant Field");

  option(SFDM::Tags::physical_model()).attach_trigger( boost::bind ( &Term::trigger_physical_model, this ) );

}

/////////////////////////////////////////////////////////////////////////////////////

void Term::trigger_physical_model()
{
  // try to configure solution vars
  if (Component::Ptr found_solution_vars = find_component_ptr_with_tag(physical_model(),SFDM::Tags::solution_vars()) )
    m_solution_vars = found_solution_vars->as_ptr<physics::Variables>();
  else
    throw SetupError(FromHere(),"solution_vars not found in physical model");

  m_riemann_solver = solver().as_type<SFDSolver>().riemann_solver().as_ptr<RiemannSolvers::RiemannSolver>();

  allocate_cache();
}

/////////////////////////////////////////////////////////////////////////////////////

void Term::allocate_cache()
{
  cache.phys_props = physical_model().create_properties();
  cache.phys_flux.resize( physical_model().neqs(), physical_model().ndim() );
  cache.dummy_grads.resize( physical_model().neqs(), physical_model().ndim() );
  cache.phys_ev.resize(physical_model().neqs());
  cache.phys_coords.resize( physical_model().ndim() );
  cache.plane_jacobian_normal.resize(physical_model().ndim());
  cache.unit_normal.resize(physical_model().ndim());
}

/////////////////////////////////////////////////////////////////////////////////////

Term::~Term() {}

/////////////////////////////////////////////////////////////////////////////////////

void Term::link_fields()
{
  if( is_null( m_solution.lock() ) )
  {
    m_solution = solver().field_manager()
                         .get_child( SFDM::Tags::solution() ).follow()->as_ptr_checked<Field>();
    configure_option_recursively( SFDM::Tags::solution(), m_solution.lock()->uri() );
  }

  if( is_null( m_residual.lock() ) )
  {
    m_residual = solver().field_manager()
                         .get_child( SFDM::Tags::residual() ).follow()->as_ptr_checked<Field>();
    configure_option_recursively( SFDM::Tags::residual(), m_residual.lock()->uri() );
  }

  if( is_null( m_wave_speed.lock() ) )
  {
    m_wave_speed = solver().field_manager()
                           .get_child( SFDM::Tags::wave_speed() ).follow()->as_ptr_checked<Field>();
    configure_option_recursively( SFDM::Tags::wave_speed(), m_wave_speed.lock()->uri() );
  }

  if( is_null( m_jacob_det.lock() ) )
  {
    m_jacob_det = solver().field_manager()
                          .get_child( SFDM::Tags::jacob_det() ).follow()->as_ptr_checked<Field>();
    configure_option_recursively( SFDM::Tags::jacob_det(), m_jacob_det.lock()->uri() );
  }

}

/////////////////////////////////////////////////////////////////////////////////////


Flyweight::Flyweight(const mesh::Entities& entities_comp, const Uint element_idx, Term& this_term) :
  term(this_term),
  entities(entities_comp),
  geometry(entities.element_type()),
  space(term.solution_field().space(entities)),
  sf(space.shape_function().as_type<SFDM::ShapeFunction>())
{
  entities.allocate_coordinates(term.cache.geometry_nodes);
  set_element(element_idx);
}

/////////////////////////////////////////////////////////////////////////////

Flyweight::Flyweight(const Flyweight& flyweight) :
  term(flyweight.term),
  entities(flyweight.entities),
  geometry(entities.element_type()),
  space(term.solution_field().space(entities)),
  sf(space.shape_function().as_type<SFDM::ShapeFunction>())
{
  throw common::ShouldNotBeHere(FromHere(),"Flyweight is not supposed to be copied");
  entities.allocate_coordinates(term.cache.geometry_nodes);
  set_element(flyweight.element->idx);
}

/////////////////////////////////////////////////////////////////////////////

Flyweight& Flyweight::operator= (const Flyweight& flyweight)
{
  if (this != &flyweight)
    throw common::ShouldNotBeHere(FromHere(),"Flyweight is not supposed to be copied");
  return *this;
}

/////////////////////////////////////////////////////////////////////////////

// Set extrinsic state
Flyweight::Element& Flyweight::set_element(const Uint element_idx)
{
  entities.put_coordinates(term.cache.geometry_nodes,element_idx);
  element = boost::shared_ptr<Element>(new Element(*this,element_idx));
  return *element;
}

/////////////////////////////////////////////////////////////////////////////

void Flyweight::reconstruct_solution_in_flx_pt(const Uint flx_pt, RealVector& sol_in_flx_pt)
{
  sol_in_flx_pt.setZero();
  boost_foreach(const Uint sol_pt, sf.interpolate_sol_to_flx_used_sol_pts(flx_pt))
  {
    term.cache.coeff = sf.interpolate_sol_to_flx_coeff(flx_pt,sol_pt);
    for (Uint var=0; var<sol_in_flx_pt.size(); ++var)
      sol_in_flx_pt[var] += term.cache.coeff * element->solution[sol_pt][var];
  }
}

/////////////////////////////////////////////////////////////////////////////

void Flyweight::add_flx_pt_gradient_contribution_to_residual(const Uint flx_pt, const RealVector& flx_in_flx_pt)
{
  cf3_assert(sf.flx_pt_dirs(flx_pt).size()==1);
  Uint var;
  boost_foreach(const Uint direction, sf.flx_pt_dirs(flx_pt))
  boost_foreach(const Uint sol_pt, sf.interpolate_grad_flx_to_sol_used_sol_pts(flx_pt,direction))
  {
    term.cache.coeff = sf.interpolate_grad_flx_to_sol_coeff(flx_pt,direction,sol_pt);
    for (var=0; var<flx_in_flx_pt.size(); ++var)
      element->residual[sol_pt][var] -= term.cache.coeff * flx_in_flx_pt[var] / element->jacob_det[sol_pt][0];
  }
}

/////////////////////////////////////////////////////////////////////////////

void Flyweight::add_flx_pt_contribution_to_wave_speed(const Uint flx_pt, const Real& ws_in_flx_pt)
{
  if (term.m_compute_wave_speed)
  {
    cf3_assert(sf.flx_pt_dirs(flx_pt).size()==1);
    boost_foreach(const Uint direction, sf.flx_pt_dirs(flx_pt))
        boost_foreach(const Uint sol_pt, sf.interpolate_flx_to_sol_used_sol_pts(flx_pt,direction))
    {
      term.cache.coeff = sf.interpolate_flx_to_sol_coeff(flx_pt,direction,sol_pt);
      element->wave_speed[sol_pt][0] += term.cache.coeff * ws_in_flx_pt / element->jacob_det[sol_pt][0];
    }
  }
}

/////////////////////////////////////////////////////////////////////////////

void Flyweight::compute_analytical_flux(const Uint flx_pt, const RealVector& sol_in_flx_pt, RealVector& flx_in_flx_pt, Real& ws_in_flx_pt)
{
  cf3_assert(sf.flx_pt_dirs(flx_pt).size()==1);

  geometry.compute_plane_jacobian_normal(sf.flx_pts().row(flx_pt),term.cache.geometry_nodes,(CoordRef)sf.flx_pt_dirs(flx_pt)[0],term.cache.plane_jacobian_normal);

  // compute physical properties in flux point
  term.solution_vars().compute_properties(term.cache.geometry_nodes.row(0),sol_in_flx_pt, term.cache.dummy_grads, *term.cache.phys_props);
  // compute flux in flux point
  term.solution_vars().flux(*term.cache.phys_props,term.cache.plane_jacobian_normal,flx_in_flx_pt);

//  // transform physical flux to local coordinate system
//  flx_in_flx_pt = term.cache.phys_flux*term.cache.plane_jacobian_normal;

  if (term.m_compute_wave_speed)
  {
    term.cache.plane_jacobian_det = term.cache.plane_jacobian_normal.norm();
    term.cache.unit_normal = term.cache.plane_jacobian_normal/term.cache.plane_jacobian_det;
    term.m_solution_vars.lock()->flux_jacobian_eigen_values(*term.cache.phys_props,term.cache.unit_normal, term.cache.phys_ev);
    ws_in_flx_pt = term.cache.plane_jacobian_det * term.cache.phys_ev.cwiseAbs().maxCoeff() / 2.;
  }
}

/////////////////////////////////////////////////////////////////////////////

void Flyweight::compute_numerical_flux(const Uint flx_pt, const RealVector& sol_left, const RealVector& sol_right, RealVector& flx_in_flx_pt, Real& ws_in_flx_pt)
{
  cf3_assert(sf.flx_pt_dirs(flx_pt).size()==1);
  const Uint dir = sf.flx_pt_dirs(flx_pt)[0];
  const Real sign = sf.flx_pt_sign(flx_pt,dir);
  geometry.compute_plane_jacobian_normal(sf.flx_pts().row(flx_pt),term.cache.geometry_nodes,(CoordRef)dir,term.cache.plane_jacobian_normal);
  term.cache.plane_jacobian_det = term.cache.plane_jacobian_normal.norm();
  term.cache.unit_normal = sign*term.cache.plane_jacobian_normal/term.cache.plane_jacobian_det;
  if (term.m_compute_wave_speed)
  {
    term.riemann_solver().compute_interface_flux_and_wavespeeds(sol_left,sol_right,term.cache.unit_normal,flx_in_flx_pt,term.cache.phys_ev);
    ws_in_flx_pt = term.cache.plane_jacobian_det * term.cache.phys_ev.cwiseAbs().maxCoeff()/2.;
  }
  else
    term.riemann_solver().compute_interface_flux(sol_left,sol_right,term.cache.unit_normal,flx_in_flx_pt);

  flx_in_flx_pt *= sign*term.cache.plane_jacobian_det;
}

/////////////////////////////////////////////////////////////////////////////

// extrinsic state
Flyweight::Element::Element(const Flyweight& flyweight, const Uint element_idx) :
  idx(element_idx),
  field_idx(flyweight.space.indexes_for_element(idx)),
  solution(flyweight.term.solution_field().view(field_idx)),
  residual(flyweight.term.residual_field().view(field_idx)),
  wave_speed(flyweight.term.wave_speed_field().view(field_idx)),
  jacob_det(flyweight.term.jacob_det_field().view(field_idx))
{}

/////////////////////////////////////////////////////////////////////////////

Flyweight Term::create_flyweight(const mesh::Entities& entities, const Uint element_idx)
{
  return Flyweight(entities, element_idx, *this);
}

Flyweight Term::create_flyweight(const mesh::Entity& entity)
{
  return Flyweight(*entity.comp, entity.idx, *this);
}

std::vector< boost::shared_ptr<Flyweight> > Term::create_flyweight(const mesh::Face2Cell& face)
{
  std::vector< boost::shared_ptr<Flyweight> > flyweights(2);
  for (Uint side=0; side<2u; ++side)
    flyweights[side] = boost::shared_ptr<Flyweight>(new Flyweight(*face.cells()[side].comp,face.cells()[side].idx,*this));
  return flyweights;
}

/////////////////////////////////////////////////////////////////////////////

} // SFDM
} // cf3

/////////////////////////////////////////////////////////////////////////////////////
