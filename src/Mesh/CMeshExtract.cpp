#include <boost/foreach.hpp>
#include <boost/regex.hpp>
#include <boost/filesystem.hpp>

#include "Common/ObjectProvider.hpp"
#include "Common/ComponentPredicates.hpp"

#include "Mesh/CMeshExtract.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CRegion.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
  
  using namespace Common;
  
  class IsGroup
  {
  public:
    IsGroup () {}
    
    bool operator()(const Component& component)
    {
      return count(range_typed<CElements>(component));
    }
  }; // IsGroup
  
  
////////////////////////////////////////////////////////////////////////////////

Common::ObjectProvider < Mesh::CMeshExtract,
                         Mesh::CMeshTransformer,
                         Mesh::MeshLib,
                         1 >
CMeshExtract_Provider ( "Extract" );

//////////////////////////////////////////////////////////////////////////////

CMeshExtract::CMeshExtract( const CName& name )
: CMeshTransformer(name)
{
  BUILD_COMPONENT;
}

/////////////////////////////////////////////////////////////////////////////
  
std::string CMeshExtract::brief_description() const
{
  return "Extract given regions from the mesh";
}

/////////////////////////////////////////////////////////////////////////////

std::string CMeshExtract::help() const
{
  std::stringstream out;
  out << "  " << brief_description() << "\n";
  out << "  Usage: Extract [region_name1 region_name2 ...]\n\n";
  out << "          Special cases: \"surfaces\", \"volumes\" as region_name.\n";
  out << "      These take precedence over real region_names."; 
  
  return out.str();
}
  
/////////////////////////////////////////////////////////////////////////////

void CMeshExtract::transform(const CMesh::Ptr& mesh, const std::vector<std::string>& args)
{

  m_mesh = mesh;

  
  // special cases "volumes" and "surfaces" as arg
  BOOST_FOREACH(const std::string region_name, args)
  {
    if (region_name == "surfaces")
    {
      Uint dimensionality = 0;
      // Find maximal dimensionality of the whole mesh
      BOOST_FOREACH( const CElements& elements, recursive_range_typed<CElements>(*m_mesh))
      {
	dimensionality = std::max(elements.element_type().dimensionality() , dimensionality);
      }
      
      // delete volume regions
      BOOST_FOREACH( CRegion& region, recursive_filtered_range_typed<CRegion>(*m_mesh,IsGroup()))
      {
        bool is_volume = false;
        BOOST_FOREACH( const CElements& elements, range_typed<CElements>(region))
        {
          if(elements.element_type().dimensionality() == dimensionality)
          {
            is_volume = true; 
            break;
          }
        }
        if (is_volume)
        {
          // delete this region
          region.get_parent()->remove_component(region.name());
        }
      }
    }
    else if (region_name == "volumes")
    {
      Uint dimensionality = 0;
      // Find maximal dimensionality of the whole mesh
      BOOST_FOREACH( const CElements& elements, recursive_range_typed<CElements>(*m_mesh))
      {
        dimensionality = std::max(elements.element_type().dimensionality() , dimensionality);
      }
      
      // delete volume regions
      BOOST_FOREACH( CRegion& region, recursive_filtered_range_typed<CRegion>(*m_mesh,IsGroup()))
      {
        bool is_volume = true;
        BOOST_FOREACH( const CElements& elements, range_typed<CElements>(region))
        {
          if(elements.element_type().dimensionality() < dimensionality)
          {
            is_volume = false; 
            break;
          }
        }
        if (!is_volume)
        {
          // delete this region
          region.get_parent()->remove_component(region.name());
        }
      }
    }    
  }
  
  
  // For every region, see if its path matches the regex, and store it in a list
  std::list<std::string> keep_region_paths;
  BOOST_FOREACH( CRegion& region, recursive_range_typed<CRegion>(*m_mesh))
  {
    // see if this region has to be deleted.
    bool found = false;
    BOOST_FOREACH(const std::string& expression, args)
    {
      if (boost::regex_match(region.full_path().string(),boost::regex(".*"+expression+".*")))
      {
        found = true;
        keep_region_paths.push_back(region.full_path().string());
        break;
      }
    }
  }
  
  // Parse the list into individual regions that will not be removed
  std::set<std::string> keep_region;
  BOOST_FOREACH(const boost::filesystem::path& region_path, keep_region_paths)
  {
    BOOST_FOREACH(const std::string& region_name, region_path)
      keep_region.insert(region_name);
  }

  // Remove regions whose name doesn't appear in the parsed list "keep_region"
  BOOST_FOREACH( CRegion& region, recursive_range_typed<CRegion>(*m_mesh))
  {
    bool found = (std::find(keep_region.begin(),keep_region.end(),region.name()) != keep_region.end());
    if (!found)  region.get_parent()->remove_component(region.name());
  }
  
  
  // remove regions that have no elements
  BOOST_FOREACH( CRegion& region, recursive_range_typed<CRegion>(*m_mesh))
  {
    if (region.recursive_elements_count() == 0)
    {
      CFinfo << "removing empty element_region " << region.full_path().string() << CFendl;
      region.get_parent()->remove_component(region.name());
    }
  }

  
  
}

//////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
