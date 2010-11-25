// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CTable_hpp
#define CF_Mesh_CTable_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"

#include "Mesh/ArrayBufferT.hpp"
#include "Mesh/LibMesh.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

////////////////////////////////////////////////////////////////////////////////

/// Component holding a connectivity table
/// The table has to be filled through a buffer.
/// Before using the table one has to be sure that
/// the buffer is flushed.
/// @author Willem Deconinck Tiago Quintino

template<typename ValueT>
class CTable : public Common::Component
{
public: // typedefs
  typedef ValueT value_type;
  typedef boost::multi_array<ValueT,2> ArrayT;
  typedef typename boost::subarray_gen<ArrayT,1>::type Row;
  typedef const typename boost::const_subarray_gen<ArrayT,1>::type ConstRow;
  typedef ArrayBufferT<ValueT> Buffer;

  typedef boost::shared_ptr<CTable> Ptr;
  typedef boost::shared_ptr<CTable const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CTable ( const std::string& name )  : Component ( name )
  {
    add_tag( type_name() );
  }

  /// Get the class name
  static std::string type_name () { return "CTable<"+class_name<ValueT>()+">"; }
  
  /// Initialize the array with a fixed column size and remove all existing rows, if any
  void initialize(const Uint nb_cols)
  {
    m_array.resize(boost::extents[0][nb_cols]); 
  }

  /// Resize the array to the given number of rows
  void resize(const Uint nb_rows)
  {
    m_array.resize(boost::extents[nb_rows][row_size()]);
  }

  /// @return A reference to the array data
  ArrayT& array() { return m_array; }

  /// @return A const reference to the array data
  const ArrayT& array() const { return m_array; }

  /// @return A Buffer object that can fill this Array
  Buffer create_buffer(const size_t buffersize=16384)
  {
    // make sure the array has its columnsize defined
    cf_assert(row_size() > 0);
    return Buffer(m_array,buffersize); 
  }

  /// @return A mutable row of the underlying array
  Row operator[](const Uint idx) { return m_array[idx]; }

  /// @return A const row of the underlying array
  ConstRow operator[](const Uint idx) const { return m_array[idx]; }

  /// @return The number of local rows in the array
  Uint size() const { return m_array.size(); }

	/// @return The number of elements in each row, i.e. the number of columns of the array
  Uint row_size() const { return m_array.shape()[1]; }

  /// copy a given row into the array
  /// @param [in] array_idx the index of the row that will be set
  /// @param [in] row       the row that will be copied into the array
  template<typename VectorT>
  void set_row(const Uint array_idx, const VectorT& row)
  {
    cf_assert(row.size() == row_size());

    Row row_to_set = m_array[array_idx];

    for(Uint j=0; j<row.size(); ++j)
      row_to_set[j] = row[j];
  }

  private: // data

    /// storage of the array
    ArrayT m_array;
};

std::ostream& operator<<(std::ostream& os, const CTable<bool>::ConstRow& row);
std::ostream& operator<<(std::ostream& os, const CTable<Uint>::ConstRow& row);
std::ostream& operator<<(std::ostream& os, const CTable<int>::ConstRow& row);
std::ostream& operator<<(std::ostream& os, const CTable<Real>::ConstRow& row);
std::ostream& operator<<(std::ostream& os, const CTable<std::string>::ConstRow& row);

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CTable_hpp
