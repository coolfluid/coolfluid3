#include "Common/ObjectProvider.hpp"

#include "Common/CLink.hpp"

#include "Mesh/MeshAPI.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CRegion.hpp"

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

CField& CField::create_field(CRegion& support, const Uint dim, std::map<CArray*,CArray*>& data_for_coordinates)
{  
  create_component_type<CLink>("support")->link_to(support.get()); 
  if (! range_typed<CArray>(support).empty()) // if coordinates in this support_region
    data_for_coordinates[&get_component_typed<CArray>(support)] = &create_data(dim);
  
  BOOST_FOREACH(CElements& elements, range_typed<CElements>(support))
  {
    CFinfo << "creating elements " << elements.name() << CFendl;
    CArray* coordinates = &elements.coordinates();
    CArray& data = *data_for_coordinates[coordinates];

    CElements& field_elements = *create_component_type<CElements>(elements.name());
    field_elements.initialize_linked(elements,data);
  }

  BOOST_FOREACH(CRegion& support_level_down, range_typed<CRegion>(support))
  {
    CField& field = *create_component_type<CField>(support_level_down.name());
    field.create_field(support_level_down,dim,data_for_coordinates);
  }
  return *this;
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
  
  elements->initialize_linked(*elements,*data);
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

const CRegion& CField::support() const
{
  return *boost::dynamic_pointer_cast<CRegion const>(get_child("support")->get());  // get() because it is a link
}
  
//////////////////////////////////////////////////////////////////////////////
  
} // Mesh
} // CF
