// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/filesystem/path.hpp>
#include <boost/mpi/collectives.hpp>

#include "Common/Log.hpp"
#include "Common/OptionT.hpp"
#include "Common/OptionArray.hpp"
#include "Common/OptionURI.hpp"
#include "Common/ComponentPredicates.hpp"
#include "Common/Foreach.hpp"
#include "Common/MPI/PE.hpp"

#include "Mesh/CMeshReader.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CDomain.hpp"

namespace CF {
namespace Mesh {

using namespace Common;

////////////////////////////////////////////////////////////////////////////////

CMeshReader::CMeshReader ( const std::string& name  ) :
  Component ( name )
{
  // signals
  this->regist_signal ( "read" , "reads a mesh", "Read mesh" )->connect ( boost::bind ( &CMeshReader::signal_read, this, _1 ) );
  signal("read").signature
      .insert<URI>("Domain", "Domain to load mesh into" )
      .insert_array<URI>( "Files" , "Files to read" );

  // options

//  std::vector< URI > dummy;
//  m_properties.add_option< OptionArrayT<URI> > ( "Files",  "Files to read" , dummy );

//  m_properties["Files"].as_option().mark_basic();

}

////////////////////////////////////////////////////////////////////////////////

CMeshReader::~CMeshReader()
{
}

//////////////////////////////////////////////////////////////////////////////

void CMeshReader::signal_read( XmlNode& xml  )
{
  XmlParams p (xml);

  URI path = p.get_option<URI>("Domain");

  if( path.scheme() != URI::Scheme::CPATH )
    throw ProtocolError( FromHere(), "Wrong protocol to access the Domain component, expecting a \'cpath\' but got \'" + path.string() +"\'");

  // get the domain
  CDomain::Ptr domain = look_component<CDomain>( path );
  if (!domain)
    throw CastingFailed( FromHere(), "Component in path \'" + path.string() + "\' is not a valid CDomain." );

  // std::vector<URI> files = property("Files").value<std::vector<URI> >();
  std::vector<URI> files = p.get_array<URI>("Files");

  // check protocol for file loading
  boost_foreach(URI file, files)
  {
    if( file.empty() || file.scheme() != URI::Scheme::FILE )
      throw ProtocolError( FromHere(), "Wrong protocol to access the file, expecting a \'file\' but got \'" + file.string() + "\'" );
  }

  // create a mesh in the domain
  if( !files.empty() )
  {
    CMesh::Ptr mesh = domain->create_component<CMesh>("Mesh");

    // Get the file paths
    boost_foreach(URI file, files)
    {
      boost::filesystem::path fpath( file.string_without_scheme() );
      read_from_to(fpath, mesh);
    }
  }
  else
  {
    throw BadValue( FromHere(), "No mesh was read because no files were selected." );
  }
}

//////////////////////////////////////////////////////////////////////////////

CMesh::Ptr CMeshReader::create_mesh_from(boost::filesystem::path& file)
{
  // Create the mesh
  CMesh::Ptr mesh ( allocate_component<CMesh>("mesh") );

  // Call implementation
  read_from_to(file,mesh);

  // return the mesh
  return mesh;
}

//////////////////////////////////////////////////////////////////////////////

CMeshReader::BufferMap
  CMeshReader::create_element_regions_with_buffermap (CRegion& parent_region, CNodes& nodes,
                                                    const std::vector<std::string>& etypes)
{
  // Create regions for each element type
  BufferMap buffermap;
  boost_foreach(const std::string& etype, etypes)
  {
    CElements& etype_region = parent_region.create_elements(etype,nodes);
    // CFinfo << "create: " << etype_region->full_path().string() << "\n" << CFflush;

    buffermap[etype] = boost::shared_ptr<CTable<Uint>::Buffer> (new CTable<Uint>::Buffer(etype_region.connectivity_table().create_buffer()));
  }
  return buffermap;
}

////////////////////////////////////////////////////////////////////////////////

void CMeshReader::remove_empty_element_regions(CRegion& parent_region)
{

  boost_foreach(CElements& region, find_components_recursively<CElements>(parent_region))
  {
    // find the empty regions
    bool empty_on_this_rank = region.connectivity_table().array().empty();
    bool empty_on_all_ranks = empty_on_this_rank;
    if (mpi::PE::instance().is_init()) {
      boost::mpi::communicator world;
      empty_on_all_ranks = boost::mpi::all_reduce(world, empty_on_this_rank, std::logical_and<bool>());
    }
    if ( empty_on_all_ranks )
    {
      CElements::Ptr removed = boost::dynamic_pointer_cast<CElements>(region.get_parent()->remove_component(region.name()));
      removed.reset();
    }
  }

  // loop over regions
  boost_foreach(CRegion& region, find_components_recursively<CRegion>(parent_region))
  {
    // find the empty regions
    if ( find_components<CRegion>(region).empty() && find_components<CElements>(region).empty() )
      {
        CRegion::Ptr removed = boost::dynamic_pointer_cast<CRegion>(region.get_parent()->remove_component(region.name()));
        removed.reset();
      }
  }
}

} // Mesh
} // CF
