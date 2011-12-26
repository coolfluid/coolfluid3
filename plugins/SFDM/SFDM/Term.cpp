// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include "common/Link.hpp"
#include "common/Log.hpp"
#include "common/FindComponents.hpp"
#include "common/OptionList.hpp"
#include "common/List.hpp"

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
#include "SFDM/ElementCaching.hpp"

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

  options().add_option(SFDM::Tags::solution(), m_solution)
      .pretty_name("Solution Field")
      .link_to(&m_solution);

  options().add_option(SFDM::Tags::wave_speed(), m_wave_speed)
      .pretty_name("Wave Speed Field")
      .link_to(&m_wave_speed);

  options().add_option(SFDM::Tags::residual(), m_residual)
      .pretty_name("Residual Field")
      .link_to(&m_residual);

  options().add_option(SFDM::Tags::jacob_det(), m_jacob_det)
      .pretty_name("Jacobian Determinant Field")
      .link_to(&m_jacob_det);

  options().add_option(SFDM::Tags::shared_caches(), m_shared_caches)
      .pretty_name("Share Caches")
      .link_to(&m_shared_caches);

  options().option(SFDM::Tags::physical_model()).attach_trigger( boost::bind ( &Term::trigger_physical_model, this ) );

}

/////////////////////////////////////////////////////////////////////////////////////

void Term::trigger_physical_model()
{
  // try to configure solution vars
  if (Handle<Component> found_solution_vars = find_component_ptr_with_tag(physical_model(),SFDM::Tags::solution_vars()) )
    m_solution_vars = found_solution_vars->handle<physics::Variables>();
  else
    throw SetupError(FromHere(),"solution_vars not found in physical model");

  m_riemann_solver = solver().handle<SFDSolver>()->riemann_solver().handle<RiemannSolvers::RiemannSolver>();

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

void Term::create_term_field()
{
  if (is_null(m_term_field))
  {
    link_fields();
    boost::shared_ptr<math::VariablesDescriptor> vars(allocate_component<math::VariablesDescriptor>("tmp"));
    vars->set_variables(solution_field().descriptor().description());
    vars->prefix_variable_names(name()+"_");
    m_term_field = solution_field().field_group().create_field(name(),vars->description()).handle<Field>();
  }
  if (is_null(m_term_wave_speed_field))
  {
    link_fields();
    boost::shared_ptr<math::VariablesDescriptor> vars(allocate_component<math::VariablesDescriptor>("tmp"));
    vars->set_variables(name()+"_wavespeed[vector]");
    m_term_wave_speed_field = solution_field().field_group().create_field(name()+"_wavespeed",vars->description()).handle<Field>();
  }

}

/////////////////////////////////////////////////////////////////////////////////////

void Term::set_neighbour(const Handle<Entities const>& entities, const Uint elem_idx, const Uint face_nb,
                   Handle<Entities const>& neighbour_entities, Uint& neighbour_elem_idx, Uint& neighbour_face_nb,
                   Handle<Entities const>& face_entities, Uint& face_idx)
{
  ElementConnectivity const& face_connectivity = *entities->get_child("face_connectivity")->handle<ElementConnectivity>();
  Entity face = face_connectivity[elem_idx][face_nb];
  face_entities = face.comp->handle<Entities>();
  face_idx = face.idx;
  FaceCellConnectivity const& cell_connectivity = *face.comp->get_child("cell_connectivity")->handle<FaceCellConnectivity>();
  if (cell_connectivity.is_bdry_face()[face.idx])
  {
    neighbour_entities = Handle<Entities const>();
  }
  else
  {
    if (cell_connectivity.connectivity()[face.idx][LEFT].comp == entities.get() &&
        cell_connectivity.connectivity()[face.idx][LEFT].idx == elem_idx)
    {
      neighbour_entities = cell_connectivity.connectivity()[face.idx][RIGHT].comp->handle<Entities>();
      neighbour_elem_idx = cell_connectivity.connectivity()[face.idx][RIGHT].idx;
      neighbour_face_nb = cell_connectivity.face_number()[face.idx][RIGHT];
    }
    else
    {
      neighbour_entities = cell_connectivity.connectivity()[face.idx][LEFT].comp->handle<Entities>();
      neighbour_elem_idx = cell_connectivity.connectivity()[face.idx][LEFT].idx;
      neighbour_face_nb = cell_connectivity.face_number()[face.idx][LEFT];
    }
  }
}

/////////////////////////////////////////////////////////////////////////////////////

void Term::link_fields()
{
  if( is_null( m_solution ) )
  {
    m_solution = Handle<Field>( follow_link( solver().field_manager().get_child( SFDM::Tags::solution() ) ) );
    configure_option_recursively( SFDM::Tags::solution(), m_solution );
  }

  if( is_null( m_residual ) )
  {
    m_residual = Handle<Field>( follow_link( solver().field_manager().get_child( SFDM::Tags::residual() ) ) );
    configure_option_recursively( SFDM::Tags::residual(), m_residual );
  }

  if( is_null( m_wave_speed ) )
  {
    m_wave_speed = Handle<Field>( follow_link( solver().field_manager().get_child( SFDM::Tags::wave_speed() ) ) );
    configure_option_recursively( SFDM::Tags::wave_speed(), m_wave_speed );
  }

  if( is_null( m_jacob_det ) )
  {
    m_jacob_det = Handle<Field>( follow_link( solver().field_manager().get_child( SFDM::Tags::jacob_det() ) ) );
    configure_option_recursively( SFDM::Tags::jacob_det(), m_jacob_det );
  }

  if( is_null( m_delta ) )
  {
    m_delta = Handle<Field>( follow_link( solver().field_manager().get_child( SFDM::Tags::delta() ) ) );
    configure_option_recursively( SFDM::Tags::delta(), m_delta );
  }

  if( is_null( m_shared_caches ) )
  {
    m_shared_caches = Handle<SharedCaches>( solver().handle<SFDSolver>()->shared_caches().handle<SharedCaches>() );
  }
}

/////////////////////////////////////////////////////////////////////////////////////


Flyweight::Flyweight(const mesh::Entities& entities_comp, const Uint element_idx, Term& this_term) :
  term(this_term),
  entities(entities_comp),
  geometry(entities.element_type()),
  space(term.solution_field().space(entities)),
  sf(*space.shape_function().handle<SFDM::ShapeFunction>())
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
  sf(*space.shape_function().handle<SFDM::ShapeFunction>())
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

void Flyweight::add_flx_pt_gradient_contribution_to_residual(const Uint flx_pt, const RealVector& flx_in_flx_pt,bool outward)
{
  cf3_assert(sf.flx_pt_dirs(flx_pt).size()==1);
  Uint var;
  boost_foreach(const Uint direction, sf.flx_pt_dirs(flx_pt))
  {
    const Uint dir = sf.flx_pt_dirs(flx_pt)[0];
    const Real sign = sf.flx_pt_sign(flx_pt,dir) * (outward ? 1. : -1. );
    // sign is positive if unit-normal is positive in coord direction
    boost_foreach(const Uint sol_pt, sf.interpolate_grad_flx_to_sol_used_sol_pts(flx_pt,direction))
    {
      term.cache.coeff = sf.interpolate_grad_flx_to_sol_coeff(flx_pt,direction,sol_pt);
      for (var=0; var<flx_in_flx_pt.size(); ++var)
        element->residual[sol_pt][var] -= term.cache.coeff * sign * flx_in_flx_pt[var] / element->jacob_det[sol_pt][0];
    }
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
  RealVector coord = geometry.shape_function().value(sf.flx_pts().row(flx_pt)) * term.cache.geometry_nodes;

  // compute physical properties in flux point
  term.solution_vars().compute_properties(coord,sol_in_flx_pt, term.cache.dummy_grads, *term.cache.phys_props);
  // compute flux in flux point
  term.solution_vars().flux(*term.cache.phys_props,term.cache.plane_jacobian_normal,flx_in_flx_pt);

  /// this is required if flx_pt is also a face_pt, because add_flx_pt_gradient_contribution_to_residual takes this into account
  flx_in_flx_pt *= sf.flx_pt_sign(flx_pt,sf.flx_pt_dirs(flx_pt)[0]);

  if (term.m_compute_wave_speed)
  {
    term.cache.plane_jacobian_det = term.cache.plane_jacobian_normal.norm();
    term.cache.unit_normal = term.cache.plane_jacobian_normal/term.cache.plane_jacobian_det;
    term.m_solution_vars->flux_jacobian_eigen_values(*term.cache.phys_props,term.cache.unit_normal, term.cache.phys_ev);
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

  RealVector coord = geometry.shape_function().value(sf.flx_pts().row(flx_pt)) * term.cache.geometry_nodes;
  term.cache.plane_jacobian_det = term.cache.plane_jacobian_normal.norm();
  term.cache.unit_normal = sign*term.cache.plane_jacobian_normal/term.cache.plane_jacobian_det;
  if (term.m_compute_wave_speed)
  {
    term.riemann_solver().compute_interface_flux_and_wavespeeds(sol_left,sol_right,coord,term.cache.unit_normal,flx_in_flx_pt,term.cache.phys_ev);
    ws_in_flx_pt = term.cache.plane_jacobian_det * term.cache.phys_ev.cwiseAbs().maxCoeff()/2.;
  }
  else
    term.riemann_solver().compute_interface_flux(sol_left,sol_right,coord,term.cache.unit_normal,flx_in_flx_pt);

  flx_in_flx_pt *= term.cache.plane_jacobian_det;
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
