#include <boost/foreach.hpp>

#include "Common/ObjectProvider.hpp"
#include "Common/ComponentPredicates.hpp"

#include "Mesh/CMeshInfo.hpp"
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

Common::ObjectProvider < Mesh::CMeshInfo,
                         Mesh::CMeshTransformer,
                         Mesh::MeshLib,
                         1 >
CMeshInfo_Provider ( "Info" );

//////////////////////////////////////////////////////////////////////////////

CMeshInfo::CMeshInfo( const CName& name )
: CMeshTransformer(name)
{
  BUILD_COMPONENT;
}

/////////////////////////////////////////////////////////////////////////////

void CMeshInfo::transform(const CMesh::Ptr& mesh, const std::vector<std::string>& args)
{

  m_mesh = mesh;

  CFinfo << "Element distribution:" << CFendl;
  CFinfo << "---------------------" << CFendl;
  BOOST_FOREACH( const CRegion& region, filtered_range_typed<CRegion>(*m_mesh,IsComponentTrue()))
  {
    CFinfo << print_region_tree(region) << CFflush;
  }  
  
}

//////////////////////////////////////////////////////////////////////////////

std::string CMeshInfo::print_region_tree(const CRegion& region, Uint level)
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
