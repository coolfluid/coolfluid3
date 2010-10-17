// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_ElementNodes_hpp
#define CF_Mesh_ElementNodes_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Math/RealVector.hpp"

#include "Mesh/LibMesh.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/CArray.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

////////////////////////////////////////////////////////////////////////////////

/// Provide a constant view of the nodes of a single element, offering operator[] and size() functions
/// Copying this creates a shallow copy.
struct Mesh_API ConstElementNodeView
{
  ConstElementNodeView() {}
  ConstElementNodeView(CArray const& coordinates, const CTable::ConstRow& connectivity);
  Uint size() const;
  CArray::ConstRow operator[](const Uint idx) const;
  CArray::Row operator[](const Uint idx);
private:
  struct Data;
  boost::shared_ptr<Data> m_data;
};

////////////////////////////////////////////////////////////////////////////////

typedef std::vector<RealVector> ElementNodeVector;

/// Copy of the node coordinates
class Mesh_API ElementNodes
{
public:
  ElementNodes();
  ElementNodes(const Uint nb_nodes, const Uint dim);
  
  /// For easy construction with boost::assign::list_of
  ElementNodes(const std::vector< std::vector<Real> > v);
  
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
  void fill(const CArray& coordinates, const RowT& element_row)
  {
    for(Uint i = 0; i != m_nb_nodes; ++i)
    {
      memcpy(&m_data[i*m_dim], &coordinates[element_row[i]][0], m_dim*sizeof(Real));
    }
  }
  
  friend Mesh_API Common::LogStream& operator<<(Common::LogStream& output, const ElementNodes& nodes);
  
private:
  std::vector<Real> m_data;
  std::vector<RealVector> m_data_views;
  Uint m_nb_nodes, m_dim;
};

////////////////////////////////////////////////////////////////////////////////

/// Starting at the given iterator, fill its sequence with the node coordinates
/// ListT must support operator[], resize(size, value) and ::value_type
template<typename ListT, typename RowT>
inline void fill_node_list(ListT& result, const CArray& coordinates, const RowT& element_row)
{
  const Uint nb_nodes = element_row.size();
  typedef typename ListT::value_type CoordT;
  const Uint dim = coordinates.row_size();
  result.resize(nb_nodes, CoordT(dim));
  for(Uint i = 0; i != nb_nodes; ++i)
  {
    const CArray::ConstRow from_coords = coordinates[element_row[i]];
    CoordT& to_coords = result[i];
    for(Uint j = 0; j != dim; ++j)
      to_coords[j] = from_coords[j];
  }
}


////////////////////////////////////////////////////////////////////////////////

/// Convenience function to create a 1D point
inline RealVector point1(const Real x)
{
  RealVector result(1);
  result[0] = x;
  return result;
}

/// Convenience function to create a 2D point
inline RealVector point2(const Real x, const Real y)
{
  RealVector result(2);
  result[0] = x;
  result[1] = y;
  return result;
}

/// Convenience function to create a 3D point
inline RealVector point3(const Real x, const Real y, const Real z)
{
  RealVector result(3);
  result[0] = x;
  result[1] = y;
  result[2] = z;
  return result;
}

////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& output, const ConstElementNodeView& nodeVector);
Mesh_API Common::LogStream& operator<<(Common::LogStream& output, const ElementNodes& nodes);

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_ElementNodes_hpp
