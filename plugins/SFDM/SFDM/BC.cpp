// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the BCs of the
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
#include "SFDM/BC.hpp"
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

BC::BC ( const std::string& name ) :
  cf3::solver::Action(name)
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
      .pretty_name("Jacobian DeBCinant Field")
      .link_to(&m_jacob_det);

  options().add_option(SFDM::Tags::shared_caches(), m_shared_caches)
      .pretty_name("Share Caches")
      .link_to(&m_shared_caches);
}

/////////////////////////////////////////////////////////////////////////////////////

void BC::link_fields()
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

void BC::find_inner_cell(const Handle<Entities const>& face_entities, const Uint face_idx, Handle<Entities const>& cell_entities, Uint& cell_idx, Uint& cell_face_nb)
{
  FaceCellConnectivity const& cell_connectivity = *face_entities->get_child("cell_connectivity")->handle<FaceCellConnectivity>();
  cell_entities = cell_connectivity.connectivity()[face_idx][LEFT].comp->handle<Entities>();
  cell_idx = cell_connectivity.connectivity()[face_idx][LEFT].idx;
  cell_face_nb = cell_connectivity.face_number()[face_idx][LEFT];
}

/////////////////////////////////////////////////////////////////////////////////////

BC::~BC() {}

/////////////////////////////////////////////////////////////////////////////

} // SFDM
} // cf3

/////////////////////////////////////////////////////////////////////////////////////
