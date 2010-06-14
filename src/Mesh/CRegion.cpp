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
  BUILD_COMPONENT;
}

////////////////////////////////////////////////////////////////////////////////

CRegion::~CRegion()
{
//  BOOST_FOREACH(CRegion::Ptr subregion, m_subregions)
//    subregion.reset();
//  m_elementType.reset();
//  m_connTable.reset();
}

////////////////////////////////////////////////////////////////////////////////

CRegion::Ptr CRegion::create_region( const CName& name )
{
//  CRegion::Ptr new_region ( new CRegion(name) );
//  m_subregions.push_back(new_region);
//  add_component ( new_region );
//  return new_region;
  return create_component_type<CRegion>(name);
}

////////////////////////////////////////////////////////////////////////////////

CTable::Ptr CRegion::create_connectivityTable( const CName& name )
{
//  CTable::Ptr new_connTable ( new CTable(name) );
//  m_connTable = new_connTable;
//  add_component ( m_connTable );
//  return new_connTable;
  return create_component_type<CTable>(name);
}

////////////////////////////////////////////////////////////////////////////////

CElements::Ptr CRegion::create_elementType( const CName& name )
{
//  CElements::Ptr new_elementType ( new CElements(name) );
//  m_elementType = new_elementType;
//  add_component ( m_elementType );
//  return new_elementType;
  return create_component_type<CElements>(name);
}

//////////////////////////////////////////////////////////////////////////////

CRegion::Ptr CRegion::create_leaf_region (const std::string& etype_name )
{
  std::string region_name(etype_name);
  CRegion::Ptr region = create_region(region_name);
  CElements& elements = *region->create_elementType();
  elements.set_elementType(etype_name);
  Uint nbNodes = elements.get_elementType()->getNbNodes();
  region->create_connectivityTable()->initialize(nbNodes);
  return region;
}

//////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
