#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>

#include "Mesh/CRegion.hpp"

namespace CF {
namespace Mesh {

using namespace Common;

////////////////////////////////////////////////////////////////////////////////

CRegion::CRegion ( const CName& name  ) :
  Component ( name )
{
  BUILD_COMPONENT;
}

////////////////////////////////////////////////////////////////////////////////

CRegion::~CRegion()
{
}

////////////////////////////////////////////////////////////////////////////////

CRegion::Ptr CRegion::create_region( const CName& name )
{
  CRegion::Ptr new_region ( new CRegion(name) );
  m_subregions.push_back(new_region);
  add_component ( new_region );
  return get_component<CRegion>(name);
}

////////////////////////////////////////////////////////////////////////////////

CTable::Ptr CRegion::create_connectivityTable( const CName& name )
{
  CTable::Ptr new_connTable ( new CTable(name) );
  m_connTable = new_connTable;
  add_component ( m_connTable );
  return get_component<CTable>(name);
}

////////////////////////////////////////////////////////////////////////////////

CElements::Ptr CRegion::create_elementType( const CName& name )
{
  CElements::Ptr new_elementType ( new CElements(name) );
  m_elementType = new_elementType;
  add_component ( m_elementType );
  return get_component<CElements>(name);
}

//////////////////////////////////////////////////////////////////////////////

CRegion::Ptr CRegion::create_leaf_region (const std::string& etype_name )
{
  std::string region_name(etype_name);
  create_region(region_name);
  CRegion::Ptr region = get_component<CRegion>(region_name);
  region->create_connectivityTable();
  region->create_elementType();
  region->get_component<CElements>("type")->set_elementType(etype_name);
  Uint nbNodes = region->get_component<CElements>("type")->get_elementType()->getNbNodes();
  region->get_component<CTable>("table")->initialize(nbNodes);
  return region;
}

////////////////////////////////////////////////////////////////////////////////

struct CRegion::ElementNodeVector::Data : boost::noncopyable
{
  Data(const Uint iElem, const Uint nNodes, CArray& coordinatesArray, const CTable& connectivityTable) :
      elem(iElem),
      nbNodes(nNodes),
      coordinates(coordinatesArray),
      connectivity(connectivityTable)
  {}

  const Uint elem;
  const Uint nbNodes;
  CArray& coordinates;
  const CTable& connectivity;
};

////////////////////////////////////////////////////////////////////////////////

CRegion::ElementNodeVector::ElementNodeVector(const Uint iElem, const Uint nbNodes, CArray& coordinates, const CTable& connectivity) :
    m_data(new Data(iElem, nbNodes, coordinates, connectivity))
{}

////////////////////////////////////////////////////////////////////////////////

Uint CRegion::ElementNodeVector::size() const {
  return m_data->nbNodes;
}

////////////////////////////////////////////////////////////////////////////////

CArray::ConstRow CRegion::ElementNodeVector::operator[](const Uint idx) const {
  cf_assert(idx < size());
  return m_data->coordinates.get_array()[m_data->connectivity.get_table()[m_data->elem][idx]];
}

////////////////////////////////////////////////////////////////////////////////

CArray::Row CRegion::ElementNodeVector::operator[](const Uint idx) {
  cf_assert(idx < size());
  return m_data->coordinates.get_array()[m_data->connectivity.get_table()[m_data->elem][idx]];
}

////////////////////////////////////////////////////////////////////////////////

struct CRegion::ConstElementNodeVector::Data : boost::noncopyable
{
  Data(const Uint iElem, const Uint nNodes, const CArray& coordinatesArray, const CTable& connectivityTable) :
      elem(iElem),
      nbNodes(nNodes),
      coordinates(coordinatesArray),
      connectivity(connectivityTable)
  {}

  const Uint elem;
  const Uint nbNodes;
  const CArray& coordinates;
  const CTable& connectivity;
};

////////////////////////////////////////////////////////////////////////////////

CRegion::ConstElementNodeVector::ConstElementNodeVector(const Uint iElem, const Uint nbNodes, const CArray& coordinates, const CTable& connectivity) :
    m_data(new Data(iElem, nbNodes, coordinates, connectivity))
{}

////////////////////////////////////////////////////////////////////////////////

CRegion::ConstElementNodeVector::ConstElementNodeVector(const CRegion::ElementNodeVector& elementNodeVector) :
    m_data(new Data(elementNodeVector.m_data->elem, elementNodeVector.m_data->nbNodes, elementNodeVector.m_data->coordinates, elementNodeVector.m_data->connectivity))
{}

////////////////////////////////////////////////////////////////////////////////

Uint CRegion::ConstElementNodeVector::size() const {
  return m_data->nbNodes;
}

////////////////////////////////////////////////////////////////////////////////

CArray::ConstRow CRegion::ConstElementNodeVector::operator[](const Uint idx) const {
  cf_assert(idx < size());
  return m_data->coordinates.get_array()[m_data->connectivity.get_table()[m_data->elem][idx]];
}

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
