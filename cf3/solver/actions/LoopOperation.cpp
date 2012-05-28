// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "common/Builder.hpp"
#include "common/OptionT.hpp"
#include "common/OptionURI.hpp"
#include "common/OptionComponent.hpp"
#include "common/OptionList.hpp"

#include "common/List.hpp"
#include "mesh/Elements.hpp"

#include "solver/actions/LoopOperation.hpp"


/////////////////////////////////////////////////////////////////////////////////////

using namespace cf3::common;
using namespace cf3::mesh;

namespace cf3 {
namespace solver {
namespace actions {

///////////////////////////////////////////////////////////////////////////////////////

LoopOperation::LoopOperation ( const std::string& name ) :
  common::Action(name),
  m_can_start_loop(true),
  m_call_config_elements(true),
  m_idx(0)
{
  // Following option is ignored if the loop is not about elements
  options().add("elements", URI())
      .supported_protocol(URI::Scheme::CPATH)
      .description("Elements that are being looped")
      .attach_trigger ( boost::bind ( &LoopOperation::config_elements,   this ) );

  options().add("loop_index", 0u)
      .description("Index that is being looped")
      .link_to( &m_idx );

}

////////////////////////////////////////////////////////////////////////////////

void LoopOperation::config_elements()
{
  // Safeguard in case elements are set using set_elements
  // otherwise this would get triggered
  if (m_call_config_elements)
  {
    const URI uri = options().value<URI>("elements");
    m_elements = access_component_checked(uri)->handle<Entities>();
    if ( is_null(m_elements) )
      throw CastingFailed (FromHere(), "Elements must be of a Entities or derived type");
  }
}

////////////////////////////////////////////////////////////////////////////////

void LoopOperation::set_elements(Entities& elements)
{
  // disable LoopOperation::config_elements() trigger
  m_call_config_elements = false;

  // Set elements
  m_elements = elements.handle<Entities>();

  // Call triggers
  options().option("elements").trigger();

  // re-enable Loop::Operation::config_elements() trigger
  m_call_config_elements = true;
}

////////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3

////////////////////////////////////////////////////////////////////////////////////

