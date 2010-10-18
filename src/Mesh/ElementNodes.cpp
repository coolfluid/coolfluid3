// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/foreach.hpp>

#include "Common/StreamHelpers.hpp"

#include "Mesh/ElementNodes.hpp"


namespace CF {
namespace Mesh {

using namespace Common;

////////////////////////////////////////////////////////////////////////////////

namespace detail {

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

}

////////////////////////////////////////////////////////////////////////////////

struct ConstElementNodeView::Data : public detail::ElementData<CArray const>
{
  Data(CArray const& coordinates, const CTable::ConstRow& connectivity) : detail::ElementData<CArray const>(coordinates, connectivity) {}
};

////////////////////////////////////////////////////////////////////////////////

ConstElementNodeView::ConstElementNodeView(CArray const& coordinates, const CTable::ConstRow& connectivity) :
    m_data(new Data(coordinates, connectivity))
{}

////////////////////////////////////////////////////////////////////////////////

Uint ConstElementNodeView::size() const { return m_data->connectivity.size(); }

////////////////////////////////////////////////////////////////////////////////

CArray::ConstRow ConstElementNodeView::operator[](const Uint idx) const
{
  cf_assert(idx < size());
  return m_data->coordinates[m_data->connectivity[idx]];
}

////////////////////////////////////////////////////////////////////////////////

ElementNodes::ElementNodes() : m_nb_nodes(0), m_dim(0)
{
}

ElementNodes::ElementNodes(const CF::Uint nb_nodes, const CF::Uint dim) : m_nb_nodes(0), m_dim(0)
{
  resize(nb_nodes, dim);
}

ElementNodes::ElementNodes ( const std::vector< std::vector< Real > > v )  : m_nb_nodes(0), m_dim(0)
{
  if(v.size())
  {
    resize(v.size(), v.front().size());
    for(Uint node_idx = 0; node_idx != m_nb_nodes; ++node_idx)
    {
      cf_assert(v[node_idx].size() == m_dim);
      for(Uint i = 0; i != m_dim; ++i)
      {
        m_data[node_idx*m_dim + i] = v[node_idx][i];
      }
    }
  }
}

const CF::RealVector& ElementNodes::operator[](const CF::Uint i) const
{
  return m_data_views[i];
}

RealVector& ElementNodes::operator[](const CF::Uint i)
{
  return m_data_views[i];
}

void ElementNodes::resize(const CF::Uint nb_nodes, const CF::Uint dim)
{
  if(dim == m_dim && nb_nodes == m_nb_nodes)
    return;
  
  m_data.resize(nb_nodes*dim);
  m_data_views.resize(nb_nodes);
  
  if(dim == m_dim)
  {
    if(nb_nodes > m_nb_nodes)
    {
      for(Uint node = m_nb_nodes; node != nb_nodes; ++node)
        m_data_views[node] = RealVector(dim, &m_data[node*dim]);
    }
  }
  else
  {
    for(Uint node = 0; node != nb_nodes; ++node)
        m_data_views[node] = RealVector(dim, &m_data[node*dim]);
  }
  
  m_nb_nodes = nb_nodes;
  m_dim = dim;
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

Common::LogStream& operator<<(Common::LogStream& output, const ElementNodes& nodes)
{
  print_vector(output, nodes.m_data_views, " ; ");
  return output;
}


////////////////////////////////////////////////////////////////////////////////


} // Mesh
} // CF
