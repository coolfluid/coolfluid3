#include <boost/foreach.hpp>

#include "Common/ObjectProvider.hpp"
#include "Common/ComponentPredicates.hpp"

#include "Mesh/CMeshExtract.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CRegion.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
  
  using namespace Common;
  
  class IsElementRegion
  {
  public:
    IsElementRegion () {}
    
    bool operator()(const Component& component)
    {
      return !range_typed<CTable>(component).empty() && !range_typed<CElements>(component).empty();
    }
    
  }; // IsElementRegion
  
  class IsGroup
  {
  public:
    IsGroup () {}
    
    bool operator()(const Component& component)
    {
      return count(filtered_range_typed<CRegion>(component,m_isElement));
    }
    
  private:
    IsElementRegion m_isElement;
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

void CMeshExtract::transform(const CMesh::Ptr& mesh, const std::vector<std::string>& args)
{

  m_mesh = mesh;

  BOOST_FOREACH(const std::string region_name, args)
  {
    // special cases:
    if (region_name == "surfaces")
    {
      Uint dimensionality = 0;
      // Find maximal dimensionality of the whole mesh
      BOOST_FOREACH( const CRegion& region, recursive_filtered_range_typed<CRegion>(*m_mesh,IsElementRegion()))
      dimensionality = std::max(region.elements_type().getDimensionality() , dimensionality);
      
      // delete volume regions
      BOOST_FOREACH( CRegion& region, recursive_filtered_range_typed<CRegion>(*m_mesh,IsGroup()))
      {
        bool is_volume = false;
        BOOST_FOREACH( const CRegion& element_region, filtered_range_typed<CRegion>(region,IsComponentTrue()))
        {
          if(element_region.elements_type().getDimensionality() == dimensionality)
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
      BOOST_FOREACH( const CRegion& region, recursive_filtered_range_typed<CRegion>(*m_mesh,IsElementRegion()))
      dimensionality = std::max(region.elements_type().getDimensionality() , dimensionality);
      
      // delete volume regions
      BOOST_FOREACH( CRegion& region, recursive_filtered_range_typed<CRegion>(*m_mesh,IsGroup()))
      {
        bool is_volume = true;
        BOOST_FOREACH( const CRegion& element_region, filtered_range_typed<CRegion>(region,IsComponentTrue()))
        {
          if(element_region.elements_type().getDimensionality() < dimensionality)
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
    else
    {
      BOOST_FOREACH( CRegion& region, recursive_filtered_range_typed<CRegion>(*m_mesh,IsGroup()))
      {
        if (region.name() != region_name)
        {
          // delete this region
          region.get_parent()->remove_component(region.name());          
        }
      }
    }

    
  }
  
}

//////////////////////////////////////////////////////////////////////////////

std::string CMeshExtract::print_region_tree(const CRegion& region, Uint level)
{
  std::string tree;
  
  for (Uint i=0; i<level; i++)
    tree += "  ";
  tree += region.name() + " (" + StringOps::to_str<Uint>(region.recursive_elements_count()) +  ")\n";
  
  BOOST_FOREACH( const CRegion& subregion, filtered_range_typed<CRegion>(region,IsComponentTrue()))
  {
    tree += print_region_tree(subregion,level+1);
  }
  return tree;    
}

//////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
