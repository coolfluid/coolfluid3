// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/algorithm/string.hpp>

#include "common/Builder.hpp"

#include "common/Signal.hpp"
#include "common/LibCommon.hpp"

namespace cf3 {
namespace common {

RegistTypeInfo<Builder,LibCommon> Builder_TypeRegistration();

////////////////////////////////////////////////////////////////////////////////

Builder::Builder ( const std::string& name) : Component ( name )
{
  regist_signal( "create_component" )
    .connect( boost::bind( &Builder::signal_create_component, this, _1 ) )
    .description("builds a component")
    .pretty_name("Build component");

  signal("create_component")->
      signature(boost::bind(&Builder::signature_signal_create_component, this, _1));

}

Builder::~Builder() {}

////////////////////////////////////////////////////////////////////////////////

void Builder::signal_create_component ( SignalArgs& args )
{
  XML::SignalFrame params = args.map( XML::Protocol::Tags::key_options() );

  URI path ( params.get_option<URI>("path") );

  boost::shared_ptr<Component> comp = build ( path.name() );
  Handle<Component> parent = access_component_checked( path.base_path() );
  parent->add_component(comp);
}

////////////////////////////////////////////////////////////////////////////////

void Builder::signature_signal_create_component ( SignalArgs& args )
{
  SignalOptions options( args );
  options.add("path", URI("cpath:")).description("Full path for the created component" );
}

////////////////////////////////////////////////////////////////////////////////////////////

std::string Builder::extract_namespace (const std::string& builder_name)
{
  using namespace boost::algorithm;

  return std::string( builder_name.begin(), find_last(builder_name,".").begin() );
}

////////////////////////////////////////////////////////////////////////////////////////////

std::string Builder::extract_reduced_name (const std::string& builder_name)
{
  using namespace boost::algorithm;

  return std::string( find_last(builder_name,".").end(), builder_name.end() );
}

////////////////////////////////////////////////////////////////////////////////////////////

std::string Builder::extract_library_name (const std::string& builder_name)
{
  return Libraries::namespace_to_libname( extract_namespace( builder_name ) );
}

////////////////////////////////////////////////////////////////////////////////

Builder& find_builder(const std::string& builder_name)
{
  std::string libnamespace = Builder::extract_namespace(builder_name);

  URI builder_path = Core::instance().libraries().uri()
  / URI(libnamespace)
  / URI(builder_name);



  Handle<Builder> cbuilder( follow_link(Core::instance().root().access_component( builder_path )) );

  if( is_null(cbuilder) ) // try to load the library that contains the builder
  {
    if(is_null(Core::instance().libraries().autoload_library_with_builder( builder_name )))
      throw ValueNotFound(FromHere(), "Library for builder " + builder_name + " could not be autoloaded");

    cbuilder = Handle<Builder>(follow_link(Core::instance().root().access_component( builder_path )));
  }

  if( is_null(cbuilder) ) // if still fails, then give up
    throw ValueNotFound( FromHere(), "Could not find builder \'" + builder_name + "\'"
    " neither a plugin library that contains it." );

  return *cbuilder;
}

////////////////////////////////////////////////////////////////////////////////

Handle< Builder > find_builder_ptr(const std::string& builder_name)
{
  std::string libnamespace = Builder::extract_namespace(builder_name);

  URI builder_path = Core::instance().libraries().uri()
  / URI(libnamespace)
  / URI(builder_name);

  Handle<Builder> cbuilder( follow_link(Core::instance().root().access_component( builder_path )) );

  if( is_null(cbuilder) ) // try to load the library that contains the builder
  {
    if(is_null(Core::instance().libraries().autoload_library_with_builder( builder_name )))
      return Handle<Builder>();

    cbuilder = Handle<Builder>(follow_link(Core::instance().root().access_component( builder_path )));
  }

  return cbuilder;
}


} // common
} // cf3
