// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_ElementData_hpp
#define cf3_mesh_ElementData_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/EigenAssertions.hpp"

#include "math/MatrixTypes.hpp"

#include "mesh/LibMesh.hpp"
#include "common/Table.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {

////////////////////////////////////////////////////////////////////////////////

/// Fill STL-vector like per-node data storage
template<typename NodeValuesT, typename RowT>
void fill(NodeValuesT& to_fill, const common::Table<Real>& data_array, const RowT& element_row, const Uint start=0)
{
  const Uint nb_nodes = element_row.size();
  const Uint dim = data_array.row_size();
  const Uint end = start+dim;
  for(Uint node = 0; node != nb_nodes; ++node)
  {
    const common::Table<Real>::ConstRow data_row = data_array[element_row[node]];
    for(Uint j = start; j != end; ++j)
      to_fill[node][j-start] = data_row[j];
  }
}

/// Fill static sized matrices
template<typename RowT, int NbRows, int NbCols>
void fill(Eigen::Matrix<Real, NbRows, NbCols>& to_fill, const common::Table<Real>& data_array, const RowT& element_row, const Uint start=0)
{
  for(int node = 0; node != NbRows; ++node)
  {
    const common::Table<Real>::ConstRow data_row = data_array[element_row[node]];
    for(Uint j = 0; j != NbCols; ++j)
      to_fill(node, j) = data_row[j+start];
  }
}

/// Fill dynamic matrices
template<typename RowT>
void fill(RealMatrix& to_fill, const common::Table<Real>& data_array, const RowT& element_row, const Uint start=0)
{
  const Uint nb_nodes = element_row.size();
  const Uint dim = data_array.row_size();
  const Uint end = start+dim;
  for(Uint node = 0; node != nb_nodes; ++node)
  {
    const common::Table<Real>::ConstRow data_row = data_array[element_row[node]];
    for(Uint j = start; j != end; ++j)
      to_fill(node, j-start) = data_row[j];
  }
}

/// Generic nodal data consists of matrices with dimensions NbRows and NbCols
/// Commented, not really needed?
// template<Uint NbNodes, Uint NbRows, Uint NbCols=1>
// struct ElementNodeValues
// {
//   static const Uint nb_nodes  = NbNodes;
//   static const Uint nb_rows   = NbRows;
//   static const Uint nb_cols   = NbCols;
//   static const Uint node_size = NbRows*NbCols;
//
//   /// Type of value at each node
//   typedef Eigen::Matrix<Real, NbRows, NbCols> ValueT;
//
//   Uint size() const
//   {
//     return nb_nodes;
//   }
//
//   ElementNodeValues() :m_data(NbNodes) {}
//
//   const ValueT& operator[](const Uint i) const
//   {
//     return m_data[i];
//   }
//
//   template<typename RowT>
//   void fill(const common::Table<Real>& data_array, const RowT& element_row, const Uint start=0)
//   {
//     static const Uint end = start+node_size;
//     for(Uint node = 0; node != nb_nodes; ++node)
//     {
//       ValueT& mat = m_data[node];
//       const common::Table<Real>::ConstRow data_row = data_array[element_row[node]];
//       for(Uint j = 0; j != NbCols; ++j)
//       {
//         const Uint offset = start + j*NbRows;
//         for(Uint i = 0; i != NbRows; ++i)
//         {
//           mat(i, j) = data_row[offset + i];
//         }
//       }
//     }
//   }
//
// private:
//   std::vector<ValueT,Eigen::aligned_allocator<ValueT> > m_data;
// };


////////////////////////////////////////////////////////////////////////////////

/// View of nodal data, allowing modification of the referenced data
template<Uint NbNodes, Uint NbRows, Uint NbCols=1>
struct ElementNodeView;

///Specialization for single Real values
template<Uint NbNodes>
struct ElementNodeView<NbNodes, 1, 1>
{
  static const Uint nb_nodes = NbNodes;
  static const Uint nb_rows = 1;
  static const Uint nb_cols = 1;
  static const Uint node_size = 1;

  Uint size() const
  {
    return nb_nodes;
  }

  const Real& operator[](const Uint i) const
  {
    return *m_data[i];
  }

  Real& operator[](const Uint i)
  {
    return *m_data[i];
  }

  template<typename RowT>
  void fill(common::Table<Real>& data_array, const RowT& element_row, const Uint start=0)
  {
    for(Uint i = 0; i != nb_nodes; ++i)
    {
      m_data[i] = &data_array[element_row[i]][start];
    }
  }

private:
  Real* m_data[NbNodes];
};

/// Utility function to convert a vector-like type to a RealVector
template<typename RowT>
RealVector to_vector(const RowT& row)
{
  const Uint row_size = row.size();
  RealVector result(row_size);
  for(Uint i =0; i != row_size; ++i)
    result[i] = row[i];
  return result;
}

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_ElementData_hpp
