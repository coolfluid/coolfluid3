#include "Mesh/CRegion.hpp"

namespace CF {
namespace Mesh {

using namespace Common;

////////////////////////////////////////////////////////////////////////////////

CRegion::CRegion ( const CName& name  ) :
  Component ( name ),
  m_isLowestLevelRegion ( true )
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
  m_isLowestLevelRegion = false;
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

void CRegion_iterator::first() 
{
  m_vec.resize(0);
  fillVector(m_root);
  m_it = m_vec.begin();
  increment();
}

//////////////////////////////////////////////////////////////////////////////

bool CRegion_iterator::isDone()
// returns true when the traversal is completed
{
  return m_region==NULL;
}

//////////////////////////////////////////////////////////////////////////////

CRegion_iterator CRegion_iterator::end()
{
  return CRegion_iterator();
}

//////////////////////////////////////////////////////////////////////////////

void CRegion_iterator::fillVector(CRegion* p) {
  m_vec.push_back(p);
  const Uint nb_subregions = p->get_subregions().size();
  for(Uint i = 0; i<nb_subregions; i++ )
  { 
    fillVector(p->get_subregions()[i].get());
  }
}

void CRegion_iterator::increment() 
{ 
  m_it++;
  if (m_it != m_vec.end()) {
    m_region = *m_it;
  }
  else {
    m_region = NULL;
  }
} 

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
