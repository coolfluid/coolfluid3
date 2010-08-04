#include "Common/Log.hpp"
#include "Common/Factory.hpp"

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

void CElements::initialize(const std::string& element_type_name, const CArray::ConstPtr coordinates)
{
  set_element_type(element_type_name);
  cf_assert(m_element_type);
  const Uint nb_nodes = m_element_type->nb_nodes();
  create_connectivity_table("connectivity_table").initialize(nb_nodes);
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
  CTable* tab = dynamic_cast<CTable*>(ptr.get());
  cf_assert(tab);
  return *tab;
}

//////////////////////////////////////////////////////////////////////////////

const CF::Mesh::CTable& CElements::connectivity_table() const
{
  Component::ConstPtr ptr = get_child("connectivity_table");
  cf_assert(ptr);
  const CTable* tab = dynamic_cast<const CTable*>(ptr.get());
  cf_assert(tab);
  return *tab;
}

//////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
