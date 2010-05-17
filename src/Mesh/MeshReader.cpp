#include <boost/foreach.hpp>
#include "Mesh/MeshReader.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

////////////////////////////////////////////////////////////////////////////////

MeshReader::MeshReader()
{
}

////////////////////////////////////////////////////////////////////////////////

MeshReader::~MeshReader()
{
}

////////////////////////////////////////////////////////////////////////////////

std::map<std::string,boost::shared_ptr<CTable::Buffer> > MeshReader::create_buffermap_for_elementConnectivity(CRegion::Ptr& parent_region, std::vector<std::string>& etypes)
{
  // Create regions for each element type
  std::map<std::string,boost::shared_ptr<CTable::Buffer> > buffermap;
  BOOST_FOREACH(std::string& etype, etypes)
  {
    CRegion::Ptr etype_region = parent_region->create_leaf_region(etype);
    CFinfo << "region_name = " << etype << "\n" << CFendl;
    buffermap[etype]=boost::shared_ptr<CTable::Buffer>
      (new CTable::Buffer(etype_region->get_component<CTable>("table")->create_buffer()));
  }
  return buffermap;
}

////////////////////////////////////////////////////////////////////////////////

void MeshReader::remove_empty_leaf_regions(CRegion::Ptr& parent_region)
{
  // Find the empty regions
  for (CRegion::Iterator region=parent_region->begin(); region!=parent_region->end(); ++region)
  {
    if (!region->has_subregions())
    {
      if (region->get_component<CTable>("table")->get_table().size() == 0)
      {
        // no elements in connectivity table --> remove this region
        CFinfo << "remove: " << region->full_path().string() << "\n" << CFendl;
        region->get_parent()->remove_component(region->name());
        region.get_ptr().reset();
      }
    }
  }
}


////////////////////////////////////////////////////////////////////////////////

} // namespace Mesh
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

