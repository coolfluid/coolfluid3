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

void CRegion::put_subregions(std::vector< boost::shared_ptr<CRegion> >& vec)
{  
  BOOST_FOREACH(boost::shared_ptr<CRegion> subregion, m_subregions)
  {
    vec.push_back(subregion);
    subregion->put_subregions(vec);
  }
}

//////////////////////////////////////////////////////////////////////////////
  
CRegion::iterator::iterator(std::vector<boost::shared_ptr<CRegion> >& vec, boost::shared_ptr<Component> parent) :  
  m_vec(vec) , 
  m_vecIt(m_vec.begin()), 
  m_region(boost::dynamic_pointer_cast<CRegion>(parent)),
  m_parent(parent)
{
  if(vec.size())
    m_region = (*m_vecIt); 
}    

//////////////////////////////////////////////////////////////////////////////

CRegion::iterator CRegion::begin()
{
  std::vector<boost::shared_ptr<CRegion> > vec;
  put_subregions(vec);
  return iterator(vec, shared_from_this());
}

//////////////////////////////////////////////////////////////////////////////

CRegion::iterator CRegion::end()
{
  std::vector<boost::shared_ptr<CRegion> > vec;
  return iterator(vec, shared_from_this());
}

//////////////////////////////////////////////////////////////////////////////

void CRegion::iterator::increment() 
{ 
  m_vecIt++;
  if (m_vecIt != m_vec.end()) {
    m_region = (*m_vecIt);
  }
  else {
    m_region = boost::dynamic_pointer_cast<CRegion>( m_parent );
  }
} 

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
