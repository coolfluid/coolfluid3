#include <boost/foreach.hpp>

#include "Mesh/ElementNodes.hpp"

namespace CF {
namespace Mesh {

using namespace Common;

////////////////////////////////////////////////////////////////////////////////

template<typename CoordinatesT>
struct ElementData : boost::noncopyable
{
  ElementData(CoordinatesT& coordinatesArray, const CTable::ConstRow& elementConnectivity) :
      coordinates(coordinatesArray),
      connectivity(elementConnectivity)
  {}

  CoordinatesT& coordinates;
  const CTable::ConstRow connectivity;
};

////////////////////////////////////////////////////////////////////////////////

struct ElementNodeView::Data : public ElementData<CArray>
{
  Data(CArray& coordinates, const CTable::ConstRow& connectivity) : ElementData<CArray>(coordinates, connectivity) {}
};

////////////////////////////////////////////////////////////////////////////////

ElementNodeView::ElementNodeView(CArray& coordinates, const CTable::ConstRow& connectivity) :
    m_data(new Data(coordinates, connectivity))
{}

////////////////////////////////////////////////////////////////////////////////

Uint ElementNodeView::size() const {
  return m_data->connectivity.size();
}

////////////////////////////////////////////////////////////////////////////////

CArray::ConstRow ElementNodeView::operator[](const Uint idx) const {
  cf_assert(idx < size());
  cf_assert(m_data->connectivity[idx] < m_data->coordinates.size());
  return m_data->coordinates[m_data->connectivity[idx]];
}

////////////////////////////////////////////////////////////////////////////////

CArray::Row ElementNodeView::operator[](const Uint idx) {
  cf_assert(idx < size());
  cf_assert(m_data->connectivity[idx] < m_data->coordinates.size());
  return m_data->coordinates[m_data->connectivity[idx]];
}

////////////////////////////////////////////////////////////////////////////////


struct ConstElementNodeView::Data : public ElementData<CArray const>
{
  Data(CArray const& coordinates, const CTable::ConstRow& connectivity) : ElementData<CArray const>(coordinates, connectivity) {}
};

////////////////////////////////////////////////////////////////////////////////

ConstElementNodeView::ConstElementNodeView(CArray const& coordinates, const CTable::ConstRow& connectivity) :
    m_data(new Data(coordinates, connectivity))
{}

////////////////////////////////////////////////////////////////////////////////

ConstElementNodeView::ConstElementNodeView(const ElementNodeView& elementNodeVector) :
    m_data(new Data(elementNodeVector.m_data->coordinates, elementNodeVector.m_data->connectivity))
{}

////////////////////////////////////////////////////////////////////////////////

Uint ConstElementNodeView::size() const {
  return m_data->connectivity.size();
}

////////////////////////////////////////////////////////////////////////////////

CArray::ConstRow ConstElementNodeView::operator[](const Uint idx) const {
  cf_assert(idx < size());
  return m_data->coordinates[m_data->connectivity[idx]];
}

////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& output, const ElementNodeView& nodeVector)
{
  const Uint num_nodes = nodeVector.size();
  for(Uint node = 0; node != num_nodes; ++node) {
    output << "( ";
    BOOST_FOREACH(Real coordinate, nodeVector[node]) {
      output << coordinate << " ";
    }
    output << ") ";
  }
  return output;
}

std::ostream& operator<<(std::ostream& output, const ConstElementNodeView& nodeVector)
{
  const Uint num_nodes = nodeVector.size();
  for(Uint node = 0; node != num_nodes; ++node) {
    output << "( ";
    BOOST_FOREACH(Real coordinate, nodeVector[node]) {
      output << coordinate << " ";
    }
    output << ") ";
  }
  return output;
}

////////////////////////////////////////////////////////////////////////////////


} // Mesh
} // CF
