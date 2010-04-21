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

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
