// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_ArrayBufferT_hpp
#define cf3_common_ArrayBufferT_hpp

////////////////////////////////////////////////////////////////////////////////

#include <deque>

#include <boost/foreach.hpp>

#include "common/BoostArray.hpp"
#include "common/BasicExceptions.hpp"
#include "common/StringConversion.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {

////////////////////////////////////////////////////////////////////////////////

/// @brief A Buffer that is constructed passing a boost::multi_array<T,2> table.
/// This class allows to interface this table by using a buffer.
///
/// The idea is to add and remove rows from the table through this buffer.
/// The table is resized when the buffer is full, and values are copied from
/// the buffer into the table.
///
/// First entry that is removed from the array using rm_row(), will also be the first to be filled
/// when non-empty buffers are flushed. So in order of removal.
///
/// @note Before using the matching array in algorithms, one has to be sure that
/// the buffer is flushed. This is done automatically at buffer destruction,
/// or manually by calling flush().
///
/// @author Willem Deconinck

template < typename T >
class ArrayBufferT
{

public: // typedef

  typedef boost::multi_array<T,2> Array_t;
  typedef T value_type;

  typedef boost::detail::multi_array::sub_array<T,1> SubArray_t;
  typedef boost::detail::multi_array::const_sub_array<T,1> ConstSubArray_t;

private:

  struct Buffer
  {
    Buffer() {}
    Buffer(const Uint size, const Uint nb_cols) { resize(size,nb_cols); }
    Array_t rows;
    std::vector<bool> is_not_empty;
    void resize(const Uint size, const Uint nb_cols)
    {
      rows.resize(boost::extents[size][nb_cols]);
      is_not_empty.resize(size);
      is_not_empty.assign(size,false);
    }
    void reset()
    {
      is_not_empty.assign(size(),false);
    }
    Uint size() const { return rows.size(); }
  };

public: // functions

  /// Contructor
  /// @param array The table that will be interfaced with
  /// @param nbRows The size the buffer will be allocated with
  ArrayBufferT (Array_t& array, size_t nbRows);

  /// Virtual destructor
  virtual ~ArrayBufferT();

  /// Get the class name
  static std::string type_name () { return "Buffer"; }

  // functions specific to the Buffer component

  /// Change the buffer to the new size
  void change_buffersize(const size_t nbRows);

  /// Flush the buffer in the connectivity Buffer
  /// 2 cases:
  /// - Array has to expand
  ///   - resize array
  ///   - copy all non-empty buffer entries in sequence to array entries marked to be removed (first one removed, is first one refilled)
  ///   - copy all non-empty buffer entries in sequence to array entries in the expanded part
  /// - Array has to shrink
  ///   - copy all non-empty buffer entries in sequence to array entries marked to be removed (lower indices first)
  ///   - swap entries in the array starting from the index old_array_size to remaining empty array entries in the new array
  ///   - resize array

  void flush();

  /// Add a row to the buffer.
  /// rows are only added to the buffer, even if there are empty rows in the array!
  /// Only when flush() is called, will the empty rows be filled.
  /// @param [in] row Row to be added to buffer
  /// @return the index in the array+buffers. If array has size 4, and buffer size 3, the last idx will be 6;
  template<typename vectorType>
  Uint add_row(const vectorType& row);

  Uint add_empty_row();

  /// Add a row directly to the array
  /// @param [in] row Row to be added to buffer or array
  /// @return the index in the array+buffers
  template<typename vectorType>
  Uint add_row_directly(const vectorType& row);

  /// copy a given row into the array or buffer, depending on the given index
  /// @param [in] array_idx the index of the row that will be set (both in array and buffers)
  /// @param [in] row       the row that will be copied into the buffer or array
  template<typename vectorType>
  void set_row(const Uint array_idx, const vectorType& row);

  /// @return the row with index idx, searching both in array and buffers
  SubArray_t get_row(const Uint idx);

  /// Mark row as empty in array or buffer
  /// @param [in] array_idx the index of the row to be removed
  void rm_row(const Uint array_idx);

  /// @return the array that the buffer operates on
  Array_t& get_appointed() {return m_array;}

  /// @return total number of allocated rows, including all buffers and the array
  Uint total_allocated();

  /// @return the number of buffers that are created
  Uint buffers_count() const { return m_buffers.size(); }

  /// increase the size of the array, only to be used when going to write directly in array
  void increase_array_size(const size_t increase);

private: // functions

  /// Create a new buffer, allocate it with m_buffersize, and fill m_new_buffer_rows with the new ones.
  void add_buffer();

  bool is_array_row_empty(const Uint row) const
  {
    return (std::find(m_empty_array_rows.begin(),m_empty_array_rows.end(),row) != m_empty_array_rows.end());
  }

  void reset()
  {
    m_buffers.resize(0);
    m_new_buffer_rows.clear();
//    Uint idx = m_array.size();
//    BOOST_FOREACH(Buffer& buffer, m_buffers)
//    {
//       buffer.reset();
//       m_new_buffer_rows.push_back(idx++);
//    }

    m_new_array_rows.clear();
    m_empty_array_rows.clear();
    m_empty_buffer_rows.clear();
  }

  std::string string();

private: // data

  /// reference to the array that is buffered
  Array_t& m_array;

  /// the number of columns of the array
  Uint m_nb_cols;

  /// The size newly created buffers will have
  /// @note it is safe to change in the middle of buffer operations
  Uint m_buffersize;

  /// vector of temporary buffers
  std::vector<Buffer> m_buffers;

  /// storage of removed array rows
  std::deque<Uint> m_empty_array_rows;

  /// storage of array rows where rows can be added directly using add_row_directly
  std::deque<Uint> m_new_array_rows;

  /// storage of removed buffer rows
  std::deque<Uint> m_empty_buffer_rows;

  /// storage of buffer rows where rows can be added
  std::deque<Uint> m_new_buffer_rows;

}; // ConnectivityTable

////////////////////////////////////////////////////////////////////////////////

template<typename T>
ArrayBufferT<T>::ArrayBufferT (typename ArrayBufferT<T>::Array_t& array, size_t nbRows) :
  m_array(array),
  m_nb_cols(m_array.shape()[1]),
  m_buffersize(nbRows)
{
}

////////////////////////////////////////////////////////////////////////////////

template<typename T>
ArrayBufferT<T>::~ArrayBufferT()
{
  // make sure to flush before deleting the buffer
  flush();
}

////////////////////////////////////////////////////////////////////////////////

template<typename T>
inline Uint ArrayBufferT<T>::total_allocated()
{
  Uint allocated=m_array.size();
  BOOST_FOREACH(const Buffer& buffer, m_buffers)
    allocated += buffer.size();
  return allocated;
}

////////////////////////////////////////////////////////////////////////////////

template<typename T>
void ArrayBufferT<T>::flush()
{

  // get total number of allocated rows
  Uint allocated_size = total_allocated();
  Uint old_array_size = m_array.size();

  // get total number of empty rows
  Uint nb_emptyRows = m_empty_array_rows.size() + m_empty_buffer_rows.size() + m_new_buffer_rows.size();
  Uint new_size = allocated_size-nb_emptyRows;

  if (new_size > old_array_size)
  {
    // make m_array bigger
    m_array.resize(boost::extents[new_size][m_nb_cols]);

    // copy each buffer into the array
    Uint array_idx=old_array_size;
    BOOST_FOREACH (Buffer& buffer, m_buffers)
    {
      for (Uint row_idx=0; row_idx<buffer.size(); ++row_idx)
      {
        SubArray_t row = buffer.rows[row_idx];
        if (buffer.is_not_empty[row_idx])   // for each non-empty row from all buffers
        {
          // first find empty rows inside the old part array
          if (!m_empty_array_rows.empty())
          {
            SubArray_t empty_array_row = get_row(m_empty_array_rows.front());
            m_empty_array_rows.pop_front();
            empty_array_row = row;
          }
          else // then select the new array rows to be filled
          {
            cf3_assert(array_idx < m_array.size());
            SubArray_t empty_array_row=m_array[array_idx++];
            empty_array_row = row;
          }
        }
      }
    }
  }
  else // More rows to be removed than added, now we need to swap rows
  {
    // copy all buffer rows in the m_array
    BOOST_FOREACH (Buffer& buffer, m_buffers)
    {
      for (Uint row_idx=0; row_idx<buffer.size(); ++row_idx)
      {
        SubArray_t row = buffer.rows[row_idx];
        if (buffer.is_not_empty[row_idx])   // for each non-empty row from all buffers
        {
          Uint empty_array_row_idx = m_empty_array_rows.front();
          m_empty_array_rows.pop_front();
          SubArray_t empty_array_row = get_row(empty_array_row_idx);
          empty_array_row = row;
        }
      }
    }
    Uint full_row_idx = new_size;
    // The part of the table with rows > new_size will be deallocated
    // The empty rows from the allocated part must be swapped with filled
    // rows from the part that will be deallocated
    Uint nb_empty_rows = m_empty_array_rows.size();
    for (Uint e=0; e<nb_empty_rows; ++e)
    {
      Uint empty_row_idx = m_empty_array_rows[e];
      // swap only necessary if the empty row is in the allocated part
      if (empty_row_idx < new_size)
      {
        // swap this empty row with a full one in the part that will be deallocated

        // 1) find next full row
        cf3_assert(full_row_idx<m_array.size());
        while(is_array_row_empty(full_row_idx))
        {
          full_row_idx++;
          cf3_assert(full_row_idx<m_array.size());
        }

        // 2) swap them
        cf3_assert(empty_row_idx<m_array.size());
        m_array[empty_row_idx] = m_array[full_row_idx];
        full_row_idx++;
      }
    }

    // make m_array smaller
    m_array.resize(boost::extents[new_size][m_nb_cols]);
  }

  // clear all buffers
  reset();
}

//////////////////////////////////////////////////////////////////////////////

template<typename T>
inline typename ArrayBufferT<T>::SubArray_t ArrayBufferT<T>::get_row(const Uint idx)
{
  Uint cummulative_size = m_array.size();
  if (idx < cummulative_size)
  {
    return m_array[idx];
  }
  else
  {
    BOOST_FOREACH(Buffer& buffer, m_buffers)
    {
      if (idx<cummulative_size+buffer.size())
        return buffer.rows[idx-cummulative_size];
      cummulative_size += buffer.size();
    }
  }
  throw common::BadValue(FromHere(),"Trying to access index that is not allocated: ["+common::to_str(idx)+">="+common::to_str(cummulative_size)+"]");
  return m_array[0];
}

//////////////////////////////////////////////////////////////////////

template<typename T>
inline void ArrayBufferT<T>::increase_array_size(const size_t increase)
{
  Uint old_size = m_array.size();
  Uint new_size = old_size+increase;
  m_array.resize(boost::extents[new_size][m_nb_cols]);
  for (Uint i_new=old_size; i_new<new_size; ++i_new)
  {
    m_new_array_rows.push_back(i_new);
  }
}

//////////////////////////////////////////////////////////////////////

template<typename T>
inline void ArrayBufferT<T>::add_buffer()
{
  Uint idx = total_allocated();
  m_buffers.push_back(Buffer(m_buffersize,m_nb_cols));
  for (Uint i=0; i<m_buffersize; ++i)
    m_new_buffer_rows.push_back(idx++);
  cf3_assert(total_allocated()==idx);
}

//////////////////////////////////////////////////////////////////////////////

template<typename T>
template<typename vectorType>
inline Uint ArrayBufferT<T>::add_row(const vectorType& row)
{
  if (m_new_buffer_rows.empty())
    add_buffer(); // will make a whole lot of new new_buffer_rows
  Uint idx = m_new_buffer_rows.front();
  set_row(idx,row);
  m_new_buffer_rows.pop_front();
  return idx;
}

//////////////////////////////////////////////////////////////////////////////

template<typename T>
inline Uint ArrayBufferT<T>::add_empty_row()
{
  if (m_new_buffer_rows.empty())
    add_buffer(); // will make a whole lot of new new_buffer_rows
  Uint idx = m_new_buffer_rows.front();
  std::vector<T> dummy(m_nb_cols);
  set_row( idx , std::vector<T>(m_nb_cols) );
  m_new_buffer_rows.pop_front();
  return idx;
}

//////////////////////////////////////////////////////////////////////////////

template<typename T>
template<typename vectorType>
inline Uint ArrayBufferT<T>::add_row_directly(const vectorType& row)
{
  cf3_assert(!m_new_array_rows.empty());
  Uint idx = m_new_array_rows.front();
  set_row(idx,row);
  m_new_array_rows.pop_front();
  return idx;
}

//////////////////////////////////////////////////////////////////////

template<typename T>
template<typename vectorType>
inline void ArrayBufferT<T>::set_row(const Uint array_idx, const vectorType& row)
{
  cf3_assert(row.size() == m_nb_cols);
  Uint cummulative_size = m_array.size();
  if (array_idx < cummulative_size)
  {
    for (Uint i=0; i<row.size(); ++i)
      m_array[array_idx][i] = row[i];
    m_empty_array_rows.erase(std::find(m_empty_array_rows.begin(),m_empty_array_rows.end(),array_idx));
    return;
  }
  else
  {
    BOOST_FOREACH(Buffer& buffer, m_buffers)
    {
      if (array_idx<cummulative_size+buffer.size())
      {
        for (Uint i=0; i<row.size(); ++i)
          buffer.rows[array_idx-cummulative_size][i]=row[i];
        buffer.is_not_empty[array_idx-cummulative_size]=true;
        return;
      }
      cummulative_size += buffer.size();
    }
  }
  throw common::BadValue(FromHere(),"Trying to access index that is not allocated");
}

//////////////////////////////////////////////////////////////////////////////

template<typename T>
inline void ArrayBufferT<T>::rm_row(const Uint array_idx)
{
  Uint cummulative_size = m_array.size();
  if (array_idx < cummulative_size)
  {
    m_empty_array_rows.push_back(array_idx);
    return;
  }
  else
  {
    m_empty_buffer_rows.push_back(array_idx);

    BOOST_FOREACH(Buffer& buffer, m_buffers)
    {
      if (array_idx<cummulative_size+buffer.size())
      {
        buffer.is_not_empty[array_idx-cummulative_size]=false;
        return;
      }
      cummulative_size += buffer.size();
    }
  }
  throw common::BadValue(FromHere(),"Trying to access index that is not allocated");
}

//////////////////////////////////////////////////////////////////////////////

template<typename T>
inline void ArrayBufferT<T>::change_buffersize(const size_t buffersize)
{
  m_buffersize = buffersize;
}

////////////////////////////////////////////////////////////////////////////////

template<typename T>
inline std::string ArrayBufferT<T>::string()
{
  using namespace common;
  std::string str;
  for (Uint i=0; i<m_array.size(); ++i)
  {
    str += "    " + to_str(i) + ":    ";
    if (is_array_row_empty(i))
    {
      str += "X   ( ";
      for (Uint j=0; j<m_array[i].size(); ++j)
        str += to_str(m_array[i][j]) + " ";
      str += ")\n";
    }
    else
    {
      for (Uint j=0; j<m_array[i].size(); ++j)
        str += to_str(m_array[i][j]) + " ";
      str += "\n";
    }
  }
  Uint s=m_array.size();
  for (Uint b=0; b<m_buffers.size(); ++b)
  {
    str += "    ----buffer["+to_str(b)+"]----\n";
    for (Uint i=0; i<m_buffers[b].size(); ++i)
    {
      str += "    " + to_str(s) + ":    ";
      if ( std::find(m_empty_buffer_rows.begin(),m_empty_buffer_rows.end(),s) != m_empty_buffer_rows.end())
      {
        str += "X   ( ";
        for (Uint j=0; j<m_buffers[b].rows[i].size(); ++j)
          str += to_str(m_buffers[b].rows[i][j]) + " ";
        str += ")\n";
      }
      else if( std::find(m_new_buffer_rows.begin(),m_new_buffer_rows.end(),s) != m_new_buffer_rows.end())
      {
        str += "\n";
      }
      else
      {
        for (Uint j=0; j<m_buffers[b].rows[i].size(); ++j)
          str += to_str(m_buffers[b].rows[i][j]) + " ";
        str += "\n";
      }
      ++s;
    }
  }
  return str;
}

////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_ArrayBufferT_hpp
