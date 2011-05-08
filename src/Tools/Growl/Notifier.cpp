// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Tools/Growl/Notifier.hpp"

#include "Common/Signal.hpp"
#include "Common/XML/SignalOptions.hpp"

#include "Common/Log.hpp"
#include "Common/OptionT.hpp"
#include "Common/CBuilder.hpp"

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

namespace CF {
namespace Tools {
namespace Growl {

using namespace Common;
using namespace Common::XML;

ComponentBuilder < Notifier, Component, LibGrowl > Notifier_Builder;

////////////////////////////////////////////////////////////////////////////////

Notifier::Notifier ( const std::string& name ) :
  Common::Component(name),
  m_application_name("COOLFluiD")
{

  properties().add_option( OptionT<std::string>::create("application_name","Application Name","Name of the application",m_application_name) )
      ->link_to(&m_application_name);

  regist_signal ( "notify" , "Notify iPhone Growl app", "Notify" )->signal->connect ( boost::bind ( &Notifier::signal_notify, this, _1 ) );
  signal("notify")->signature->connect(boost::bind(&Notifier::signature_notify, this, _1));

}

////////////////////////////////////////////////////////////////////////////////

Notifier::~Notifier()
{
}

////////////////////////////////////////////////////////////////////////////////

void Notifier::notify(const std::string& event, const std::string& description)
{
  int rc;
  char* server = NULL;
  char* password = NULL;
  std::string notify = "coolfluid notify";
  char* title = NULL;
  char* message = NULL;
  char* icon = NULL;
  char* url = NULL;
  int tcpsend = 0;

  std::string mytitle = std::string(m_application_name)+"    "+event;
  title = string_to_utf8_alloc(mytitle.c_str());
  message = string_to_utf8_alloc(description.c_str());
  //icon = string_to_utf8_alloc(argv[optind + 2]);
  //url = string_to_utf8_alloc(argv[optind + 3]);

  if (!server) server = string_to_utf8_alloc("localhost");

  growl_init();
  if (tcpsend)
  {
    rc = growl(server,m_application_name.c_str(),notify.c_str(),title,message,icon,password,url);
  }
  else
  {
    rc = growl_udp(server,m_application_name.c_str(),notify.c_str(),title,message,icon,password,url);
  }
  growl_shutdown();

  if (title) free(title);
  if (message) free(message);
  if (icon) free(icon);
  if (url) free(url);
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

} // Growl
} // Tools
} // CF

////////////////////////////////////////////////////////////////////////////////
