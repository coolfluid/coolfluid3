#include <boost/foreach.hpp>

#include "Common/Log.hpp"
#include "Common/OptionT.hpp"
#include "Common/ComponentPredicates.hpp"

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
}

////////////////////////////////////////////////////////////////////////////////

CMeshReader::~CMeshReader()
{
}

////////////////////////////////////////////////////////////////////////////////

void CMeshReader::defineConfigOptions(Common::OptionList& options)
{
  //options.add< OptionT<std::string> >  ( "File",  "File to read" , "" );
  //options.add< Common::OptionT<std::string> >  ( "Mesh",  "Mesh to construct" , "" );
}

//////////////////////////////////////////////////////////////////////////////

void CMeshReader::read( XmlNode& node  )
{
  // Get the mesh component in the tree
  /// @todo[1]: wait for Tiago for functionality

  // Get the file path
  boost::filesystem::path file = option("File")->value<std::string>();

  // Call implementation
  /// @todo wait for todo[1]
  // read_from_to(file,mesh);

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
  CMeshReader::create_element_regions_with_buffermap (CRegion& parent_region, CArray::Ptr coordinates,
                                                    const std::vector<std::string>& etypes)
{
  // Create regions for each element type
  BufferMap buffermap;
  BOOST_FOREACH(const std::string& etype, etypes)
  {
    CElements& etype_region = parent_region.create_elements(etype,coordinates);
    // CFinfo << "create: " << etype_region->full_path().string() << "\n" << CFflush;
    buffermap[etype]=boost::shared_ptr<CTable::Buffer>
      (new CTable::Buffer(etype_region.connectivity_table().create_buffer()));
  }
  return buffermap;
}

////////////////////////////////////////////////////////////////////////////////

void CMeshReader::remove_empty_element_regions(CRegion& parent_region)
{
  BOOST_FOREACH(CElements& region, recursive_range_typed<CElements>(parent_region))
  {
    // find the empty regions
    if ( region.connectivity_table().table().empty() )
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


} // Mesh
} // CF
