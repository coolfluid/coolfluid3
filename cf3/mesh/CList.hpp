// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_CList_hpp
#define cf3_mesh_CList_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Component.hpp"

#include "mesh/LibMesh.hpp"
#include "mesh/ListBufferT.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {

////////////////////////////////////////////////////////////////////////////////

/// @brief Component holding a 1 dimensional array of a templated type
///
/// The internal structure is that of a boost::multi_array,
/// so storage is contingent in memory for reducing cache missing
//
/// The list can be filled through a buffer. The buffer avoids
/// the typical reallocation in a std::vector. Flushing the buffer
/// will resize the list and copy itself into the list.
/// This happens automatically when the buffer is destroyed, or can
/// also be done manually. @see class ListBufferT
/// Before using the list one has to be sure that
/// the buffer is flushed.
///
/// @author Willem Deconinck
/// @author Bart Janssens
/// @author Tiago Quintino

template <typename ValueT>
class Mesh_API CList : public common::Component
{
public: // typedefs

  /// @brief the value type stored in each entry of the 2-dimensional table
  typedef ValueT value_type;

  /// @brief the type of the internal structure of the list
  typedef boost::multi_array<ValueT,1> ListT;

  /// @brief the type of the buffer used to interact with the table
  typedef ListBufferT<ValueT> Buffer;

  /// @brief boost::shared_ptr shortcut of this component
  typedef boost::shared_ptr<CList> Ptr;

  /// @brief boost::shared_ptr shortcut of this component const version
  typedef boost::shared_ptr<CList const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CList ( const std::string& name ) :
    Component ( name )
  {

  }

  /// Get the component type name
  /// @returns the component type name
  static std::string type_name () { return "CList<"+common::class_name<ValueT>()+">"; }

  /// Resize the array to the given number of rows
  /// @param[in] new_size The size allocated after resizing
  void resize(const Uint new_size)
  {
    m_array.resize(boost::extents[new_size]);
  }

  /// Modifiable access to the internal structure
  /// @return A reference to the array data
  ListT& array() { return m_array; }

  /// Non-modifiable access to the internal structure
  /// @return A const reference to the array data
  const ListT& array() const { return m_array; }

  /// Create a buffer with a given number of entries
  /// @param[in] buffersize the size that the buffer is allocated with
  ///                       the default value is 16384
  /// @return A Buffer object that can fill this Array
  Buffer create_buffer(const size_t buffersize=16384)
  {
    return Buffer(m_array,buffersize);
  }

  /// Create a buffer with a given number of entries
  /// @param[in] buffersize the size that the buffer is allocated with
  ///                       the default value is 16384
  /// @return A Buffer object that can fill this Array
  typename Buffer::Ptr create_buffer_ptr(const size_t buffersize=16384)
  {
    return typename Buffer::Ptr ( new Buffer(m_array,buffersize) );
  }


  /// Operator to have modifiable access to a list-entry
  /// @return A mutable entry of the underlying array
  ValueT& operator[](const Uint idx) { return m_array[idx]; }

  /// Operator to have non-modifiable access to a list-entry
  /// @return A const entry of the underlying array
  const ValueT& operator[](const Uint idx) const { return m_array[idx]; }

  /// Number of rows, excluding rows that may be in the buffer
  /// @return The number of local rows in the array
  Uint size() const { return m_array.size(); }

private: // data

  /// storage of the array
  ListT m_array;

};

////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const CList<bool>& list);
std::ostream& operator<<(std::ostream& os, const CList<Uint>& list);
std::ostream& operator<<(std::ostream& os, const CList<int>& list);
std::ostream& operator<<(std::ostream& os, const CList<Real>& list);
std::ostream& operator<<(std::ostream& os, const CList<std::string>& list);

/////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_CList_hpp
