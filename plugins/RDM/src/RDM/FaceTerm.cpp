// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Signal.hpp"
#include "Common/OptionComponent.hpp"

#include "Mesh/CField.hpp"

#include "Physics/PhysModel.hpp"
#include "Physics/Variables.hpp"

#include "RDM/RDSolver.hpp"
#include "RDM/FaceLoop.hpp"
#include "RDM/FaceTerm.hpp"

using namespace CF::Common;
using namespace CF::Mesh;

namespace CF {
namespace RDM {

/////////////////////////////////////////////////////////////////////////////////////

FaceTerm::FaceTerm ( const std::string& name ) :
  CF::Solver::Action(name)
{
  mark_basic();

  m_options.add_option(OptionComponent<CField>::create( RDM::Tags::solution(), &m_solution))
      ->pretty_name("Solution Field");

  m_options.add_option(OptionComponent<CField>::create( RDM::Tags::wave_speed(), &m_wave_speed))
      ->pretty_name("Wave Speed Field");

  m_options.add_option(OptionComponent<CField>::create( RDM::Tags::residual(), &m_residual))
      ->pretty_name("Residual Field");
}

FaceTerm::~FaceTerm() {}

void FaceTerm::link_fields()
{
  if( is_null( m_solution.lock() ) )
  {
    m_solution = solver().as_type<RDM::RDSolver>().fields()
                         .get_child( RDM::Tags::solution() ).follow()->as_ptr_checked<CField>();
    configure_option_recursively( RDM::Tags::solution(), m_solution.lock()->uri() );
  }

  if( is_null( m_residual.lock() ) )
  {
    m_residual = solver().as_type<RDM::RDSolver>().fields()
                         .get_child( RDM::Tags::residual() ).follow()->as_ptr_checked<CField>();
    configure_option_recursively( RDM::Tags::residual(), m_residual.lock()->uri() );
  }

  if( is_null( m_wave_speed.lock() ) )
  {
    m_wave_speed = solver().as_type<RDM::RDSolver>().fields()
                         .get_child( RDM::Tags::wave_speed() ).follow()->as_ptr_checked<CField>();
    configure_option_recursively( RDM::Tags::wave_speed(), m_wave_speed.lock()->uri() );
  }
}


ElementLoop& FaceTerm::access_element_loop( const std::string& type_name )
{
  // ensure that the fields are present

  link_fields();

  // get the element loop or create it if does not exist

  ElementLoop::Ptr loop;
  Common::Component::Ptr cloop = get_child_ptr( "LOOP" );
  if( is_null( cloop ) )
  {
    const std::string update_vars_type =
        physical_model().get_child( RDM::Tags::update_vars() )
                        .as_type<Physics::Variables>()
                        .type();

    loop = build_component_abstract_type_reduced< FaceLoop >( "FaceLoopT<" + type_name + "," + update_vars_type + ">" , "LOOP");
    add_component(loop);
  }
  else
    loop = cloop->as_ptr_checked<ElementLoop>();

  return *loop;
}


/////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

/////////////////////////////////////////////////////////////////////////////////////
