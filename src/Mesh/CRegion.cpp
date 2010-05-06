#include <boost/foreach.hpp>

#include "Mesh/CRegion.hpp"

namespace CF {
namespace Mesh {

using namespace Common;

////////////////////////////////////////////////////////////////////////////////

CRegion::CRegion ( const CName& name  ) :
  Component ( name )
{
}

////////////////////////////////////////////////////////////////////////////////

CRegion::~CRegion()
{
}

////////////////////////////////////////////////////////////////////////////////

void CRegion::create_region( const CName& name )
{
  boost::shared_ptr<CRegion> new_region ( new CRegion(name) );
  m_subregions.push_back(new_region);
  add_component ( new_region );
}

////////////////////////////////////////////////////////////////////////////////

void CRegion::create_connectivityTable( const CName& name )
{
  boost::shared_ptr<CTable> new_connTable ( new CTable(name) );
  m_connTable = new_connTable;
  add_component ( m_connTable );
}

////////////////////////////////////////////////////////////////////////////////

void CRegion::create_elementType( const CName& name )
{
  boost::shared_ptr<CElements> new_elementType ( new CElements(name) );
  m_elementType = new_elementType;
  add_component ( m_elementType );
}

//////////////////////////////////////////////////////////////////////////////

void CRegion::put_subregions(std::vector<boost::shared_ptr<CRegion> >& vec)
{  
  BOOST_FOREACH(boost::shared_ptr<CRegion> subregion, m_subregions)
  {
    vec.push_back(subregion);
    subregion->put_subregions(vec);
  }
}

//////////////////////////////////////////////////////////////////////////////

void CRegion::create_region_with_elementType ( const CName& etype_name )
{
  create_region(etype_name);
  boost::shared_ptr<CRegion> region = get_component<CRegion>(etype_name);
  region->create_connectivityTable();
  region->create_elementType();
  region->get_component<CElements>("type")->set_elementType(etype_name);
  Uint nbNodes = region->get_component<CElements>("type")->get_elementType()->getNbNodes();
  region->get_component<CTable>("table")->initialize(nbNodes);
}
////////////////////////////////////////////////////////////////////////////////

CRegion::Iterator CRegion::begin()
{
  std::vector<boost::shared_ptr<CRegion> > vec;
  put_subregions(vec);
  return Iterator(vec);
}

CRegion::Iterator CRegion::end()
{
 std::vector<boost::shared_ptr<CRegion> > vec;
 return Iterator(vec);
}

} // Mesh
} // CF
