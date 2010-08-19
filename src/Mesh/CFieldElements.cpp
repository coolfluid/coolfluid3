#include "Common/Log.hpp"
#include "Common/Factory.hpp"
#include "Common/CLink.hpp"
#include "Common/CGroup.hpp"

#include "Mesh/CFieldElements.hpp"
#include "Mesh/ElementType.hpp"
#include "Mesh/GeoShape.hpp"
#include "Mesh/CField.hpp"

namespace CF {
namespace Mesh {

using namespace Common;

////////////////////////////////////////////////////////////////////////////////

CFieldElements::CFieldElements ( const CName& name ) :
  CElements (name)
{
  BUILD_COMPONENT;
}

////////////////////////////////////////////////////////////////////////////////

CFieldElements::~CFieldElements()
{
}

////////////////////////////////////////////////////////////////////////////////

void CFieldElements::initialize_node_based(CElements& elements, CArray& nodal_data)
{
  // Set the shape function
  set_element_type(elements.element_type().getElementTypeName());
  cf_assert(m_element_type);

  // create a link to the geometry elements.
  CLink::Ptr support = create_component_type<CLink>("support");
  support->link_to(elements.get());
  
  // create the connectivity table as a CLink to another one.
  CLink::Ptr connectivity_table = create_component_type<CLink>(elements.connectivity_table().name());
  connectivity_table->link_to(elements.connectivity_table().get());
  
  // Set the nodal data
  m_nodal_data_name = "node_data";
  CLink::Ptr node_data = create_component_type<CLink>(m_nodal_data_name);
  node_data->add_tag(m_nodal_data_name);
  node_data->link_to(nodal_data.get());
}

////////////////////////////////////////////////////////////////////////////////

void CFieldElements::initialize_element_based(CElements& elements)
{
  // Set the shape function
  set_element_type(elements.element_type().getElementTypeName());
  cf_assert(m_element_type);
  
  // create a link to the geometry elements.
  CLink::Ptr support = create_component_type<CLink>("support");
  support->add_tag("support");
  support->link_to(elements.get());
  
  // create the connectivity table as a CLink to another one.
  CLink::Ptr connectivity_table = create_component_type<CLink>(elements.connectivity_table().name());
  connectivity_table->link_to(elements.connectivity_table().get());

  // Create elemental data
  m_elemental_data_name = "element_data";
  CArray::Ptr elm_data = create_component_type<CArray>(m_elemental_data_name);
  elm_data->add_tag(m_elemental_data_name);
}

//////////////////////////////////////////////////////////////////////////////

CArray& CFieldElements::nodal_data()
{
  Component& nod_data = get_component(*this,IsComponentTag(m_nodal_data_name));
  return *nod_data.get_type<CArray>();
}

//////////////////////////////////////////////////////////////////////////////

const CArray& CFieldElements::nodal_data() const
{
  const Component& nod_data = get_component(*this,IsComponentTag(m_nodal_data_name));
  return *nod_data.get_type<CArray const>();
}

//////////////////////////////////////////////////////////////////////////////

CArray& CFieldElements::elemental_data()
{
  return get_component_typed<CArray>(*this,IsComponentTag(m_elemental_data_name));
}

//////////////////////////////////////////////////////////////////////////////

const CArray& CFieldElements::elemental_data() const
{
  return get_component_typed<CArray const>(*this,IsComponentTag(m_elemental_data_name));
}
  
//////////////////////////////////////////////////////////////////////////////
  
CElements& CFieldElements::get_geometry_elements()
{
  Component& geometry_elements = get_component(*this,IsComponentTag("support"));
  return *geometry_elements.get_type<CElements>();
}
  
//////////////////////////////////////////////////////////////////////////////

const CElements& CFieldElements::get_geometry_elements() const
{
  const Component& geometry_elements = get_component(*this,IsComponentTag("support"));
  return *geometry_elements.get_type<CElements const>();
}
    
//////////////////////////////////////////////////////////////////////////////

  
} // Mesh
} // CF
