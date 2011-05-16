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
  regist_signal ( "load_library" , "loads a library", "Load Library" )->signal->connect ( boost::bind ( &CLibraries::signal_load_library, this, _1 ) );

  signal("create_component")->is_hidden = true;
  signal("rename_component")->is_hidden = true;
  signal("move_component")->is_hidden = true;
  signal("delete_component")->is_hidden = true;

  signal("load_library")->signature->connect( boost::bind(&CLibraries::signature_load_library, this, _1) );
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

void CLibraries::signal_load_library ( SignalArgs& args )
{
  SignalFrame p = args.map( Protocol::Tags::key_options() );

  URI file = p.get_option<URI>("lib");

  load_library(file);

#if 0
  std::vector<URI> files = p.get_array<URI>("Libraries");

  // check protocol for file loading
  BOOST_FOREACH(URI file, files)
  {
    if( file.empty() || file.scheme() != URI::Scheme::FILE )
      throw ProtocolError( FromHere(), "Wrong protocol to access the file, expecting a \'file\' but got \'" + file.string() + "\'" );
  }

  if( !files.empty() )
  {
    // Get the file paths
    BOOST_FOREACH(URI file, files)
    {
      boost::filesystem::path fpath( file.path() );

      OSystem::instance().lib_loader()->load_library( fpath.path() );
    }
  }
  else
  {
    throw BadValue( FromHere(), "No library was loaded because no files were selected." );
  }
#endif
}

////////////////////////////////////////////////////////////////////////////////

void CLibraries::signature_load_library ( SignalArgs& args )
{
  SignalFrame p = args.map( Protocol::Tags::key_options() );

  SignalOptions options( args );

  std::vector<URI::Scheme::Type> schemes(1);

  schemes[0] = URI::Scheme::FILE;
  options.add("lib", URI(), "Library to load", schemes );

}

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF
