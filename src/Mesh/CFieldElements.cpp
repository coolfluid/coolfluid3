#include "Common/CLink.hpp"
#include "Common/ComponentPredicates.hpp"

#include "Mesh/CFieldElements.hpp"
#include "Mesh/CArray.hpp"
#include "Mesh/CTable.hpp"

namespace CF {
namespace Mesh {

using namespace Common;

////////////////////////////////////////////////////////////////////////////////

CFieldElements::CFieldElements ( const CName& name ) :
  CElements (name)
{
  BUILD_COMPONENT;
  properties()["element_based"]=false;
  properties()["node_based"]=false;
}

////////////////////////////////////////////////////////////////////////////////

CFieldElements::~CFieldElements()
{
}

////////////////////////////////////////////////////////////////////////////////

void CFieldElements::initialize(CElements& elements)
{
  // Set the shape function
  set_element_type(elements.element_type().getElementTypeName());
  cf_assert(m_element_type);

  // create a link to the geometry elements.
  CLink::Ptr support = create_component_type<CLink>("support");
  support->link_to(elements.get());
  support->add_tag("support");
  
  // create the connectivity table as a CLink to another one.
  CLink::Ptr connectivity_table = create_component_type<CLink>(elements.connectivity_table().name());
  connectivity_table->link_to(elements.connectivity_table().get());
  
}

////////////////////////////////////////////////////////////////////////////////
  
void CFieldElements::add_node_based_storage(CArray& nodal_data)
{
  // Set the nodal data
  m_data_name = "node_data";
  CLink::Ptr node_data = create_component_type<CLink>(m_data_name);
  node_data->add_tag(m_data_name);
  node_data->link_to(nodal_data.get());
  properties()["node_based"]=true;
}

////////////////////////////////////////////////////////////////////////////////

void CFieldElements::add_element_based_storage()
{
  // Create elemental data
  m_data_name = "element_data";
  CArray::Ptr elm_data = create_component_type<CArray>(m_data_name);
  elm_data->add_tag(m_data_name);
  properties()["element_based"]=true;
}
  
//////////////////////////////////////////////////////////////////////////////

CArray& CFieldElements::data()
{
  Component& data = get_component(*this,IsComponentTag(m_data_name));
  return *data.get_type<CArray>();
}

//////////////////////////////////////////////////////////////////////////////

const CArray& CFieldElements::data() const
{
  const Component& data = get_component(*this,IsComponentTag(m_data_name));
  return *data.get_type<CArray const>();
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
