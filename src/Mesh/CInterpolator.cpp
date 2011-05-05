// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/OptionComponent.hpp"
#include "Common/OptionT.hpp"
#include "Common/CreateComponent.hpp"

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
  properties().add_option(OptionComponent<CField>::create("source","Source Field","Field to interpolate from",&m_source))
    ->mark_basic();
    
  properties().add_option(OptionComponent<CField>::create("target","TargetField","Field to interpolate to",&m_target))
    ->mark_basic();

  properties().add_option(OptionT<bool>::create("store","Store","Flag to store weights and stencils used for faster interpolation",true));
  
  properties().add_option(OptionT<std::string>::create("stencil_computer","Stencil Computer","Builder name of the stencil computer",std::string("stencilcomputer")))
    ->attach_trigger( boost::bind( &CInterpolator::configure_stencil_computer, this ) )
    ->mark_basic();
    
  properties().add_option(OptionT<std::string>::create("function","Interpolator Function","Builder name of the interpolator function",std::string("function")))
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
  m_stencil_computer = create_component_abstract_type<CStencilComputer>(property("stencil_computer").value<std::string>(),"stencil_computer");
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
