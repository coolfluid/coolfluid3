// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/OptionT.hpp"
#include "Common/OptionURI.hpp"
#include "Common/OptionComponent.hpp"

#include "Mesh/CList.hpp"
#include "Mesh/CElements.hpp"

#include "Solver/Actions/CLoopOperation.hpp"


/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Mesh;

namespace CF {
namespace Solver {
namespace Actions {

///////////////////////////////////////////////////////////////////////////////////////

CLoopOperation::CLoopOperation ( const std::string& name ) : 
  Common::CAction(name),
  m_call_config_elements(true),
  m_idx(0),
  m_can_start_loop(true)
{  
  // Following option is ignored if the loop is not about elements
  //m_properties.add_option(OptionComponent<Mesh::CEntities>::create("Elements","Elements that are being looped",&m_elements));
  m_properties.add_option(OptionURI::create("Elements","Elements that are being looped", URI("cpath:"), URI::Scheme::CPATH))
    ->attach_trigger ( boost::bind ( &CLoopOperation::config_elements,   this ) );
  
  m_properties.add_option< OptionT<Uint> > ("LoopIndex","Index that is being looped", 0u )->link_to( &m_idx );
  
}

////////////////////////////////////////////////////////////////////////////////

void CLoopOperation::config_elements()
{
  // Safeguard in case elements are set using set_elements
  // otherwise this would get triggered
  if (m_call_config_elements)
  {
    URI uri;
    property("Elements").put_value(uri);
    m_elements = access_component_ptr<CEntities>(uri);
    if ( is_null(m_elements.lock()) )
      throw CastingFailed (FromHere(), "Elements must be of a CEntities or derived type");    
  }
}

////////////////////////////////////////////////////////////////////////////////

void CLoopOperation::set_elements(CEntities& elements)
{
  // disable CLoopOperation::config_elements() trigger
  m_call_config_elements = false;
  
  // Set elements
  m_elements = elements.as_ptr<CEntities>();

  // Call triggers
  property("Elements").as_option().trigger();
  
  // re-enable CLoop::Operation::config_elements() trigger
  m_call_config_elements = true;
}

////////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Solver
} // CF

////////////////////////////////////////////////////////////////////////////////////

