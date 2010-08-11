#include "Common/ObjectProvider.hpp"

#include "Mesh/MeshAPI.hpp"
#include "Mesh/CField.hpp"

namespace CF {
namespace Mesh {

using namespace Common;

Common::ObjectProvider < CField, Component, MeshLib, NB_ARGS_1 >
CField_Provider ( CField::type_name() );

////////////////////////////////////////////////////////////////////////////////

CField::CField ( const CName& name  ) :
  Component ( name )
{
  BUILD_COMPONENT;
}

////////////////////////////////////////////////////////////////////////////////

CField::~CField()
{
}

////////////////////////////////////////////////////////////////////////////////

CField& CField::create_field( const CName& name )
{
  return *create_component_type<CField>(name);
}

////////////////////////////////////////////////////////////////////////////////

CElements& CField::create_elements(const std::string& element_type_name, CArray::Ptr data)
{
  std::string name = "elements_" + element_type_name;
  CElements::Ptr elements = create_component_type<CElements>(name);
  
  // if no argument is passed for coordinates, take the coordinates defined in this field.
  // throws error if not found.
  if (!data.get())
  {
    data = get_child_type<CArray>("data");
    if (!data.get())
      throw ValueNotFound(FromHere(), "Component with name \"data\" does not exist in component with path [" 
                          + this->full_path().string() + "]");
  }
  
  elements->initialize(element_type_name,data);
  return *elements;
}

//////////////////////////////////////////////////////////////////////////////

CArray& CField::create_data(const Uint& dim)
{
  CArray& data = *create_component_type<CArray>("data");
  data.initialize(dim);
  return data;
}

//////////////////////////////////////////////////////////////////////////////
  
} // Mesh
} // CF
