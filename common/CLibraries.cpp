// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iostream>

#include <boost/algorithm/string.hpp>

#include "common/Log.hpp"
#include "common/Signal.hpp"
#include "common/CLibrary.hpp"
#include "common/CBuilder.hpp"
#include "common/CLibraries.hpp"
#include "common/OSystem.hpp"
#include "common/OptionArray.hpp"
#include "common/LibLoader.hpp"
#include "common/Foreach.hpp"
#include "common/FindComponents.hpp"

#include "common/XML/SignalOptions.hpp"
#include "common/XML/SignalFrame.hpp"
#include "common/XML/Protocol.hpp"
#include "common/XML/XmlNode.hpp"

using namespace cf3::common::XML;

namespace cf3 {
namespace common {

////////////////////////////////////////////////////////////////////////////////

CLibraries::CLibraries ( const std::string& name) : Component ( name )
{
  TypeInfo::instance().regist<CLibraries>(CLibraries::type_name());

  m_properties["brief"] = std::string("Library loader");
  m_properties["description"] = std::string("Loads external libraries, and holds links to all builders each library offers");

  // signals
  regist_signal( "load_libraries" )
    ->connect( boost::bind( &CLibraries::signal_load_libraries, this, _1 ) )
    ->description("loads libraries")
    ->pretty_name("Load Libraries");

  signal("create_component")->hidden(true);
  signal("rename_component")->hidden(true);
  signal("move_component")->hidden(true);
  signal("delete_component")->hidden(true);

  signal("load_libraries")->signature( boost::bind(&CLibraries::signature_load_libraries, this, _1) );
}

////////////////////////////////////////////////////////////////////////////////

CLibraries::~CLibraries()
{
}

////////////////////////////////////////////////////////////////////////////////

std::string CLibraries::namespace_to_libname( const std::string& libnamespace )
{
  // Copy holding the result
  std::string result = libnamespace;

  if( boost::starts_with(result, "CF.") )
      boost::replace_first(result, "CF", "coolfluid");

  boost::replace_all(result, ".", "_");
  boost::to_lower(result);

  return result;
}

////////////////////////////////////////////////////////////////////////////////

bool CLibraries::is_loaded( const std::string& name )
{
  return is_not_null( get_child_ptr( name ) );
}

////////////////////////////////////////////////////////////////////////////////

void CLibraries::load_library( const URI& file )
{
  if( file.empty() || file.scheme() != URI::Scheme::FILE )
    throw InvalidURI( FromHere(), "Expected a file:// got \'" + file.string() + "\'" );

  boost::filesystem::path fpath( file.path() );

  return OSystem::instance().lib_loader()->load_library( fpath.string() );
}

////////////////////////////////////////////////////////////////////////////////

CLibrary::Ptr CLibraries::autoload_library_with_namespace( const std::string& libnamespace )
{
  if( libnamespace.empty() )
    throw BadValue( FromHere(), "Library namespace is empty - cannot guess library name" );

  CLibrary::Ptr lib;

  const std::string lib_name = namespace_to_libname( libnamespace );

  try // to auto-load in case builder not there
  {
    CFinfo << "Auto-loading plugin " << lib_name << CFendl;
    OSystem::instance().lib_loader()->load_library(lib_name);
    lib = get_child( libnamespace ).as_ptr_checked<CLibrary>();
  }
  catch(const std::exception& e)
  {
    throw ValueNotFound(FromHere(),
                        "Failed to auto-load plugin " + lib_name + ": " + e.what());
  }

  cf3_assert( is_not_null(lib) );

  return lib;
}

////////////////////////////////////////////////////////////////////////////////

CLibrary::Ptr CLibraries::autoload_library_with_builder( const std::string& builder_name )
{
  if( builder_name.empty() )
    throw BadValue( FromHere(), "Builder name is empty - cannot guess library name" );

  CLibrary::Ptr lib;

  const std::string libnamespace = CBuilder::extract_namespace( builder_name );
  const std::string lib_name = namespace_to_libname( libnamespace );

  try // to auto-load in case builder not there
  {
    CFinfo << "Auto-loading plugin " << lib_name << CFendl;
    OSystem::instance().lib_loader()->load_library(lib_name);
    lib = get_child( libnamespace ).as_ptr_checked<CLibrary>();
  }
  catch(const std::exception& e)
  {
    throw ValueNotFound(FromHere(),
                        "Failed to auto-load plugin " + lib_name + ": " + e.what());
  }

  cf3_assert( is_not_null(lib) );

  return lib;
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

//  std::vector<URI::Scheme::Type> schemes(1);
//  schemes[0] = URI::Scheme::FILE;

  std::vector<URI> dummy;

  options.add_option< OptionArrayT<URI> >("libs", dummy)
      ->description("Libraries to load");
      //->cast_to<OptionURI>()->set_supported_protocols(schemes);

}

////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3
