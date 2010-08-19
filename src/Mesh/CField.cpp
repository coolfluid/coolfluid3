#include "Common/ObjectProvider.hpp"

#include "Common/CLink.hpp"

#include "Mesh/MeshAPI.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CFieldElements.hpp"

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

CField& CField::create_field(const std::string& field_name, CRegion& support, const Uint dim, std::map<const CArray*,CArray*>& data_for_coordinates)
{  
  m_field_name = field_name;
  support.add_field_link(*this);
  create_component_type<CLink>("support")->link_to(support.get()); 
  if (! range_typed<CArray>(support).empty()) // if coordinates in this support_region
    data_for_coordinates[&get_component_typed<CArray>(support)] = &create_data(dim);
  
  BOOST_FOREACH(CElements& elements, range_typed<CElements>(support))
  {
    CFinfo << "creating elements " << elements.name() << CFendl;
    CFieldElements& field_elements = *create_component_type<CFieldElements>(elements.name());
    field_elements.add_tag("FieldElements");
    field_elements.initialize(elements);
    field_elements.add_node_based_storage(*data_for_coordinates[&elements.coordinates()]);
    elements.add_field_elements_link(field_elements);
  }

  BOOST_FOREACH(CRegion& support_level_down, range_typed<CRegion>(support))
  {
    CField& field = *create_component_type<CField>(support_level_down.name());
    field.create_field(m_field_name,support_level_down,dim,data_for_coordinates);
  }
  return *this;
}
  
////////////////////////////////////////////////////////////////////////////////

CField& CField::create_element_based_field(const std::string& field_name, CRegion& support)
{  
  m_field_name = field_name;
  support.add_field_link(*this);
  create_component_type<CLink>("support")->link_to(support.get()); 
  
  BOOST_FOREACH(CElements& geometry_elements, range_typed<CElements>(support))
  {
    CFinfo << "creating elements element_based" << geometry_elements.name() << CFendl;
    CFieldElements& field_elements = *create_component_type<CFieldElements>(geometry_elements.name());
    field_elements.add_tag("FieldElements");
    field_elements.initialize(geometry_elements);
    field_elements.add_element_based_storage();
    geometry_elements.add_field_elements_link(field_elements);
  }
  
  BOOST_FOREACH(CRegion& support_level_down, range_typed<CRegion>(support))
  {
    CField& field = *create_component_type<CField>(support_level_down.name());
    field.create_element_based_field(m_field_name,support_level_down);
  }
  return *this;
}

////////////////////////////////////////////////////////////////////////////////

CElements& CField::create_elements(const std::string& element_type_name, CArray& data)
{
  throw NotImplemented (FromHere(), "this function is not implemented yet");
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
  return *get_child("support")->get_type<CRegion const>();  // get() because it is a link
}
  
//////////////////////////////////////////////////////////////////////////////

CRegion& CField::support()
{
  return *get_child("support")->get_type<CRegion>();  // get() because it is a link
}
  
//////////////////////////////////////////////////////////////////////////////
  
} // Mesh
} // CF
