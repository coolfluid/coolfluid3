// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/OptionComponent.hpp"

#include "Mesh/CInterpolator.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CField2.hpp"

namespace CF {
namespace Mesh {

using namespace Common;
using namespace Common::XML;

////////////////////////////////////////////////////////////////////////////////

CInterpolator::CInterpolator ( const std::string& name  ) :
  Component ( name )
{
  properties().add_option(OptionComponent<CField2>::create("SourceField","Field to interpolate from",&m_source))
    ->mark_basic();
    
  properties().add_option(OptionComponent<CField2>::create("TargetField","Field to interpolate to",&m_target))
    ->mark_basic();

}

////////////////////////////////////////////////////////////////////////////////

CInterpolator::~CInterpolator()
{
}

//////////////////////////////////////////////////////////////////////////////

void CInterpolator::signal_interpolate( Signal::arg_t& node  )
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
  construct_internal_storage(*m_source.lock()->parent()->as_type<CMesh>());
  interpolate_field_from_to(*m_source.lock(),*m_target.lock());
}

////////////////////////////////////////////////////////////////////////////////


} // Mesh
} // CF
