// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Tools/Prowl/Notifier.hpp"

#include "Common/Signal.hpp"
#include "Common/XML/SignalOptions.hpp"

#include "Common/Log.hpp"
#include "Common/OptionT.hpp"
#include "Common/CBuilder.hpp"

extern "C" {
#include "Tools/Prowl/prowl/prowl.h"
}

namespace CF {
namespace Tools {
namespace Prowl {

using namespace Common;
using namespace Common::XML;

ComponentBuilder < Notifier, Component, LibProwl > Notifier_Builder;

////////////////////////////////////////////////////////////////////////////////

Notifier::Notifier ( const std::string& name ) :
  Common::Component(name),
  m_priority(NORMAL),
  m_application_name("COOLFluiD")
{

  char* env_var = std::getenv("PROWL_API_KEY");
  if (env_var != NULL)
  {
    m_api_key = env_var;
  }

  properties().add_option( OptionT<int>::create("priority","Priority","Priority [-2 = Very Low, -1 = Moderate, 0 = Normal, 1 = High, 2 = Emergency]",m_priority) )
      ->link_to(&m_priority);
  properties().add_option( OptionT<std::string>::create("application_name","Application Name","Name of the application",m_application_name) )
      ->link_to(&m_application_name);
  properties().add_option( OptionT<std::string>::create("api_key","API key","Prowl API key, personal to one iOS device (default = $PROWL_API_KEY)",m_api_key) )
      ->link_to(&m_api_key)
      ->mark_basic();

  regist_signal ( "notify" , "Notify iPhone Prowl app", "Notify" )->signal->connect ( boost::bind ( &Notifier::signal_notify, this, _1 ) );
  signal("notify")->signature->connect(boost::bind(&Notifier::signature_notify, this, _1));

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

  options.add<std::string>("event" , "new_event" , "Event name" );
  options.add<std::string>("description" , " " , "Description of the event" );
}

////////////////////////////////////////////////////////////////////////////////

void Notifier::signal_notify ( SignalArgs& node )
{
  SignalOptions options( node );

  std::string event       = options.exists("event")       ? options.option<std::string>("event")       : " ";
  std::string description = options.exists("description") ? options.option<std::string>("description") : " ";

  notify(event,description);
}

////////////////////////////////////////////////////////////////////////////////

} // Prowl
} // Tools
} // CF

////////////////////////////////////////////////////////////////////////////////
