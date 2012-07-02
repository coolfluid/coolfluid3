// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Tools/Prowl/Notifier.hpp"

#include "common/Signal.hpp"
#include "common/XML/SignalOptions.hpp"

#include "common/Log.hpp"
#include "common/OptionList.hpp"
#include "common/Builder.hpp"

extern "C" {
#include "Tools/Prowl/prowl/prowl.h"
}

namespace cf3 {
namespace Tools {
namespace Prowl {

using namespace common;
using namespace common::XML;

ComponentBuilder < Notifier, Component, LibProwl > Notifier_Builder;

////////////////////////////////////////////////////////////////////////////////

Notifier::Notifier ( const std::string& name ) :
  common::Component(name),
  m_priority(NORMAL),
  m_application_name("COOLFluiD")
{

  char* env_var = std::getenv("PROWL_API_KEY");
  if (env_var != NULL)
  {
    m_api_key = env_var;
  }

  options().add( "priority", m_priority) 
      .description("Priority [-2 = Very Low, -1 = Moderate, 0 = Normal, 1 = High, 2 = Emergency]")
      .pretty_name("Priority")
      .link_to(&m_priority);

  options().add( "application_name", m_application_name) 
      .description("Name of the application")
      .pretty_name("Application Name")
      .link_to(&m_application_name);

  options().add( "api_key", m_api_key) 
      .description("Prowl API key, personal to one iOS device (default = $PROWL_API_KEY)")
      .pretty_name("API key")
      .link_to(&m_api_key)
      .mark_basic();

  regist_signal( "notify" )
    .connect( boost::bind( &Notifier::signal_notify, this, _1 ) )
    .description("Notify iPhone Prowl app")
    .pretty_name("Notify")
    .signature(boost::bind(&Notifier::signature_notify, this, _1));
}

////////////////////////////////////////////////////////////////////////////////

void Notifier::notify(const std::string& event, const std::string& description)
{
   int response = prowl_push_msg(
        const_cast<char*>(m_api_key.c_str()),
        m_priority,
        const_cast<char*>(m_application_name.c_str()),
        const_cast<char*>(event.c_str()),
        const_cast<char*>(description.c_str())
       );

   if (response != SUCCESS)
   {
     switch (response)
     {
        case BAD_REQUEST:
          CFerror << "Bad request, the parameters you provided did not validate" << CFendl;
          CFerror << "Possible priorities: [-2, -1, 0 , 1, 2]" << CFendl;
          break;
        case NOT_AUTHORIZED:
          CFerror << "Not authorized, the given API key ["<<m_api_key<<"] is not valid, and does not correspond to a user." << CFendl;
          break;
        case NOT_ACCEPTABLE:
          CFerror << "Not acceptable, your IP address has exceeded the API limit." << CFendl;
          break;
        case NOT_APPROVED:
          CFerror << "Not approved, the user has yet to approve your retrieve request." << CFendl;
          break;
        case INTERNAL_SERVER_ERROR:
          CFerror << "Internal server error, something failed to execute properly on the Prowl side." << CFendl;
          break;
        case INVALID_RESPONSE:
          CFerror << "Prowl sent an invalid response back" << CFendl;
          break;
        default:
          CFerror << "Error code ["<<response<<"], unknown error" << CFendl;
          break;
     }
   }
}

//////////////////////////////////////////////////////////////////////////////

void Notifier::signature_notify ( SignalArgs& node)
{
  SignalOptions options( node );

  options.add("event", "new_event")
      .description("Event name");

  options.add("description" , " ")
      .description("Description of the event" );
}

////////////////////////////////////////////////////////////////////////////////

void Notifier::signal_notify ( SignalArgs& node )
{
  SignalOptions options( node );

  std::string event       = options.check("event")       ? options.value<std::string>("event")       : " ";
  std::string description = options.check("description") ? options.value<std::string>("description") : " ";

  notify(event,description);
}

////////////////////////////////////////////////////////////////////////////////

} // Prowl
} // Tools
} // cf3

////////////////////////////////////////////////////////////////////////////////
