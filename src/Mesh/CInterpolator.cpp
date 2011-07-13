// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/OptionComponent.hpp"
#include "Common/OptionT.hpp"


#include "Mesh/CInterpolator.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CStencilComputer.hpp"

namespace CF {
namespace Mesh {

using namespace Common;
using namespace Common::XML;

////////////////////////////////////////////////////////////////////////////////

CInterpolator::CInterpolator ( const std::string& name  ) :
  Component ( name )
{
  m_options.add_option(OptionComponent<CField>::create("source", &m_source))
      ->set_description("Field to interpolate from")
      ->set_pretty_name("Source Field")
      ->mark_basic();

  m_options.add_option(OptionComponent<CField>::create("target", &m_target))
      ->set_description("Field to interpolate to")
      ->set_pretty_name("TargetField")
      ->mark_basic();

  m_options.add_option(OptionT<bool>::create("store", true))
      ->set_description("Flag to store weights and stencils used for faster interpolation")
      ->set_pretty_name("Store");

  m_options.add_option(OptionT<std::string>::create("stencil_computer", std::string("stencilcomputer")))
      ->set_description("Builder name of the stencil computer")
      ->set_pretty_name("Stencil Computer")
      ->attach_trigger( boost::bind( &CInterpolator::configure_stencil_computer, this ) )
      ->mark_basic();

  m_options.add_option(OptionT<std::string>::create("function", std::string("function")))
      ->set_description("Builder name of the interpolator function")
      ->set_pretty_name("Interpolator Function")
      ->attach_trigger( boost::bind( &CInterpolator::configure_interpolator_function, this ) )
      ->mark_basic();
}

////////////////////////////////////////////////////////////////////////////////

CInterpolator::~CInterpolator()
{
}

////////////////////////////////////////////////////////////////////////////////

void CInterpolator::configure_stencil_computer()
{
  if (is_not_null(m_stencil_computer))
    remove_component(m_stencil_computer->name());
  m_stencil_computer = build_component_abstract_type<CStencilComputer>(option("stencil_computer").value<std::string>(),"stencil_computer");
}

////////////////////////////////////////////////////////////////////////////////

void CInterpolator::configure_interpolator_function()
{

}

//////////////////////////////////////////////////////////////////////////////

void CInterpolator::signal_interpolate( SignalArgs& node  )
{
  interpolate();
}

////////////////////////////////////////////////////////////////////////////////

void CInterpolator::interpolate()
{
  if ( m_source.expired() )
    throw SetupError (FromHere(), "SourceField option was not set");
  if ( m_target.expired() )
    throw SetupError (FromHere(), "TargetField option was not set");
  construct_internal_storage(*m_source.lock()->parent().as_ptr<CMesh>());
  interpolate_field_from_to(*m_source.lock(),*m_target.lock());
}

////////////////////////////////////////////////////////////////////////////////


} // Mesh
} // CF
