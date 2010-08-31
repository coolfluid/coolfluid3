#include <boost/foreach.hpp>
#include <boost/filesystem/path.hpp>

#include "Common/Log.hpp"
#include "Common/OptionT.hpp"
#include "Common/OptionArray.hpp"
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

void CMeshReader::regist_signals ( CMeshReader* self )
{
  self->regist_signal ( "read" , "reads a mesh" )->connect ( boost::bind ( &CMeshReader::read, self, _1 ) );
}

////////////////////////////////////////////////////////////////////////////////

void CMeshReader::defineConfigOptions(Common::OptionList& options)
{
  std::vector<boost::filesystem::path> dummy;
  options.add< OptionArrayT<boost::filesystem::path> >  ( "Files",  "Files to read" , dummy );
  options.add< OptionT<std::string> >  ( "Mesh",  "Mesh to construct" , "" );
}

//////////////////////////////////////////////////////////////////////////////

void CMeshReader::read( XmlNode& node  )
{
  // Get the mesh
  CMesh::Ptr mesh = look_component_type<CMesh>( option("Mesh")->value<std::string>() );

  // Get the file paths
  std::vector<boost::filesystem::path> files;
  BOOST_FOREACH(boost::filesystem::path file, option("Files")->value<std::vector<boost::filesystem::path> >())
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
