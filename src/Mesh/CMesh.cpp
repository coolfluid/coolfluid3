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

CField& CMesh::create_field( const CName& name , const Uint dim, CRegion& support)
{
  CField& field = *create_component_type<CField>(name);
  std::map<CArray*,CArray*> data_for_coordinates;
  field.create_field(support,dim,data_for_coordinates);

  CFinfo << "data_for_coordinates.size() = " << data_for_coordinates.size() << CFendl;
  return field;
}
//
//
//CField& CMesh::create_field( const CName& name , const CField& other_field)
//{
//  CField::Ptr field = create_component_type<CField>(name);
//  return *field;
//}
//
//
//
//CField& CMesh::create_field_with_shapefunction( const CName& name , const CRegion& support, const ElementType& shape_function)
//{
//  CField::Ptr field = create_component_type<CField>(name);
//  return *field;
//}
//
//
//
//CField& CMesh::create_field_with_shapefunction( const CName& name , const CField& other_field, const ElementType& shape_function)
//{
//  CField::Ptr field = create_component_type<CField>(name);
//  return *field;
//}
//
//

} // Mesh
} // CF
