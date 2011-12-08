// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include "common/OptionComponent.hpp"
#include "common/OptionList.hpp"
#include "common/OptionList.hpp"
#include "common/OptionT.hpp"

#include "mesh/Interpolator.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Field.hpp"
#include "mesh/StencilComputer.hpp"

#include "common/OptionList.hpp"

#include "common/OptionList.hpp"

namespace cf3 {
namespace mesh {

using namespace common;
using namespace common::XML;

////////////////////////////////////////////////////////////////////////////////

Interpolator::Interpolator ( const std::string& name  ) :
  Component ( name )
{
  options().add_option("source", m_source)
      .description("Field to interpolate from")
      .pretty_name("Source Field")
      .mark_basic()
      .link_to(&m_source);

  options().add_option("target", m_target)
      .description("Field to interpolate to")
      .pretty_name("TargetField")
      .mark_basic()
      .link_to(&m_target);

  options().add_option("store", true)
      .description("Flag to store weights and stencils used for faster interpolation")
      .pretty_name("Store");

  options().add_option("stencil_computer", std::string("stencilcomputer"))
      .description("Builder name of the stencil computer")
      .pretty_name("Stencil Computer")
      .attach_trigger( boost::bind( &Interpolator::configure_stencil_computer, this ) )
      .mark_basic();

  options().add_option("function", std::string("function"))
      .description("Builder name of the interpolator function")
      .pretty_name("Interpolator Function")
      .attach_trigger( boost::bind( &Interpolator::configure_interpolator_function, this ) )
      .mark_basic();
}

////////////////////////////////////////////////////////////////////////////////

Interpolator::~Interpolator()
{
}

////////////////////////////////////////////////////////////////////////////////

void Interpolator::configure_stencil_computer()
{
  if (is_not_null(m_stencil_computer))
    remove_component(m_stencil_computer->name());
  m_stencil_computer = make_handle(build_component_abstract_type<StencilComputer>(options().option("stencil_computer").value<std::string>(),"stencil_computer"));
}

////////////////////////////////////////////////////////////////////////////////

void Interpolator::configure_interpolator_function()
{

}

//////////////////////////////////////////////////////////////////////////////

void Interpolator::signal_interpolate( SignalArgs& node  )
{
  interpolate();
}

////////////////////////////////////////////////////////////////////////////////

void Interpolator::interpolate()
{
  if ( is_null(m_source) )
    throw SetupError (FromHere(), "SourceField option was not set");
  if ( is_null(m_target) )
    throw SetupError (FromHere(), "TargetField option was not set");
  construct_internal_storage(*Handle<Mesh>(m_source->parent()));
  interpolate_field_from_to(*m_source,*m_target);
}

////////////////////////////////////////////////////////////////////////////////


} // mesh
} // cf3
