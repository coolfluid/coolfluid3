#include "Common/ObjectProvider.hpp"

#include "Mesh/MeshAPI.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CArray.hpp"

namespace CF {
namespace Mesh {

using namespace Common;

Common::ObjectProvider < CMesh, Component, MeshLib, NB_ARGS_1 >
CMesh_Provider ( CMesh::type_name() );

////////////////////////////////////////////////////////////////////////////////

CMesh::CMesh ( const CName& name  ) :
  Component ( name )
{
  BUILD_COMPONENT;
}

////////////////////////////////////////////////////////////////////////////////

CMesh::~CMesh()
{
}

////////////////////////////////////////////////////////////////////////////////

CRegion::Ptr CMesh::create_region( const CName& name )
{
  CRegion::Ptr new_region ( new CRegion(name) );

  m_regions.push_back(new_region);

  add_component ( new_region );
  return new_region;
}

////////////////////////////////////////////////////////////////////////////////

CArray::Ptr CMesh::create_array( const CName& name )
{
  CArray::Ptr new_array ( new CArray(name) );

  m_arrays.push_back(new_array);

  add_component ( new_array );
  return new_array;
}

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
