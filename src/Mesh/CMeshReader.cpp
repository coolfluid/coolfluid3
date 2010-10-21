// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/foreach.hpp>
#include <boost/filesystem/path.hpp>

#include "Common/Log.hpp"
#include "Common/OptionT.hpp"
#include "Common/OptionArray.hpp"
#include "Common/OptionURI.hpp"
#include "Common/ComponentPredicates.hpp"
#include "Common/MPI/PEInterface.hpp"

#include "Mesh/CMeshReader.hpp"
#include "Mesh/CRegion.hpp"

namespace CF {
namespace Mesh {

using namespace Common;

////////////////////////////////////////////////////////////////////////////////

CMeshReader::CMeshReader ( const CName& name  ) :
	Component ( name )
{
	BUILD_COMPONENT;
	comm_pattern = SimpleCommunicationPattern(); // must be created after MPI init
}

////////////////////////////////////////////////////////////////////////////////

CMeshReader::~CMeshReader()
{
}

////////////////////////////////////////////////////////////////////////////////

void CMeshReader::regist_signals ( CMeshReader* self )
{
  self->regist_signal ( "read" , "reads a mesh", "Read mesh" )->connect ( boost::bind ( &CMeshReader::read, self, _1 ) );
}

////////////////////////////////////////////////////////////////////////////////

void CMeshReader::defineConfigProperties(Common::PropertyList& options)
{
  std::vector<boost::filesystem::path> dummy;
  Option::Ptr option;
  OptionURI::Ptr option_uri;

  options.add_option< OptionArrayT<boost::filesystem::path> >  ( "Files",  "Files to read" , dummy );
  option = options.add_option<OptionURI> ( "Mesh",  "Mesh to construct" , "" );

  options["Files"].as_option().mark_basic();
  options["Mesh"].as_option().mark_basic();

  option_uri = boost::dynamic_pointer_cast<OptionURI> (option);
  option_uri->supported_protocol("cpath");
}

//////////////////////////////////////////////////////////////////////////////

void CMeshReader::read( XmlNode& node  )
{
  // Get the mesh
  CMesh::Ptr mesh = look_component_type<CMesh>( property("Mesh").value<std::string>() );

  // Get the file paths
  std::vector<boost::filesystem::path> files;
  BOOST_FOREACH(boost::filesystem::path file, property("Files").value<std::vector<boost::filesystem::path> >())
    read_from_to(file,mesh);
}

//////////////////////////////////////////////////////////////////////////////

CMesh::Ptr CMeshReader::create_mesh_from(boost::filesystem::path& file)
{
  // Create the mesh
  CMesh::Ptr mesh (new CMesh("mesh"));

  // Call implementation
  read_from_to(file,mesh);

  // return the mesh
  return mesh;
}

//////////////////////////////////////////////////////////////////////////////

CMeshReader::BufferMap
  CMeshReader::create_element_regions_with_buffermap (CRegion& parent_region, CArray& coordinates,
                                                    const std::vector<std::string>& etypes)
{
  // Create regions for each element type
  BufferMap buffermap;
  BOOST_FOREACH(const std::string& etype, etypes)
  {
    CElements& etype_region = parent_region.create_elements(etype,coordinates);
    // CFinfo << "create: " << etype_region->full_path().string() << "\n" << CFflush;

    buffermap[etype] = boost::shared_ptr<CTable::Buffer> (new CTable::Buffer(etype_region.connectivity_table().create_buffer()));
  }
  return buffermap;
}

////////////////////////////////////////////////////////////////////////////////

void CMeshReader::remove_empty_element_regions(CRegion& parent_region)
{

  BOOST_FOREACH(CElements& region, recursive_range_typed<CElements>(parent_region))
  {
    // find the empty regions
		bool empty_on_this_rank = region.connectivity_table().array().empty();
		bool empty_on_all_ranks = boost::mpi::all_reduce(PEInterface::instance(), empty_on_this_rank, std::logical_and<bool>());
    if ( empty_on_all_ranks )
		{
			// no elements in connectivity table --> remove this region
			//CFinfo << "remove: " << region->full_path().string() << "\n" << CFflush;
			CElements::Ptr removed = boost::dynamic_pointer_cast<CElements>(region.get_parent()->remove_component(region.name()));
			removed.reset();
		}
  }

  // loop over regions
  BOOST_FOREACH(CRegion& region, recursive_range_typed<CRegion>(parent_region))
  {
    // find the empty regions
    if ( range_typed<CRegion>(region).empty() && range_typed<CElements>(region).empty() )
      {
        // no elements in connectivity table --> remove this region
        //CFinfo << "remove: " << region->full_path().string() << "\n" << CFflush;
        CRegion::Ptr removed = boost::dynamic_pointer_cast<CRegion>(region.get_parent()->remove_component(region.name()));
        removed.reset();
      }
  }
}

void CMeshReader::collect_from_other_ranks()
{
	throw NotImplemented(FromHere(),"");
}

} // Mesh
} // CF
