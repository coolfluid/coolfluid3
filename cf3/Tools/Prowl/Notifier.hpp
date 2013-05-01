// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Tools_Prowl_hpp
#define cf3_Tools_Prowl_hpp

#include "Tools/Prowl/LibProwl.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace Tools {
namespace Prowl {

////////////////////////////////////////////////////////////////////////////////

/// @brief %Prowl notifier component
///
/// %Prowl is an iOS app that can receive push notifications (http://www.prowlapp.com/),
/// sent from different sources (most notably <a href="http://growl.info/">Growl</a>)
///
/// To use the %Prowl notifier you will need a %Prowl API key which you can generate their
/// webpage. The API key needs to be configured in this component, but will have already a
/// default value from the Environment Variable "PROWL_API_KEY", if available.
/// @author Willem Deconinck
class Tools_Prowl_API Notifier: public common::Component
{
public: // typedefs

  /// pointers
  
  

public:

  /// %Prowl priorities
  enum Priority  { VERY_LOW=-2,
                   MODERATE=-1,
                   NORMAL=0,
                   HIGH=1,
                   EMERGENCY=2} ;

  /// %Prowl response codes
  enum ResponseCodes { SUCCESS=200,               // notification successful
                       BAD_REQUEST=400,           // the parameters you provided did not validate.
                       NOT_AUTHORIZED=401,        // the API key given is not valid, and does not correspond to a user.
                       NOT_ACCEPTABLE=406,        // your IP address has exceeded the API limit.
                       NOT_APPROVED=409,          // the user has yet to approve your retrieve request.
                       INTERNAL_SERVER_ERROR=500, // something failed to execute properly on the Prowl side.
                       INVALID_RESPONSE=-1};      // the response from Prowl is invalid or could not be parsed.

public: // functions

  /// Contructor
  /// @param name of the component
  Notifier ( const std::string& name );

  /// Get the class name
  static std::string type_name () { return "Notifier"; }

  /// @brief Notify %Prowl
  /// @param event         Header of the notification
  /// @param description   Description of the notification
  /// Application name and priority of the notification are configuration options
  /// The %Prowl API key must be configured, or the environment variable
  /// PROWL_API_KEY must be set
  void notify(const std::string& event, const std::string& description);

  /// @brief Signal to notify %Prowl
  /// @see notify()
  void signal_notify ( common::SignalArgs& node );

  /// @brief signature for the signal_notify() function
  void signature_notify ( common::SignalArgs& node);

private: // data

  /// @brief The %Prowl API key
  ///
  /// Can be configured or set in the environment as well by PROWL_API_KEY
  std::string m_api_key;

  /// Configurable priority for the notifications that are sent
  int m_priority;

  /// Configurable application name, telling %Prowl which application sends
  /// the notification (default = COOLFluiD)
  std::string m_application_name;

};

////////////////////////////////////////////////////////////////////////////////

} // Prowl
} // Tools
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_Tools_Prowl_hpp

