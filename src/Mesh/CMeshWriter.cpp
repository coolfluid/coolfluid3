// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/regex.hpp>
#include <boost/algorithm/string/replace.hpp>

#include "Common/Signal.hpp"
#include "Common/OptionURI.hpp"
#include "Common/OptionArray.hpp"
#include "Common/OptionT.hpp"
#include "Common/CEnv.hpp"
#include "Common/Core.hpp"

#include "Mesh/CMeshWriter.hpp"
#include "Mesh/CNodes.hpp"
#include "Mesh/CField.hpp"

namespace CF {
namespace Mesh {

using namespace Common;

////////////////////////////////////////////////////////////////////////////////

CMeshWriter::CMeshWriter ( const std::string& name  ) :
  CAction ( name ), m_coord_dim(0), m_max_dimensionality(0)
{
  mark_basic();

  std::vector<URI> fields;
  properties().add_option< OptionArrayT<URI> >("fields","Fields to output",fields)
      ->mark_basic()
      ->attach_trigger( boost::bind( &CMeshWriter::config_fields, this ) );

  // Path to the mesh to write
  properties().add_option( OptionURI::create("mesh","Mesh", "Mesh to write", URI(), URI::Scheme::CPATH) )
      ->mark_basic();

  // Output file path
  properties().add_option( OptionURI::create("file","File","File to write",URI("mesh"),URI::Scheme::FILE) )
      ->mark_basic();

  // signal for writing the mesh
  regist_signal("write_mesh" , "Write the mesh", "Write Mesh")->
      signal->connect( boost::bind ( &CMeshWriter::signal_write, this, _1 ) );
}

////////////////////////////////////////////////////////////////////////////////

void CMeshWriter::config_fields()
{
  std::vector<URI> field_uris;
  m_properties["fields"].put_value(field_uris);

  m_fields.resize(0);
  boost_foreach ( const URI& uri, field_uris)
  {
    m_fields.push_back(access_component_ptr(uri)->as_ptr<CField>());
    if ( is_null(m_fields.back().lock()) )
      throw ValueNotFound(FromHere(),"Invalid URI ["+uri.string()+"]");
  }
}

////////////////////////////////////////////////////////////////////////////////

void CMeshWriter::set_fields(const std::vector<CField::Ptr>& fields)
{
  m_fields.resize(0);
  boost_foreach( CField::Ptr field, fields )
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
  const CMesh& mesh = access_component(property("mesh").value<URI>()).as_type<CMesh>();

  // Get the file path
  std::string file = property("file").value<URI>().string();

  // Check for environment variables
  CEnv& environment = Core::instance().environment();
  boost::regex re("\\$\\{(\\w+)\\}");
  boost::sregex_iterator i(file.begin(), file.end(), re);
  boost::sregex_iterator j;
  for(; i!=j; ++i)
  {
    if (environment.properties().check((*i)[1]) )
      boost::algorithm::replace_all(file,std::string((*i)[0]),environment.property((*i)[1]).value_str());
  }

  // Call implementation
  write_from_to(mesh,file);
}

//////////////////////////////////////////////////////////////////////////////

void CMeshWriter::compute_mesh_specifics()
{
  if( is_null(m_mesh) )
    throw SetupError( FromHere(), "Mesh has not been configured in writer");

  // - Find maximal dimensionality of the whole mesh
  m_max_dimensionality = 0;
  m_coord_dim = 0;
  BOOST_FOREACH(const CElements& elements, find_components_recursively<CElements>(*m_mesh))
  {
    m_max_dimensionality = std::max(elements.element_type().dimensionality() , m_max_dimensionality);
    m_coord_dim = std::max((Uint) elements.nodes().coordinates().row_size() , m_coord_dim);
  }
}

//////////////////////////////////////////////////////////////////////////////


} // Mesh
} // CF
