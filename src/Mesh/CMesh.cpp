#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CArray.hpp"

namespace CF {
namespace Mesh {

using namespace Common;

////////////////////////////////////////////////////////////////////////////////

CMesh::CMesh ( const CName& name  ) :
  Component ( name )
{
}

////////////////////////////////////////////////////////////////////////////////

CMesh::~CMesh()
{
}

////////////////////////////////////////////////////////////////////////////////

void CMesh::create_region( const CName& name )
{
  boost::shared_ptr<CRegion> new_region ( new CRegion(name) );

  m_regions.push_back(new_region);

  add_component ( new_region );
}

////////////////////////////////////////////////////////////////////////////////

void CMesh::create_array( const CName& name )
{
  boost::shared_ptr<CArray<VECTOR> > new_array ( new CArray<VECTOR>(name) );

  m_arrays.push_back(new_array);

  add_component ( new_array );
}

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
