// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CLibraries.hpp"
#include "Common/OSystem.hpp"
#include "Common/LibLoader.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

CLibraries::CLibraries ( const std::string& name) : Component ( name )
{
  TypeInfo::instance().regist<CLibraries>(CLibraries::type_name());

  // signals
  regist_signal ( "load_library" , "loads a library", "Load Library" )->connect ( boost::bind ( &CLibraries::load_library, this, _1 ) );

  signal("load_library").signature
      .insert<URI>("Lib", "Library to load" );

  signal("create_component").is_hidden = true;
  signal("rename_component").is_hidden = true;
  signal("move_component").is_hidden = true;
  signal("delete_component").is_hidden = true;
}

////////////////////////////////////////////////////////////////////////////////

CLibraries::~CLibraries()
{
}

void CLibraries::load_library ( XmlNode& node )
{
  XmlParams p ( node );

  URI file = p.get_option<URI>("Lib");
  boost::filesystem::path fpath( file.string_without_scheme() );

  OSystem::instance().lib_loader()->load_library( fpath.string() );

#if 0
  std::vector<URI> files = p.get_array<URI>("Libraries");

  // check protocol for file loading
  BOOST_FOREACH(URI file, files)
  {
    if( file.empty() || file.protocol() != URI::Protocol::FILE )
      throw ProtocolError( FromHere(), "Wrong protocol to access the file, expecting a \'file\' but got \'" + file.string() + "\'" );
  }

  // create a mesh in the domain
  if( !files.empty() )
  {
    // Get the file paths
    BOOST_FOREACH(URI file, files)
    {
      boost::filesystem::path fpath( file.string_without_protocol() );

      OSystem::instance().lib_loader()->load_library( fpath.string_without_protocol() );
    }
  }
  else
  {
    throw BadValue( FromHere(), "No library was loaded because no files were selected." );
  }
#endif
}

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF
