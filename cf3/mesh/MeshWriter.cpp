// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Signal.hpp"
#include "common/OptionURI.hpp"
#include "common/OptionArray.hpp"
#include "common/OptionList.hpp"
#include "common/OptionT.hpp"
#include "common/OptionList.hpp"
#include "common/Environment.hpp"
#include "common/Core.hpp"

#include "mesh/MeshWriter.hpp"
#include "mesh/MeshMetadata.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Field.hpp"

namespace cf3 {
namespace mesh {

using namespace common;

////////////////////////////////////////////////////////////////////////////////

MeshWriter::MeshWriter ( const std::string& name  ) :
  Action ( name )
{
  mark_basic();

  options().add_option("fields",std::vector<URI>())
      .description("Fields to ouptut")
      .mark_basic()
      .attach_trigger( boost::bind( &MeshWriter::config_fields, this ) );

  // Path to the mesh to write
  options().add_option( "mesh", URI())
      .supported_protocol(URI::Scheme::CPATH)
      .description("Mesh to write")
      .pretty_name("Mesh")
      .mark_basic();

  // Output file path
  options().add_option("file", URI("mesh", URI::Scheme::FILE))
      .supported_protocol(URI::Scheme::FILE)  
      .description("File to write")
      .pretty_name("File")
      .mark_basic();

  // signal for writing the mesh
  regist_signal( "write_mesh" )
    .connect( boost::bind( &MeshWriter::signal_write, this, _1 ) )
    .description("Write the mesh")
    .pretty_name("Write Mesh");
}

////////////////////////////////////////////////////////////////////////////////

void MeshWriter::config_fields()
{
  std::vector<URI> field_uris = options()["fields"].value< std::vector<URI> >();

  m_fields.resize(0);
  boost_foreach ( const URI& uri, field_uris)
  {
    m_fields.push_back(Handle<Field>(access_component_checked(uri)));
    if ( is_null(m_fields.back()) )
      throw ValueNotFound(FromHere(),"Invalid URI ["+uri.string()+"]");
  }
}

////////////////////////////////////////////////////////////////////////////////

void MeshWriter::set_fields(const std::vector<Handle< Field > >& fields)
{
  m_fields.resize(0);
  boost_foreach( Handle< Field > field, fields )
    m_fields.push_back(field);
}

////////////////////////////////////////////////////////////////////////////////

MeshWriter::~MeshWriter()
{
}

//////////////////////////////////////////////////////////////////////////////

void MeshWriter::signal_write( SignalArgs& node  )
{
  execute();
}

//////////////////////////////////////////////////////////////////////////////

void MeshWriter::execute()
{
  // Get the mesh
  Handle<Mesh const> mesh(access_component(options().option("mesh").value<URI>()));

  // Get the file path
  std::string file = options().option("file").value<URI>().string();

  // Call implementation
  write_from_to(*mesh,file);
}

//////////////////////////////////////////////////////////////////////////////


} // mesh
} // cf3
