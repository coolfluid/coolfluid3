// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Signal.hpp"
#include "Common/OptionURI.hpp"
#include "Common/OptionArray.hpp"
#include "Common/OptionT.hpp"
#include "Common/CEnv.hpp"
#include "Common/Core.hpp"

#include "Mesh/CMeshWriter.hpp"
#include "Mesh/MeshMetadata.hpp"
#include "Mesh/CNodes.hpp"
#include "Mesh/Field.hpp"

namespace CF {
namespace Mesh {

using namespace Common;

////////////////////////////////////////////////////////////////////////////////

CMeshWriter::CMeshWriter ( const std::string& name  ) :
  CAction ( name )
{
  mark_basic();

  std::vector<URI> fields;
  m_options.add_option< OptionArrayT<URI> >("fields",fields)
      ->description("Fields to ouptut")
      ->mark_basic()
      ->attach_trigger( boost::bind( &CMeshWriter::config_fields, this ) );

  // Path to the mesh to write
  m_options.add_option( OptionURI::create("mesh", URI(), URI::Scheme::CPATH) )
      ->description("Mesh to write")
      ->pretty_name("Mesh")
      ->mark_basic();

  // Output file path
  m_options.add_option( OptionURI::create("file", URI("mesh", URI::Scheme::FILE), URI::Scheme::FILE) )
      ->description("File to write")
      ->pretty_name("File")
      ->mark_basic();

  // signal for writing the mesh
  regist_signal( "write_mesh" )
    ->connect( boost::bind( &CMeshWriter::signal_write, this, _1 ) )
    ->description("Write the mesh")
    ->pretty_name("Write Mesh");
}

////////////////////////////////////////////////////////////////////////////////

void CMeshWriter::config_fields()
{
  std::vector<URI> field_uris;
  m_options["fields"].put_value(field_uris);

  m_fields.resize(0);
  boost_foreach ( const URI& uri, field_uris)
  {
    m_fields.push_back(access_component_ptr_checked(uri)->as_ptr_checked<Field>());
    if ( is_null(m_fields.back().lock()) )
      throw ValueNotFound(FromHere(),"Invalid URI ["+uri.string()+"]");
  }
}

////////////////////////////////////////////////////////////////////////////////

void CMeshWriter::set_fields(const std::vector<Field::Ptr>& fields)
{
  m_fields.resize(0);
  boost_foreach( Field::Ptr field, fields )
    m_fields.push_back(field);
}

////////////////////////////////////////////////////////////////////////////////

CMeshWriter::~CMeshWriter()
{
}

//////////////////////////////////////////////////////////////////////////////

void CMeshWriter::signal_write( SignalArgs& node  )
{
  execute();
}

//////////////////////////////////////////////////////////////////////////////

void CMeshWriter::execute()
{
  // Get the mesh
  const CMesh& mesh = access_component(option("mesh").value<URI>()).as_type<CMesh>();

  // Get the file path
  std::string file = option("file").value<URI>().string();

  // Call implementation
  write_from_to(mesh,file);
}

//////////////////////////////////////////////////////////////////////////////


} // Mesh
} // CF
