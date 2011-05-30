// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Signal.hpp"
#include "Common/CLibrary.hpp"
#include "Common/CLibraries.hpp"
#include "Common/OSystem.hpp"
#include "Common/LibLoader.hpp"
#include "Common/Foreach.hpp"
#include "Common/FindComponents.hpp"

#include "Common/XML/SignalOptions.hpp"
#include "Common/XML/SignalFrame.hpp"
#include "Common/XML/Protocol.hpp"
#include "Common/XML/XmlNode.hpp"

using namespace CF::Common::XML;

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

CLibraries::CLibraries ( const std::string& name) : Component ( name )
{
  TypeInfo::instance().regist<CLibraries>(CLibraries::type_name());

  m_properties["brief"] = std::string("Library loader");
  m_properties["description"] = std::string("Loads external libraries, and holds links to all builders each library offers");

  // signals
  regist_signal ( "load_libraries" , "loads libraries", "Load Libraries" )->signal->connect ( boost::bind ( &CLibraries::signal_load_libraries, this, _1 ) );

  signal("create_component")->is_hidden = true;
  signal("rename_component")->is_hidden = true;
  signal("move_component")->is_hidden = true;
  signal("delete_component")->is_hidden = true;

  signal("load_libraries")->signature->connect( boost::bind(&CLibraries::signature_load_libraries, this, _1) );
}

////////////////////////////////////////////////////////////////////////////////

CLibraries::~CLibraries()
{
}

////////////////////////////////////////////////////////////////////////////////

void CLibraries::load_library( const URI& file )
{
  if( file.empty() || file.scheme() != URI::Scheme::FILE )
    throw InvalidURI( FromHere(), "Expected a file:// got \'" + file.string() + "\'" );

  boost::filesystem::path fpath( file.path() );

  OSystem::instance().lib_loader()->load_library( fpath.string() );
}

////////////////////////////////////////////////////////////////////////////////

void CLibraries::initiate_all_libraries()
{
  boost_foreach( CLibrary& lib, find_components<CLibrary>(*this) )
  {
    lib.initiate(); // will do nothing if already initiated
  }
}

////////////////////////////////////////////////////////////////////////////////

void CLibraries::terminate_all_libraries()
{
  boost_foreach( CLibrary& lib, find_components<CLibrary>(*this) )
  {
    lib.terminate(); // will do nothing if already terminated
  }
}

////////////////////////////////////////////////////////////////////////////////

void CLibraries::signal_load_libraries ( SignalArgs& args )
{
  SignalOptions opts (args);

  std::vector<URI> files = opts.array<URI>("libs");

  // check protocol for file loading
  if( !files.empty() )
  {
    boost_foreach( URI file, files )
    {
      load_library(file);
    }
  }
  else
    throw BadValue( FromHere(), "No library was loaded because no files were selected." );
}

////////////////////////////////////////////////////////////////////////////////

void CLibraries::signature_load_libraries ( SignalArgs& args )
{
  SignalOptions options( args );

  std::vector<URI::Scheme::Type> schemes(1);
  schemes[0] = URI::Scheme::FILE;

  std::vector<URI> dummy;

  options.add("libs", dummy, " ; ", "Libraries to load", schemes );

}

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF
