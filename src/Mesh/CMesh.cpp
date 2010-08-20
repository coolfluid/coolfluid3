#include "Common/ObjectProvider.hpp"

#include "Common/CLink.hpp"
#include "Common/ComponentPredicates.hpp"

#include "Mesh/MeshAPI.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/ElementType.hpp"

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

CRegion& CMesh::create_region( const CName& name )
{
  return *create_component_type<CRegion>(name);
}

////////////////////////////////////////////////////////////////////////////////

CField& CMesh::create_field( const CName& name , CRegion& support)
{
  CField& field = *create_component_type<CField>(name);
  field.synchronize_with_region(support);

  return field;
}


} // Mesh
} // CF
