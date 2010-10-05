// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_ListBase_hpp
#define CF_Mesh_ListBase_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/BoostArray.hpp"

#include "Mesh/LibMesh.hpp"
// #include "Mesh/BufferT.hpp"

namespace CF {
namespace Mesh {
  
////////////////////////////////////////////////////////////////////////////////

/// Base class for 1-dimensional array data
/// @author Willem Deconinck, Tiago Quintino, Bart Janssens
template<typename ValueT>
class ListBase {

public: // typedefs
  typedef ValueT value_type;
  typedef boost::multi_array<ValueT,1> ListT;
  //typedef BufferT<ValueT> Buffer;

public: // functions
  
  /// Resize the array to the given number of rows
  void resize(const Uint nb_rows)
  {
    m_array.resize(boost::extents[nb_rows]);
  }

  /// @return A reference to the array data
  ListT& array() { return m_array; }

  /// @return A const reference to the array data
  const ListT& array() const { return m_array; }

  // /// @return A Buffer object that can fill this Array
  // Buffer create_buffer(const size_t buffersize=1024)
  // {
  //   // make sure the array has its columnsize defined
  //   cf_assert(row_size() > 0);
  //   return Buffer(m_array,buffersize); 
  // }

  /// @return A mutable row of the underlying array
  ValueT& operator[](const Uint idx) { return m_array[idx]; }

  /// @return A const row of the underlying array
  const ValueT& operator[](const Uint idx) const { return m_array[idx]; }
  
  /// @return The number of local rows in the array
  Uint size() const { return m_array.size(); }
  
private: // data

  /// storage of the array
  ListT m_array;
};

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_ListBase_hpp
