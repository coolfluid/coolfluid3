// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Signal.hpp"
#include "Common/OptionURI.hpp"
#include "Common/OptionArray.hpp"
#include "Common/OptionT.hpp"

#include "Mesh/CMeshWriter.hpp"
#include "Mesh/CNodes.hpp"
#include "Mesh/CField2.hpp"

namespace CF {
namespace Mesh {

using namespace Common;

////////////////////////////////////////////////////////////////////////////////

CMeshWriter::CMeshWriter ( const std::string& name  ) :
  Component ( name ), m_coord_dim(0), m_max_dimensionality(0)
{
  mark_basic();

  std::vector<URI> fields;
  m_properties.add_option< OptionArrayT<URI> >("Fields","Fields to output",fields)
      ->mark_basic()
      ->attach_trigger( boost::bind( &CMeshWriter::config_fields, this ) );

  // m_properties.add_option< OptionT<std::string> >  ( "File",  "File to read" , "" );
  // m_properties.add_option< OptionT<std::string> >  ( "Mesh",  "Mesh to construct" , "" );

  // Path to the mesh to write
  Common::OptionURI::Ptr mesh_path =
      boost::dynamic_pointer_cast<Common::OptionURI>( properties().add_option<Common::OptionURI>("Mesh", "Mesh to write", std::string()) );
  mesh_path->supported_protocol(CF::Common::URI::Scheme::CPATH);
  mesh_path->mark_basic();

  // Output file path
  properties().add_option< OptionT<std::string> >("File", "File to save to", std::string())
      ->mark_basic();
  //file_path->supported_protocol(CF::Common::URI::Scheme::FILE);

  // signal for writing the mesh
  regist_signal("write_mesh" , "Write the mesh", "Write Mesh")->
      signal->connect( boost::bind ( &CMeshWriter::signal_write, this, _1 ) );
}

////////////////////////////////////////////////////////////////////////////////

void CMeshWriter::config_fields()
{
  std::vector<URI> field_uris;
  m_properties["Fields"].put_value(field_uris);

  m_fields.resize(0);
  boost_foreach ( const URI& uri, field_uris)
  {
    m_fields.push_back(access_component_ptr(uri)->as_ptr<CField2>());
    if ( is_null(m_fields.back().lock()) )
      throw ValueNotFound(FromHere(),"Invalid URI ["+uri.string()+"]");
  }
}

////////////////////////////////////////////////////////////////////////////////

void CMeshWriter::set_fields(const std::vector<CField2::Ptr>& fields)
{
  m_fields.resize(0);
  boost_foreach( CField2::Ptr field, fields )
    m_fields.push_back(field);
}

////////////////////////////////////////////////////////////////////////////////

CMeshWriter::~CMeshWriter()
{
}

//////////////////////////////////////////////////////////////////////////////

void CMeshWriter::signal_write( SignalArgs& node  )
{
  write();
}

//////////////////////////////////////////////////////////////////////////////

void CMeshWriter::write()
{
  // Get the mesh
  CMesh::Ptr mesh = access_component_ptr(property("Mesh").value_str())->as_ptr<CMesh>();

  // Get the file path
  boost::filesystem::path file (property("File").value_str());

  // Call implementation
  write_from_to(mesh,file);
}

//////////////////////////////////////////////////////////////////////////////

void CMeshWriter::compute_mesh_specifics()
{
  if( is_null(m_mesh) )
    throw SetupError( FromHere(), "Mesh has not been configured in writer");

  // - Assemble the map that gives a list of elementregions for each coordinate component
  // - Find maximal dimensionality of the whole mesh
  m_all_nodes.clear();
  m_max_dimensionality = 0;
  m_coord_dim = 0;
  BOOST_FOREACH(CElements& elements, find_components_recursively<CElements>(*m_mesh))
  {
    m_all_nodes[&elements.nodes()].push_back(&elements);
    m_max_dimensionality = std::max(elements.element_type().dimensionality() , m_max_dimensionality);
    m_coord_dim = std::max((Uint) elements.nodes().coordinates().row_size() , m_coord_dim);
  }
}

//////////////////////////////////////////////////////////////////////////////


} // Mesh
} // CF
