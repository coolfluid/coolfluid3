#include "Common/Log.hpp"
#include "Common/Factory.hpp"
#include "Common/CLink.hpp"
#include "Common/CGroup.hpp"

#include "Mesh/CElements.hpp"
#include "Mesh/ElementType.hpp"
#include "Mesh/GeoShape.hpp"
#include "Mesh/CField.hpp"

namespace CF {
namespace Mesh {

using namespace Common;

////////////////////////////////////////////////////////////////////////////////

CElements::CElements ( const CName& name ) :
  Component ( name )
{
  BUILD_COMPONENT;
}

////////////////////////////////////////////////////////////////////////////////

CElements::~CElements()
{
}

////////////////////////////////////////////////////////////////////////////////

void CElements::initialize(const std::string& element_type_name, CArray& nodal_data)
{
  set_element_type(element_type_name);
  cf_assert(m_element_type);
  const Uint nb_nodes = m_element_type->nb_nodes();
  create_connectivity_table("connectivity_table").initialize(nb_nodes);
  
  m_nodal_data_name = "node_" + nodal_data.name();
  create_component_type<CLink>(m_nodal_data_name)->link_to(nodal_data.get());
  
  // Create elemental data
  m_elemental_data_name = "elm_" + nodal_data.name();
  create_component_type<CArray>(m_elemental_data_name);

}
  
////////////////////////////////////////////////////////////////////////////////

void CElements::initialize_linked(CElements& element_in, CArray& nodal_data)
{
  // Set the shape function
  set_element_type(element_in.element_type().getElementTypeName());
  cf_assert(m_element_type);

  // create a link to the geometry elements.
  create_component_type<CLink>("support")->link_to(element_in.get());
  
  // create the connectivity table as a CLink to another one.
  create_component_type<CLink>(element_in.connectivity_table().name())->link_to(element_in.connectivity_table().get());

  // Set the nodal data
  m_nodal_data_name = "node_" + nodal_data.name();
  create_component_type<CLink>(m_nodal_data_name)->link_to(nodal_data.get());
  
  // Create elemental data
  m_elemental_data_name = "elm_" + nodal_data.name();
  create_component_type<CArray>(m_elemental_data_name);

}


////////////////////////////////////////////////////////////////////////////////

void CElements::set_element_type(const std::string& etype_name)
{
  Common::SafePtr< ElementType::PROVIDER > prov =
      Factory<ElementType>::instance().getProvider( etype_name );

  m_element_type = prov->create();
}

//////////////////////////////////////////////////////////////////////////////

CTable& CElements::create_connectivity_table( const CName& name )
{
  return *create_component_type<CTable>(name);
}

//////////////////////////////////////////////////////////////////////////////

CTable& CElements::connectivity_table()
{
  Component::Ptr ptr = get_child("connectivity_table");
  cf_assert(ptr);
  CTable* tab = dynamic_cast<CTable*>(ptr->get().get()); // first get() follows the CLink , second get returns a normal pointer
  cf_assert(tab);
  return *tab;
}

//////////////////////////////////////////////////////////////////////////////

const CTable& CElements::connectivity_table() const
{
  Component::ConstPtr ptr = get_child("connectivity_table");
  cf_assert(ptr);
  const CTable* tab = dynamic_cast<const CTable*>(ptr->get().get());
  cf_assert(tab);
  return *tab;
}

//////////////////////////////////////////////////////////////////////////////

CArray& CElements::nodal_data()
{
  Component::Ptr ptr = get_child(m_nodal_data_name)->get();  // get() because it is a link
  cf_assert(ptr);
  CArray* data = dynamic_cast<CArray*>(ptr.get());
  cf_assert(data);
  return *data;
}

//////////////////////////////////////////////////////////////////////////////

const CArray& CElements::nodal_data() const
{
  Component::ConstPtr ptr = get_child(m_nodal_data_name)->get();
  cf_assert(ptr);
  const CArray* data = dynamic_cast<const CArray*>(ptr.get());
  cf_assert(data);
  return *data;
}

//////////////////////////////////////////////////////////////////////////////

CArray& CElements::elemental_data()
{
  Component::Ptr ptr = get_child(m_elemental_data_name)->get();  // get() because it is a link
  cf_assert(ptr);
  CArray* data = dynamic_cast<CArray*>(ptr.get());
  cf_assert(data);
  return *data;
}

//////////////////////////////////////////////////////////////////////////////

const CArray& CElements::elemental_data() const
{
  Component::ConstPtr ptr = get_child(m_elemental_data_name)->get();
  cf_assert(ptr);
  const CArray* data = dynamic_cast<const CArray*>(ptr.get());
  cf_assert(data);
  return *data;
}

//////////////////////////////////////////////////////////////////////////////

void CElements::add_field_elements_link(CElements& field_elements)
{
  CGroup::Ptr field_group = get_child_type<CGroup>("fields");
  if (!field_group.get())
    field_group = create_component_type<CGroup>("fields");
  
  const std::string field_name = field_elements.get_parent()->get_type<CField>()->field_name();
  field_group->create_component_type<CLink>(field_name)->link_to(field_elements.get());
}

//////////////////////////////////////////////////////////////////////////////

CElements& CElements::get_field_elements(const CName& field_name)
{
  Component::Ptr all_fields = get_child("fields");
  cf_assert(all_fields.get());
  Component::Ptr field = all_fields->get_child(field_name);
  cf_assert(field.get());
  return *field->get_type<CElements>();
}
  
//////////////////////////////////////////////////////////////////////////////
  
CElements& CElements::get_geometry_elements()
{
  Component::Ptr geometry_elements = get_child("support");
  cf_assert(geometry_elements.get());
  return *geometry_elements->get_type<CElements>();
}
//////////////////////////////////////////////////////////////////////////////

  
} // Mesh
} // CF
