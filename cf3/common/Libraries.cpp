// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iostream>

#include <boost/algorithm/string.hpp>

#include "common/Log.hpp"
#include "common/Signal.hpp"
#include "common/Library.hpp"
#include "common/Builder.hpp"
#include "common/Libraries.hpp"
#include "common/OSystem.hpp"
#include "common/OptionArray.hpp"
#include "common/LibLoader.hpp"
#include "common/Foreach.hpp"
#include "common/FindComponents.hpp"
#include "common/PropertyList.hpp"

#include "common/XML/SignalOptions.hpp"
#include "common/XML/SignalFrame.hpp"
#include "common/XML/Protocol.hpp"
#include "common/XML/XmlNode.hpp"

using namespace cf3::common::XML;

namespace cf3 {
namespace common {

////////////////////////////////////////////////////////////////////////////////

Libraries::Libraries ( const std::string& name) : Component ( name )
{
  TypeInfo::instance().regist<Libraries>(Libraries::type_name());

  properties()["brief"] = std::string("Library loader");
  properties()["description"] = std::string("Loads external libraries, and holds links to all builders each library offers");

  // signals
  regist_signal( "load_libraries" )
    .connect( boost::bind( &Libraries::signal_load_libraries, this, _1 ) )
    .description("loads libraries")
    .pretty_name("Load Libraries");

  signal("create_component")->hidden(true);
  signal("rename_component")->hidden(true);
  signal("move_component")->hidden(true);
  signal("delete_component")->hidden(true);

  signal("load_libraries")->signature( boost::bind(&Libraries::signature_load_libraries, this, _1) );
}

////////////////////////////////////////////////////////////////////////////////

Libraries::~Libraries()
{
}

////////////////////////////////////////////////////////////////////////////////

std::string Libraries::namespace_to_libname( const std::string& libnamespace )
{
  // Copy holding the result
  std::string result = libnamespace;

  if( boost::starts_with(result, "cf3.") )
      boost::replace_first(result, "cf3", "coolfluid");

  boost::replace_all(result, ".", "_");
  boost::to_lower(result);

  return result;
}

////////////////////////////////////////////////////////////////////////////////

bool Libraries::is_loaded( const std::string& name )
{
  return is_not_null( get_child( name ) );
}

////////////////////////////////////////////////////////////////////////////////

void Libraries::load_library( const URI& file )
{
  if( file.empty() || file.scheme() != URI::Scheme::FILE )
    throw InvalidURI( FromHere(), "Expected a file:// got \'" + file.string() + "\'" );

  boost::filesystem::path fpath( file.path() );

  return OSystem::instance().lib_loader()->load_library( fpath.string() );
}

////////////////////////////////////////////////////////////////////////////////

Handle<Library> Libraries::autoload_library_with_namespace( const std::string& libnamespace )
{
  if( libnamespace.empty() )
    throw BadValue( FromHere(), "Library namespace is empty - cannot guess library name" );

  const std::string lib_name = namespace_to_libname( libnamespace );

  try // to auto-load in case builder not there
  {
    CFdebug << "Auto-loading plugin " << lib_name << CFendl;
    OSystem::instance().lib_loader()->load_library(lib_name);
    Handle<Library> lib(get_child( libnamespace ));
    cf3_assert( is_not_null(lib) );
    return lib;
  }
  catch(const std::exception& e)
  {
    CFwarn << "Library " << lib_name << " failed to load with error " << e.what() << CFendl;
    return Handle<Library>();
  }
}

////////////////////////////////////////////////////////////////////////////////

Handle<Library> Libraries::autoload_library_with_builder( const std::string& builder_name )
{
  return autoload_library_with_namespace(Builder::extract_namespace( builder_name ));
}

////////////////////////////////////////////////////////////////////////////////

void Libraries::initiate_all_libraries()
{
  boost_foreach( Library& lib, find_components<Library>(*this) )
  {
    lib.initiate(); // will do nothing if already initiated
  }
}

////////////////////////////////////////////////////////////////////////////////

void Libraries::terminate_all_libraries()
{
  boost_foreach( Library& lib, find_components<Library>(*this) )
  {
    lib.terminate(); // will do nothing if already terminated
  }
}

////////////////////////////////////////////////////////////////////////////////

void Libraries::signal_load_libraries ( SignalArgs& args )
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

void Libraries::signature_load_libraries ( SignalArgs& args )
{
  SignalOptions options( args );

//  std::vector<URI::Scheme::Type> schemes(1);
//  schemes[0] = URI::Scheme::FILE;

  std::vector<URI> dummy;

  options.add("libs", dummy)
      .description("Libraries to load");
      //->cast_to<OptionURI>()->set_supported_protocols(schemes);

}

////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3
