// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_ElementData_hpp
#define CF_Mesh_ElementData_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Math/RealVector.hpp"
#include "Math/RealMatrix.hpp"

#include "Mesh/LibMesh.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/CArray.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
  
////////////////////////////////////////////////////////////////////////////////

/// Generic nodal data consists of matrices with dimensions NbRows and NbCols
/// Note: we don't consider tensors of rank higher than 2 at this time
template<Uint NbNodes, Uint NbRows, Uint NbCols=1>
struct ElementNodeValues
{
  static const Uint nb_nodes = NbNodes;
  static const Uint nb_rows = NbRows;
  static const Uint nb_cols = NbCols;
  static const Uint node_size = NbRows*NbCols;
  
  Uint size() const
  {
    return nb_nodes;
  }
  
  ElementNodeValues()
  {
    for(Uint node_idx = 0; node_idx != nb_nodes; ++node_idx)
    {
      m_views[node_idx] = RealMatrix(false, NbRows, NbCols, &m_data[node_idx*node_size]);
    }
  };
  
  const RealMatrix& operator[](const Uint i) const
  {
    return m_views[i];
  }
  
  RealMatrix& operator[](const Uint i)
  {
    return m_views[i];
  }
  
  template<typename RowT>
  void fill(const CArray& data_array, const RowT& element_row, const Uint start=0)
  {
    static const Uint end = start+node_size;
    for(Uint i = 0; i != nb_nodes; ++i)
    {
      const Uint row_offset = i*node_size;
      const CArray::ConstRow data_row = data_array[element_row[i]];
      for(Uint j = start; j != end; ++j)
        m_data[row_offset + j] = data_row[j];
    }
  }

private:
  Real m_data[NbNodes*NbRows*NbCols];
  RealMatrix m_views[NbNodes];
};

///Specialization for single Real values
template<Uint NbNodes>
struct ElementNodeValues<NbNodes, 1, 1>
{
  static const Uint nb_nodes = NbNodes;
  static const Uint nb_rows = 1;
  static const Uint nb_cols = 1;
  static const Uint node_size = 1;
  
  Uint size() const
  {
    return nb_nodes;
  }
  
  const Real operator[](const Uint i) const
  {
    return m_data[i];
  }
  
  Real& operator[](const Uint i)
  {
    return m_data[i];
  }
  
  template<typename RowT>
  void fill(const CArray& data_array, const RowT& element_row, const Uint start=0)
  {
    for(Uint i = 0; i != nb_nodes; ++i)
    {
      m_data[i] = data_array[element_row[i]][start];
    }
  }

private:
  Real m_data[NbNodes];
};

/// Specialisation for realvectors
template<Uint NbNodes, Uint NbRows>
struct ElementNodeValues<NbNodes, NbRows, 1>
{
  static const Uint nb_nodes = NbNodes;
  static const Uint nb_rows = NbRows;
  static const Uint nb_cols = 1;
  static const Uint node_size = NbRows;
  
  Uint size() const
  {
    return nb_nodes;
  }
  
  ElementNodeValues()
  {
    for(Uint node_idx = 0; node_idx != nb_nodes; ++node_idx)
    {
      m_views[node_idx] =  RealVector(nb_rows, &m_data[node_idx*nb_rows]);
    }
  };
  
  const RealVector& operator[](const Uint i) const
  {
    return m_views[i];
  }
  
  RealVector& operator[](const Uint i)
  {
    return m_views[i];
  }
  
  template<typename RowT>
  void fill(const CArray& data_array, const RowT& element_row, const Uint start=0)
  {
    static const Uint end = start+node_size;
    for(Uint i = 0; i != nb_nodes; ++i)
    {
      const Uint row_offset = i*node_size;
      const CArray::ConstRow data_row = data_array[element_row[i]];
      for(Uint j = start; j != end; ++j)
        m_data[row_offset + j] = data_row[j];
    }
  }

private:
  Real m_data[NbNodes*NbRows];
  RealVector m_views[NbNodes];
};

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
  void fill(CArray& data_array, const RowT& element_row, const Uint start=0)
  {
    for(Uint i = 0; i != nb_nodes; ++i)
    {
      m_data[i] = &data_array[element_row[i]][start];
    }
  }

private:
  Real* m_data[NbNodes];
};

/// View of nodal data. Size of tensors at nodes is runtime-modifiable
template<Uint NbNodes>
struct ElementNodeTensorViewD
{
  ElementNodeTensorViewD()
  {
    for(Uint i = 0; i != NbNodes; ++i)
    {
      m_data[i] = 0;
    }
  }
  
  ~ElementNodeTensorViewD()
  {
    clear();
  }
  
  void clear()
  {
    for(Uint i = 0; i != NbNodes; ++i)
    {
      delete m_data[i];
      m_data[i] = 0;
    }
  }
  
  Uint size() const
  {
    return NbNodes;
  }
  
  void init(const Uint nb_rows, const Uint nb_cols)
  {
    clear();
    m_nb_rows = nb_rows;
    m_nb_cols = nb_cols;
  }
  
  const RealMatrix& operator[](const Uint i) const
  {
    return *m_data[i];
  }
  
  RealMatrix& operator[](const Uint i)
  {
    return *m_data[i];
  }
  
  template<typename RowT>
  void fill(CArray& data_array, const RowT& element_row, const Uint start=0)
  {
    for(Uint i = 0; i != NbNodes; ++i)
    {
      m_data[i] = new RealMatrix(false, m_nb_rows, m_nb_cols, &data_array[element_row[i]][start]);
    }
  }

private:
  RealMatrix* m_data[NbNodes];
  Uint m_nb_rows;
  Uint m_nb_cols;
};

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_ElementData_hpp
