// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

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
//   std::vector<URI> fields;
//   m_properties.add_option< OptionArrayT<URI> > ("Fields","Fields to output",fields);
//   m_properties["Fields"].as_option().attach_trigger( boost::bind( &CMeshWriter::config_fields,   this ) );
  
  // m_properties.add_option< OptionT<std::string> >  ( "File",  "File to read" , "" );
  // m_properties.add_option< OptionT<std::string> >  ( "Mesh",  "Mesh to construct" , "" );
  
  // Path to the mesh to write
  Common::OptionURI::Ptr mesh_path = boost::dynamic_pointer_cast<Common::OptionURI>( properties().add_option<Common::OptionURI>("Mesh", "Mesh to write", std::string()) );
  mesh_path->supported_protocol(CF::Common::URI::Scheme::CPATH);
  mesh_path->mark_basic();
  
  // Output file path
  Common::Option::Ptr file_path = properties().add_option< OptionT<std::string> >("File", "File to save to", std::string());
  //file_path->supported_protocol(CF::Common::URI::Scheme::FILE);
  file_path->mark_basic();
  
  // Signal for writing the mesh
  this->regist_signal("write_mesh" , "Write the mesh", "Write Mesh")->connect( boost::bind ( &CMeshWriter::write, this, _1 ) );
}

////////////////////////////////////////////////////////////////////////////////

void CMeshWriter::config_fields()
{
  std::vector<URI> field_uris;
  m_properties["Fields"].put_value(field_uris);
  
  m_fields.resize(0);
  boost_foreach ( const URI& uri, field_uris)
  {
    m_fields.push_back(look_component<CField2>(uri));
    if ( is_null(m_fields.back().lock()) )
      throw ValueNotFound(FromHere(),"Invalid URI ["+uri.string()+"]");
  }
}

////////////////////////////////////////////////////////////////////////////////

void CMeshWriter::set_fields(const std::vector<boost::shared_ptr<CField2> >& fields)
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

void CMeshWriter::write( XmlNode& node  )
{
//   // Get the mesh component in the tree
//   /// @todo[1]: wait for Tiago for functionality
// 
//   // Get the file path
//   boost::filesystem::path file = property("File").value<std::string>();
// 
//   // Call implementation
//   /// @todo wait for todo[1]
//   // write_from_to(mesh,file);
  CMesh::Ptr mesh = look_component<CMesh>(property("Mesh").value_str());
  write_from(mesh);
}

//////////////////////////////////////////////////////////////////////////////

boost::filesystem::path CMeshWriter::write_from(const CMesh::Ptr& mesh)
{
  // Get the file path
  boost::filesystem::path file = property("File").value_str();

  // Call implementation
  write_from_to(mesh,file);

  // return the file
  return file;
}

//////////////////////////////////////////////////////////////////////////////

void CMeshWriter::compute_mesh_specifics()
{
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
