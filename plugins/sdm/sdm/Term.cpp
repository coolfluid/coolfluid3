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
#include "mesh/Dictionary.hpp"
#include "mesh/FieldManager.hpp"
#include "mesh/Space.hpp"
#include "mesh/FaceCellConnectivity.hpp"

#include "physics/PhysModel.hpp"
#include "physics/Variables.hpp"

#include "sdm/Tags.hpp"
#include "sdm/Term.hpp"
#include "sdm/SDSolver.hpp"
#include "sdm/ShapeFunction.hpp"
#include "sdm/ElementCaching.hpp"
#include "mesh/ElementConnectivity.hpp"

using namespace cf3::common;
using namespace cf3::mesh;
using namespace cf3::solver;
using namespace cf3::solver::actions;

namespace cf3 {
namespace sdm {

/////////////////////////////////////////////////////////////////////////////////////

Term::Term ( const std::string& name ) :
  cf3::solver::Action(name),
  m_compute_wave_speed(true)
{
  mark_basic();

  options().add(sdm::Tags::solution(), m_solution)
      .pretty_name("Solution Field")
      .link_to(&m_solution);

  options().add(sdm::Tags::wave_speed(), m_wave_speed)
      .pretty_name("Wave Speed Field")
      .link_to(&m_wave_speed);

  options().add(sdm::Tags::residual(), m_residual)
      .pretty_name("Residual Field")
      .link_to(&m_residual);

  options().add(sdm::Tags::jacob_det(), m_jacob_det)
      .pretty_name("Jacobian Determinant Field")
      .link_to(&m_jacob_det);

  options().add(sdm::Tags::shared_caches(), m_shared_caches)
      .pretty_name("Share Caches")
      .link_to(&m_shared_caches);

  options().option(sdm::Tags::physical_model()).attach_trigger( boost::bind ( &Term::trigger_physical_model, this ) );

}

/////////////////////////////////////////////////////////////////////////////////////

void Term::trigger_physical_model()
{
  // try to configure solution vars
  if (Handle<Component> found_solution_vars = find_component_ptr_with_tag(physical_model(),sdm::Tags::solution_vars()) )
    m_solution_vars = found_solution_vars->handle<physics::Variables>();
  else
    throw SetupError(FromHere(),"solution_vars not found in physical model");
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
    m_term_field = solution_field().dict().create_field(name(),vars->description()).handle<Field>();
    m_term_field->parallelize();
  }
  if (is_null(m_term_wave_speed_field))
  {
    link_fields();
    boost::shared_ptr<math::VariablesDescriptor> vars(allocate_component<math::VariablesDescriptor>("tmp"));
    vars->set_variables(name()+"_wavespeed");
    m_term_wave_speed_field = solution_field().dict().create_field(name()+"_wavespeed",vars->description()).handle<Field>();
    m_term_wave_speed_field->parallelize();
  }

}

/////////////////////////////////////////////////////////////////////////////////////

void Term::set_face(const Handle<Entities const>& entities, const Uint elem_idx, const Uint face_nb,
                    Handle<Entities const>& neighbour_entities, Uint& neighbour_elem_idx, Uint& neighbour_face_nb,
                    Handle<Entities const>& face_entities, Uint& face_idx, Uint& face_side)
{
  const Handle<ElementConnectivity>& face_connectivity = entities->connectivity_cell2face();
  cf3_assert(face_connectivity);
  cf3_assert(elem_idx < face_connectivity->size());
  cf3_assert(face_nb < (*face_connectivity)[elem_idx].size());
  Entity face = (*face_connectivity)[elem_idx][face_nb];
  cf3_assert_desc("face "+entities->uri().string()+"["+to_str(elem_idx)+"]["+to_str(face_nb)+"] is null", is_not_null(face.comp) );
  face_entities = face.comp->handle<Entities>();
  face_idx = face.idx;
  const Handle<FaceCellConnectivity>& cell_connectivity = face.comp->connectivity_face2cell();
  cf3_assert(cell_connectivity);
  cf3_assert(face.idx < cell_connectivity->is_bdry_face().size())
  if (cell_connectivity->is_bdry_face()[face.idx])
  {
    neighbour_entities = Handle<Entities const>();
  }
  else
  {
    cf3_assert(face.idx < cell_connectivity->connectivity().size());
    cf3_assert(is_not_null(cell_connectivity->connectivity()[face.idx][LEFT].comp))
    if (cell_connectivity->connectivity()[face.idx][LEFT].comp == entities.get() &&
        cell_connectivity->connectivity()[face.idx][LEFT].idx == elem_idx)
    {
      face_side = LEFT;
      cf3_assert(is_not_null(cell_connectivity->connectivity()[face.idx][RIGHT].comp))
      neighbour_entities = cell_connectivity->connectivity()[face.idx][RIGHT].comp->handle<Entities>();
      neighbour_elem_idx = cell_connectivity->connectivity()[face.idx][RIGHT].idx;
      neighbour_face_nb = cell_connectivity->face_number()[face.idx][RIGHT];
    }
    else
    {
      face_side = RIGHT;
      neighbour_entities = cell_connectivity->connectivity()[face.idx][LEFT].comp->handle<Entities>();
      neighbour_elem_idx = cell_connectivity->connectivity()[face.idx][LEFT].idx;
      neighbour_face_nb = cell_connectivity->face_number()[face.idx][LEFT];
    }
  }
}

/////////////////////////////////////////////////////////////////////////////////////

void Term::link_fields()
{
  if( is_null( m_solution ) )
  {
    m_solution = Handle<Field>( follow_link( solver().field_manager().get_child( sdm::Tags::solution() ) ) );
    configure_option_recursively( sdm::Tags::solution(), m_solution );
  }

  if( is_null( m_residual ) )
  {
    m_residual = Handle<Field>( follow_link( solver().field_manager().get_child( sdm::Tags::residual() ) ) );
    configure_option_recursively( sdm::Tags::residual(), m_residual );
  }

  if( is_null( m_wave_speed ) )
  {
    m_wave_speed = Handle<Field>( follow_link( solver().field_manager().get_child( sdm::Tags::wave_speed() ) ) );
    configure_option_recursively( sdm::Tags::wave_speed(), m_wave_speed );
  }

  if( is_null( m_jacob_det ) )
  {
    m_jacob_det = Handle<Field>( follow_link( solver().field_manager().get_child( sdm::Tags::jacob_det() ) ) );
    configure_option_recursively( sdm::Tags::jacob_det(), m_jacob_det );
  }

  if( is_null( m_delta ) )
  {
    m_delta = Handle<Field>( follow_link( solver().field_manager().get_child( sdm::Tags::delta() ) ) );
    configure_option_recursively( sdm::Tags::delta(), m_delta );
  }

  if( is_null( m_shared_caches ) )
  {
    m_shared_caches = Handle<SharedCaches>( solver().handle<SDSolver>()->shared_caches().handle<SharedCaches>() );
  }
}

/////////////////////////////////////////////////////////////////////////////

} // sdm
} // cf3

/////////////////////////////////////////////////////////////////////////////////////
