// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/foreach.hpp>

#include "Common/StreamHelpers.hpp"

#include "Mesh/ElementData.hpp"


namespace CF {
namespace Mesh {

using namespace Common;

ElementData::ElementData() : m_nb_nodes(0), m_dim(0)
{
}

ElementData::ElementData(const CF::Uint nb_nodes, const CF::Uint dim) : m_nb_nodes(0), m_dim(0)
{
  resize(nb_nodes, dim);
}

ElementData::ElementData ( const std::vector< std::vector< Real > > v )  : m_nb_nodes(0), m_dim(0)
{
  if(v.size())
  {
    resize(v.size(), v.front().size());
    for(Uint node_idx = 0; node_idx != m_nb_nodes; ++node_idx)
    {
      cf_assert(v[node_idx].size() == m_dim);
      for(Uint i = 0; i != m_dim; ++i)
      {
        data[node_idx][i] = v[node_idx][i];
      }
    }
  }
}

const CF::RealVector& ElementData::operator[](const CF::Uint i) const
{
  return m_data_views[i];
}

RealVector& ElementData::operator[](const CF::Uint i)
{
  return m_data_views[i];
}

void ElementData::resize(const CF::Uint nb_nodes, const CF::Uint dim)
{
  if(dim == m_dim && nb_nodes == m_nb_nodes)
    return;
  
  data.resize(boost::extents[nb_nodes][dim]);
  m_data_views.resize(nb_nodes);
  
  if(dim == m_dim)
  {
    if(nb_nodes > m_nb_nodes)
    {
      for(Uint node = m_nb_nodes; node != nb_nodes; ++node)
        m_data_views[node] = RealVector(dim, &data[node][dim]);
    }
  }
  else
  {
    for(Uint node = 0; node != nb_nodes; ++node)
      m_data_views[node] = RealVector(dim, &data[node][dim]);
  }
  
  m_nb_nodes = nb_nodes;
  m_dim = dim;
}

Common::LogStream& operator<<(Common::LogStream& output, const ElementData& nodes)
{
  print_vector(output, nodes.m_data_views, " ; ");
  return output;
}

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
