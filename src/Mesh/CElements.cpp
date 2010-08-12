#include "Common/Log.hpp"
#include "Common/Factory.hpp"
#include "Common/CLink.hpp"

#include "Mesh/CElements.hpp"
#include "Mesh/ElementType.hpp"
#include "Mesh/GeoShape.hpp"

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

void CElements::initialize(const std::string& element_type_name, CArray::Ptr data)
{
  set_element_type(element_type_name);
  cf_assert(m_element_type);
  const Uint nb_nodes = m_element_type->nb_nodes();
  create_connectivity_table("connectivity_table").initialize(nb_nodes);
  m_data_name = data->name();
  CLink::Ptr data_link = create_component_type<CLink>(m_data_name);
  data_link->link_to(data);
}
  
////////////////////////////////////////////////////////////////////////////////

void CElements::initialize_linked(CElements& element_in, CArray& data)
{
  set_element_type(element_in.element_type().getElementTypeName());
  cf_assert(m_element_type);
  m_data_name = data.name();
  create_component_type<CLink>(m_data_name)->link_to(data.get());
  
  element_in.connectivity_table().name();
  create_component_type<CLink>(element_in.connectivity_table().name())->link_to(element_in.connectivity_table().get());
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

CArray& CElements::data()
{
  Component::Ptr ptr = get_child(m_data_name)->get();  // get() because it is a link
  cf_assert(ptr);
  CArray* data = dynamic_cast<CArray*>(ptr.get());
  cf_assert(data);
  return *data;
}

//////////////////////////////////////////////////////////////////////////////

const CArray& CElements::data() const
{
  Component::ConstPtr ptr = get_child(m_data_name)->get();
  cf_assert(ptr);
  const CArray* data = dynamic_cast<const CArray*>(ptr.get());
  cf_assert(data);
  return *data;
}

//////////////////////////////////////////////////////////////////////////////
  
} // Mesh
} // CF
