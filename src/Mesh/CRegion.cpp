#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>

#include "Mesh/CRegion.hpp"

namespace CF {
namespace Mesh {

using namespace Common;

////////////////////////////////////////////////////////////////////////////////

CRegion::CRegion ( const CName& name  ) :
  Component ( name )
{
  build_component(this);
}

////////////////////////////////////////////////////////////////////////////////

CRegion::~CRegion()
{
}

////////////////////////////////////////////////////////////////////////////////

CRegion::Ptr CRegion::create_region( const CName& name )
{
  CRegion::Ptr new_region ( new CRegion(name) );
  m_subregions.push_back(new_region);
  add_component ( new_region );
  return get_component<CRegion>(name);
}

////////////////////////////////////////////////////////////////////////////////

CTable::Ptr CRegion::create_connectivityTable( const CName& name )
{
  CTable::Ptr new_connTable ( new CTable(name) );
  m_connTable = new_connTable;
  add_component ( m_connTable );
  return get_component<CTable>(name);
}

////////////////////////////////////////////////////////////////////////////////

CElements::Ptr CRegion::create_elementType( const CName& name )
{
  CElements::Ptr new_elementType ( new CElements(name) );
  m_elementType = new_elementType;
  add_component ( m_elementType );
  return get_component<CElements>(name);
}

//////////////////////////////////////////////////////////////////////////////

void CRegion::filter_subregions(std::vector<CRegion::Ptr >& vec)
{
  BOOST_FOREACH(CRegion::Ptr subregion, get_components_by_tag<CRegion>(getClassName()))
  {
    vec.push_back(subregion);
    subregion->filter_subregions(vec);
  }
}

//////////////////////////////////////////////////////////////////////////////

CRegion::Ptr CRegion::create_leaf_region (const std::string& etype_name )
{
  std::string region_name(etype_name);
  create_region(region_name);
  CRegion::Ptr region = get_component<CRegion>(region_name);
  region->create_connectivityTable();
  region->create_elementType();
  region->get_component<CElements>("type")->set_elementType(etype_name);
  Uint nbNodes = region->get_component<CElements>("type")->get_elementType()->getNbNodes();
  region->get_component<CTable>("table")->initialize(nbNodes);
  return region;
}

////////////////////////////////////////////////////////////////////////////////

CRegion::Iterator CRegion::begin()
{
  std::vector<CRegion::Ptr > vec;
  filter_subregions(vec);
  return Iterator(vec);
}

CRegion::Iterator CRegion::end()
{
 std::vector<CRegion::Ptr > vec;
 return Iterator(vec);
}

} // Mesh
} // CF
