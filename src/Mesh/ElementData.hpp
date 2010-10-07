// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_ElementData_hpp
#define CF_Mesh_ElementData_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Math/RealVector.hpp"

#include "Mesh/LibMesh.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/CArray.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
  
////////////////////////////////////////////////////////////////////////////////

/// Copy of the data for all nodes of an element
struct ElementData
{
  typedef boost::multi_array<Real, 2> DataT;
  DataT data;
  
  ElementData();
  ElementData(const Uint nb_nodes, const Uint dim);
  
  /// For easy construction with boost::assign::list_of
  ElementData(const std::vector< std::vector<Real> > v);
  
  const RealVector& operator[](const Uint i) const;
  RealVector& operator[](const Uint i);
  
  const Uint size() const { return m_nb_nodes; }
  
  void resize(const Uint nb_nodes, const Uint dim);
  
  /// Fill the node lists, always making sure the data storage is resized appropriatly
  template<typename RowT>
  void resize_and_fill(const CArray& coordinates, const RowT& element_row)
  {
    const Uint nb_nodes = element_row.size();
    const Uint dim = coordinates.row_size();
    resize(nb_nodes, dim);
    
    fill(coordinates, element_row);
  }
  
  /// Fill the node list, without checking if the data size is correct
  template<typename RowT>
  void fill(const CArray& data_array, const RowT& element_row)
  {
    for(Uint i = 0; i != m_nb_nodes; ++i)
    {
      memcpy(&data[i][0], &data_array[element_row[i]][0], m_dim*sizeof(Real));
    }
  }
  
  friend Common::LogStream& operator<<(Common::LogStream& output, const ElementData& nodes);
  
private:
  std::vector<RealVector> m_data_views;
  Uint m_nb_nodes, m_dim;
};

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_ElementData_hpp
