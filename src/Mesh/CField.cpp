#include "Common/ObjectProvider.hpp"

#include "Common/CLink.hpp"
#include "Common/ComponentPredicates.hpp"

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

CField& CField::synchronize_with_region(CRegion& support, const std::string& field_name)
{ 
  // Setup this field
  m_field_name = (field_name == "") ? name() : field_name;
  support.add_field_link(*this);
  CLink::Ptr support_link = create_component_type<CLink>("support");
  support_link->add_tag("support");
  support_link->link_to(support.get()); 
  
  // Create FieldElements if the support has them
  BOOST_FOREACH(CElements& geometry_elements, range_typed<CElements>(support))
    create_elements(geometry_elements);
  
  // Go down one level in the tree
  BOOST_FOREACH(CRegion& support_level_down, range_typed<CRegion>(support))
  {
    CField& subfield = *create_component_type<CField>(support_level_down.name());
    subfield.synchronize_with_region(support_level_down,m_field_name);
  }
  
  return *this;
}
  
////////////////////////////////////////////////////////////////////////////////

void CField::create_data_storage(const Uint dim, const DataBasis basis)
{
  switch (basis)
  {
    case ELEMENT_BASED:
      BOOST_FOREACH(CFieldElements& field_elements, recursive_range_typed<CFieldElements>(*this))
      {
        field_elements.add_element_based_storage();
        field_elements.elemental_data().array().resize(boost::extents[field_elements.elements_count()][dim]);
      }
      break;
    case NODE_BASED:
    {
      std::map<std::string,CArray*> data_for_coordinates;
      
      // Check if there are coordinates in this field, and add to map
      if (! range_typed<CArray>(support()).empty() )
      {
        CArray& coordinates = get_component_typed<CArray>(support());
        data_for_coordinates[coordinates.full_path().string()] = &create_data(dim,coordinates.size());
      }
      // Check if there are coordinates in all subfields and add to map
      BOOST_FOREACH(CField& subfield, recursive_range_typed<CField>(*this))
      {
        if (! range_typed<CArray>(subfield.support()).empty() )
        {
          CArray& coordinates = get_component_typed<CArray>(subfield.support());
          data_for_coordinates[coordinates.full_path().string()] = &subfield.create_data(dim,coordinates.size());
        }
      }

      // Add the correct data according to the map in every field elements component
      BOOST_FOREACH(CFieldElements& field_elements, recursive_range_typed<CFieldElements>(*this))
      {
        field_elements.add_node_based_storage(*data_for_coordinates[field_elements.coordinates().full_path().string()]);
      }
    }
      break;
    default:
      throw NotSupported(FromHere() , "DataBasis can only be ELEMENT_BASED or NODE_BASED");
      break;
  }
}
  
////////////////////////////////////////////////////////////////////////////////

//CField& CField::create_element_based_field(const std::string& field_name, CRegion& support)
//{  
//  m_field_name = field_name;
//  support.add_field_link(*this);
//  create_component_type<CLink>("support")->link_to(support.get()); 
//  
//  BOOST_FOREACH(CElements& geometry_elements, range_typed<CElements>(support))
//  {
//    CFinfo << "creating elements element_based" << geometry_elements.name() << CFendl;
//    CFieldElements& field_elements = *create_component_type<CFieldElements>(geometry_elements.name());
//    field_elements.add_tag("FieldElements");
//    field_elements.initialize(geometry_elements);
//    field_elements.add_element_based_storage();
//    geometry_elements.add_field_elements_link(field_elements);
//  }
//  
//  BOOST_FOREACH(CRegion& support_level_down, range_typed<CRegion>(support))
//  {
//    CField& field = *create_component_type<CField>(support_level_down.name());
//    field.create_element_based_field(m_field_name,support_level_down);
//  }
//  return *this;
//}

////////////////////////////////////////////////////////////////////////////////

CElements& CField::create_elements(CElements& geometry_elements)
{
  CFieldElements& field_elements = *create_component_type<CFieldElements>(geometry_elements.name());
  field_elements.add_tag("FieldElements");
  field_elements.initialize(geometry_elements);
  geometry_elements.add_field_elements_link(field_elements);
  return field_elements;
}

//////////////////////////////////////////////////////////////////////////////

CArray& CField::create_data(const Uint dim, const Uint nb_rows)
{
  CArray& data = *create_component_type<CArray>("data");
  data.array().resize(boost::extents[nb_rows][dim]);
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
  
////////////////////////////////////////////////////////////////////////////////

const CField& CField::subfield(const CName& name) const
{
  return get_named_component_typed<CField const>(*this,name);
}

////////////////////////////////////////////////////////////////////////////////

CField& CField::subfield(const CName& name)
{
  return get_named_component_typed<CField>(*this,name);
}
  
//////////////////////////////////////////////////////////////////////////////

const CFieldElements& CField::elements(const CName& name) const
{
  return get_named_component_typed<CFieldElements const>(*this,name);
}

////////////////////////////////////////////////////////////////////////////////

CFieldElements& CField::elements(const CName& name)
{
  return get_named_component_typed<CFieldElements>(*this,name);
}
  
//////////////////////////////////////////////////////////////////////////////
  
} // Mesh
} // CF
