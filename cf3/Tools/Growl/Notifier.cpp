// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Tools/Growl/Notifier.hpp"

#include "common/Signal.hpp"
#include "common/XML/SignalOptions.hpp"

#include "common/Log.hpp"
#include "common/OptionList.hpp"
#include "common/Builder.hpp"

extern "C" {
#include "Tools/Growl/growl/headers/growl.h"
#include "Tools/Growl/growl/headers/md5.h"
#include "Tools/Growl/growl/headers/tcp.h"

static char* string_to_utf8_alloc(const char* str) {
#ifdef _WIN32
  unsigned int codepage;
  size_t in_len = strlen(str);
  wchar_t* wcsdata;
  char* mbsdata;
  size_t mbssize, wcssize;

  codepage = GetACP();
  wcssize = MultiByteToWideChar(codepage, 0, str, in_len,  NULL, 0);
  wcsdata = (wchar_t*) malloc((wcssize + 1) * sizeof(wchar_t));
  wcssize = MultiByteToWideChar(codepage, 0, str, in_len, wcsdata, wcssize + 1);
  wcsdata[wcssize] = 0;

  mbssize = WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR) wcsdata, -1, NULL, 0, NULL, NULL);
  mbsdata = (char*) malloc((mbssize + 1));
  mbssize = WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR) wcsdata, -1, mbsdata, mbssize, NULL, NULL);
  mbsdata[mbssize] = 0;
  free(wcsdata);
  return mbsdata;
#else
  return strdup(str);
#endif
} // string_to_utf8_alloc()

} // extern "C"

namespace cf3 {
namespace Tools {
namespace Growl {

using namespace common;
using namespace common::XML;

ComponentBuilder < Notifier, Component, LibGrowl > Notifier_Builder;

////////////////////////////////////////////////////////////////////////////////

Notifier::Notifier ( const std::string& name ) :
  common::Component(name),
  m_application_name("COOLFluiD"),
  m_server("localhost"),
  m_password(),
  m_icon("http://coolfluidsrv.vki.ac.be/trac/coolfluid/attachment/wiki/ProjectLogo/coolfluid_simple_logo.png"),
  m_url("http://coolfluidsrv.vki.ac.be"),
  m_protocol(UDP)
{

  // Configuration options

  options().add( "application_name", m_application_name) 
      .description("Name of the application")
      .pretty_name("Application Name")
      .link_to(&m_application_name);

  options().add( "server", m_server) 
      .description("Server to send notification to")
      .pretty_name("Server")
      .link_to(&m_server);

  options().add( "password", m_password) 
      .description("Password for server access")
      .pretty_name("Password")
      .link_to(&m_password);

  options().add( "icon", m_icon) 
      .description("URL to icon")
      .pretty_name("Icon")
      .link_to(&m_icon);

  options().add( "url", m_url) 
      .description("URL that is followd upon clicking the notification")
      .pretty_name("URL")
      .link_to(&m_url);

  options().add( "protocol", m_protocol) 
      .description("Protocol to use: [UDP=0, TCP=1]")
      .pretty_name("Protocol")
      .link_to(&m_protocol);


  // Signals

  regist_signal( "notify" )
    .connect( boost::bind( &Notifier::signal_notify, this, _1 ) )
    .description("Notify iPhone Growl app")
    .pretty_name("Notify");
  signal("notify")->signature(boost::bind(&Notifier::signature_notify, this, _1));


}

////////////////////////////////////////////////////////////////////////////////

Notifier::~Notifier()
{
}

////////////////////////////////////////////////////////////////////////////////

void Notifier::notify(const std::string& event, const std::string& description)
{
  std::string title = m_application_name + "    " + event;

  growl_init();
  int rc;
  switch (m_protocol)
  {
    case UDP:
      rc = growl_udp(m_server.c_str(),
                     m_application_name.c_str(),
                     m_notification_type.c_str(),
                     title.c_str(),
                     description.c_str(),
                     m_icon.c_str(),
                     m_password.c_str(),
                     m_url.c_str());
      break;

    case TCP:
      rc = growl(m_server.c_str(),
                 m_application_name.c_str(),
                 m_notification_type.c_str(),
                 title.c_str(),
                 description.c_str(),
                 m_icon.c_str(),
                 m_password.c_str(),
                 m_url.c_str());
      break;
  }
  growl_shutdown();



  //  This is how growl-send does it
  //  char* server = NULL;
  //  char* password = NULL;
  //  std::string notify = "coolfluid notify";
  //  char* title = NULL;
  //  char* message = NULL;
  //  char* icon = NULL;
  //  char* url = NULL;
  //  int tcpsend = 0;

  //  std::string mytitle = std::string(m_application_name)+"    "+event;
  //  title = string_to_utf8_alloc(mytitle.c_str());
  //  message = string_to_utf8_alloc(description.c_str());
  //  icon = string_to_utf8_alloc("http://coolfluidsrv.vki.ac.be/trac/coolfluid/attachment/wiki/ProjectLogo/coolfluid_simple_logo.png");
  //  url = string_to_utf8_alloc("http://coolfluidsrv.vki.ac.be/");
  //  if (!server) server = string_to_utf8_alloc("localhost");

  //  growl_init();
  //  if (tcpsend)
  //  {
  //    rc = growl(server,m_application_name.c_str(),notify.c_str(),title,message,icon,password,url);
  //  }
  //  else
  //  {
  //    rc = growl_udp(server,m_application_name.c_str(),notify.c_str(),title,message,icon,password,url);
  //  }
  //  growl_shutdown();

  //  if (title) free(title);
  //  if (message) free(message);
  //  if (icon) free(icon);
  //  if (url) free(url);
}

//////////////////////////////////////////////////////////////////////////////

void Notifier::signature_notify ( SignalArgs& node)
{
  SignalOptions options( node );

  options.add("event", "new_event")
      .description("Event name");

  options.add("description" , " ")
      .description("Description of the event");
}

////////////////////////////////////////////////////////////////////////////////

void Notifier::signal_notify ( SignalArgs& node )
{
  SignalOptions options( node );

  std::string event       = options.check("event")       ? options.value<std::string>("event")       : " ";
  std::string description = options.check("description") ? options.value<std::string>("description") : " ";

  notify(event ,description);
}

////////////////////////////////////////////////////////////////////////////////

} // Growl
} // Tools
} // cf3

////////////////////////////////////////////////////////////////////////////////
