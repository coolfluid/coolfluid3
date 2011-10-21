// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/OptionComponent.hpp"
#include "common/OptionT.hpp"


#include "mesh/CInterpolator.hpp"
#include "mesh/CMesh.hpp"
#include "mesh/Field.hpp"
#include "mesh/CStencilComputer.hpp"

namespace cf3 {
namespace mesh {

using namespace common;
using namespace common::XML;

////////////////////////////////////////////////////////////////////////////////

CInterpolator::CInterpolator ( const std::string& name  ) :
  Component ( name )
{
  m_options.add_option(OptionComponent<Field>::create("source", &m_source))
      ->description("Field to interpolate from")
      ->pretty_name("Source Field")
      ->mark_basic();

  m_options.add_option(OptionComponent<Field>::create("target", &m_target))
      ->description("Field to interpolate to")
      ->pretty_name("TargetField")
      ->mark_basic();

  m_options.add_option(OptionT<bool>::create("store", true))
      ->description("Flag to store weights and stencils used for faster interpolation")
      ->pretty_name("Store");

  m_options.add_option(OptionT<std::string>::create("stencil_computer", std::string("stencilcomputer")))
      ->description("Builder name of the stencil computer")
      ->pretty_name("Stencil Computer")
      ->attach_trigger( boost::bind( &CInterpolator::configure_stencil_computer, this ) )
      ->mark_basic();

  m_options.add_option(OptionT<std::string>::create("function", std::string("function")))
      ->description("Builder name of the interpolator function")
      ->pretty_name("Interpolator Function")
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


} // mesh
} // cf3
