// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"
#include "common/OptionT.hpp"
#include "common/OptionURI.hpp"
#include "common/OptionComponent.hpp"

#include "common/List.hpp"
#include "mesh/Elements.hpp"

#include "solver/actions/CLoopOperation.hpp"


/////////////////////////////////////////////////////////////////////////////////////

using namespace cf3::common;
using namespace cf3::mesh;

namespace cf3 {
namespace solver {
namespace actions {

///////////////////////////////////////////////////////////////////////////////////////

CLoopOperation::CLoopOperation ( const std::string& name ) :
  common::Action(name),
  m_can_start_loop(true),
  m_call_config_elements(true),
  m_idx(0)
{
  // Following option is ignored if the loop is not about elements
  //m_options.add_option(OptionComponent<mesh::Entities>::create("elements","Elements that are being looped",&m_elements));
  options().add_option(OptionURI::create("elements", URI("cpath:"), URI::Scheme::CPATH))
      ->description("Elements that are being looped")
      ->attach_trigger ( boost::bind ( &CLoopOperation::config_elements,   this ) );

  options().add_option< OptionT<Uint> > ("loop_index", 0u)
      ->description("Index that is being looped")
      ->link_to( &m_idx );

}

////////////////////////////////////////////////////////////////////////////////

void CLoopOperation::config_elements()
{
  // Safeguard in case elements are set using set_elements
  // otherwise this would get triggered
  if (m_call_config_elements)
  {
    URI uri;
    option("elements").put_value(uri);
    m_elements = access_component_ptr_checked(uri)->as_ptr_checked<Entities>();
    if ( is_null(m_elements.lock()) )
      throw CastingFailed (FromHere(), "Elements must be of a Entities or derived type");
  }
}

////////////////////////////////////////////////////////////////////////////////

void CLoopOperation::set_elements(Entities& elements)
{
  // disable CLoopOperation::config_elements() trigger
  m_call_config_elements = false;

  // Set elements
  m_elements = elements.as_ptr_checked<Entities>();

  // Call triggers
  option("elements").trigger();

  // re-enable CLoop::Operation::config_elements() trigger
  m_call_config_elements = true;
}

////////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3

////////////////////////////////////////////////////////////////////////////////////

