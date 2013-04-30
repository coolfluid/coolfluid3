// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_ListBufferT_hpp
#define cf3_common_ListBufferT_hpp

////////////////////////////////////////////////////////////////////////////////

#include <deque>

#include "common/Foreach.hpp"
#include "common/BoostArray.hpp"
#include "common/BasicExceptions.hpp"
#include "common/StringConversion.hpp"

#include "common/ListBufferIterator.hpp"

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
/// @note Before using the matching table or array one has to be sure that
/// the buffer is flushed.
/// @author Willem Deconinck
template < typename T >
class ListBufferT
{
public: // typedef

  /// type of the iterator
  typedef ListBufferIterator<ListBufferT> iterator;
  /// type of the iterator to constant Component
  typedef ListBufferIterator<ListBufferT const> const_iterator;


  typedef boost::multi_array<T,1> Array_t;
  typedef T value_type;

private:

  struct Buffer
  {
    Buffer() {}
    Buffer(const Uint size) { resize(size); }
    Array_t rows;
    std::vector<bool> is_not_empty;
    void resize(const Uint size)
    {
      rows.resize(boost::extents[size]);
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
  ListBufferT (Array_t& array, size_t nbRows);

  /// Virtual destructor
  virtual ~ListBufferT();

  /// Get the class name
  static std::string type_name () { return "ListBuffer"; }

  // functions specific to the Buffer component

  /// Change the buffer to the new size
  void change_buffersize(const size_t nbRows);

  /// flush the buffer in the connectivity Buffer
  void flush();
  //
  /// Add a row to the buffer.
  /// @param [in] val value to be added to buffer
  /// @return the index in the array+buffers
  Uint add_row(const value_type& val);

  /// Add a row directly to the array
  /// @param [in] val value to be added to buffer or array
  /// @return the index in the array+buffers
  Uint add_row_directly(const value_type& val);

  /// copy a given row into the array or buffer, depending on the given index
  /// @param [in] array_idx the index of the row that will be set (both in array and buffers)
  /// @param [in] row       the row that will be copied into the buffer or array
  void set_row(const Uint array_idx, const value_type& row);

  /// @return the row with index idx, searching both in array and buffers
  value_type& get_row(const Uint idx);

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


  /// The begin iterator for a range containing Components
  iterator begin()
  {
    return iterator(*this,0);
  }

  /// The end iterator for a range containing Components
  iterator end()
  {
    return iterator(*this,total_allocated());
  }

  /// The begin iterator for a range containing Components (const version)
  const_iterator begin() const
  {
    return const_iterator(*this,0);
  }

  /// The end iterator for a range containing Components (const version)
  const_iterator end() const
  {
    return const_iterator(*this,total_allocated());
  }

  void reset()
  {
    m_buffers.resize(0);
    m_new_buffer_rows.clear();
//    Uint idx = m_array.size();
//    boost_foreach(Buffer& buffer, m_buffers)
//    {
//       buffer.reset();
//       m_new_buffer_rows.push_back(idx++);
//    }

    m_new_array_rows.clear();
    m_empty_array_rows.clear();
    m_empty_buffer_rows.clear();
  }

  std::string string();

private: // functions

  /// Create a new buffer, allocate it with m_buffersize, and fill m_new_buffer_rows with the new ones.
  void add_buffer();

  bool is_array_row_empty(const Uint row) const
  {
    return (std::find(m_empty_array_rows.begin(),m_empty_array_rows.end(),row) != m_empty_array_rows.end());
  }

private: // data

  /// reference to the array that is buffered
  Array_t& m_array;

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
ListBufferT<T>::ListBufferT (typename ListBufferT<T>::Array_t& array, size_t nbRows) :
  m_array(array),
  m_buffersize(nbRows)
{
}

////////////////////////////////////////////////////////////////////////////////

template<typename T>
ListBufferT<T>::~ListBufferT()
{
  // make sure to flush before deleting the buffer
  flush();
}

////////////////////////////////////////////////////////////////////////////////

template<typename T>
inline Uint ListBufferT<T>::total_allocated()
{
  Uint allocated=m_array.size();
  BOOST_FOREACH(const Buffer& buffer, m_buffers)
    allocated += buffer.size();
  return allocated;
}

////////////////////////////////////////////////////////////////////////////////

template<typename T>
void ListBufferT<T>::flush()
{

  // get total number of allocated rows
  Uint allocated_size = total_allocated();
  Uint old_array_size = m_array.size();

  // get total number of empty rows
  Uint nb_emptyRows = m_empty_array_rows.size() + m_empty_buffer_rows.size() + m_new_buffer_rows.size();
  Uint new_size = allocated_size-nb_emptyRows;
  if (new_size >= old_array_size)
  {
    // make m_array bigger
    m_array.resize(boost::extents[new_size]);

    // copy each buffer into the array
    Uint array_idx=old_array_size;
    boost_foreach (Buffer& buffer, m_buffers)
    {
      for (Uint row_idx=0; row_idx<buffer.size(); ++row_idx)
      {
        value_type& row = buffer.rows[row_idx];
        if (buffer.is_not_empty[row_idx])   // for each non-empty row from all buffers
        {
          // first find empty rows inside the old part array
          if (!m_empty_array_rows.empty())
          {
            value_type& empty_array_row = get_row(m_empty_array_rows.front());
            m_empty_array_rows.pop_front();
            empty_array_row = row;
          }
          else // then select the new array rows to be filled
          {
            cf3_assert(array_idx < m_array.size());
            value_type& empty_array_row=m_array[array_idx++];
            empty_array_row = row;
          }
        }
      }
    }
  }
  else // More rows to be removed than added, now we need to swap rows
  {
    // copy all buffer rows in the m_array
    boost_foreach (Buffer& buffer, m_buffers)
    {
      for (Uint row_idx=0; row_idx<buffer.size(); ++row_idx)
      {
        value_type& row = buffer.rows[row_idx];
        if (buffer.is_not_empty[row_idx])   // for each non-empty row from all buffers
        {
          Uint empty_array_row_idx = m_empty_array_rows.front();
          m_empty_array_rows.pop_front();
          value_type& empty_array_row = get_row(empty_array_row_idx);
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
        //m_empty_array_rows.push_back(full_row_idx);
        full_row_idx++;
      }
    }

    // make m_array smaller
    m_array.resize(boost::extents[new_size]);
  }

  // clear all buffers
  reset();
}

//////////////////////////////////////////////////////////////////////////////

template<typename T>
inline typename ListBufferT<T>::value_type& ListBufferT<T>::get_row(const Uint idx)
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
inline void ListBufferT<T>::increase_array_size(const size_t increase)
{
  Uint old_size = m_array.size();
  Uint new_size = old_size+increase;
  m_array.resize(boost::extents[new_size]);
  for (Uint i_new=old_size; i_new<new_size; ++i_new)
  {
    m_new_array_rows.push_back(i_new);
  }
}

//////////////////////////////////////////////////////////////////////

template<typename T>
inline void ListBufferT<T>::add_buffer()
{
  Uint idx = total_allocated();
  m_buffers.push_back(Buffer(m_buffersize));
  for (Uint i=0; i<m_buffersize; ++i)
    m_new_buffer_rows.push_back(idx++);
  cf3_assert(total_allocated()==idx);
}

//////////////////////////////////////////////////////////////////////////////

template<typename T>
inline Uint ListBufferT<T>::add_row(const value_type& row)
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
inline Uint ListBufferT<T>::add_row_directly(const value_type& row)
{
  cf3_assert(!m_new_array_rows.empty());
  Uint idx = m_new_array_rows.front();
  set_row(idx,row);
  m_new_array_rows.pop_front();
  return idx;
}

//////////////////////////////////////////////////////////////////////

template<typename T>
inline void ListBufferT<T>::set_row(const Uint array_idx, const value_type& row)
{
  Uint cummulative_size = m_array.size();
  if (array_idx < cummulative_size)
  {
    m_array[array_idx] = row;
    m_empty_array_rows.erase(std::find(m_empty_array_rows.begin(),m_empty_array_rows.end(),array_idx));
    return;
  }
  else
  {
    BOOST_FOREACH(Buffer& buffer, m_buffers)
    {
      if (array_idx<cummulative_size+buffer.size())
      {
        buffer.rows[array_idx-cummulative_size]=row;
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
inline void ListBufferT<T>::rm_row(const Uint array_idx)
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

    boost_foreach(Buffer& buffer, m_buffers)
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
inline void ListBufferT<T>::change_buffersize(const size_t buffersize)
{
  m_buffersize = buffersize;
}

//////////////////////////////////////////////////////////////////////////////

template<typename T>
inline std::string ListBufferT<T>::string()
{
  using namespace common;
  std::string str;
  for (Uint i=0; i<m_array.size(); ++i)
  {
    str += "    " + to_str(i) + ":    ";
    if (is_array_row_empty(i))
      str += "X   (" + to_str(m_array[i]) + ")\n";
    else
      str += to_str(m_array[i]) + "\n";
  }
  Uint s=m_array.size();
  for (Uint b=0; b<m_buffers.size(); ++b)
  {
    str += "    ----buffer["+to_str(b)+"]----\n";
    for (Uint i=0; i<m_buffers[b].size(); ++i)
    {
      str += "    " + to_str(s) + ":    ";
      if ( std::find(m_empty_buffer_rows.begin(),m_empty_buffer_rows.end(),s) != m_empty_buffer_rows.end())
        str += "X   (" + to_str(m_buffers[b].rows[i]) + ")\n";
      else if( std::find(m_new_buffer_rows.begin(),m_new_buffer_rows.end(),s) != m_new_buffer_rows.end())
        str += "\n";
      else
        str += to_str(m_buffers[b].rows[i]) + "\n";
      ++s;
    }
  }
  return str;
}

//////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_ListBufferT_hpp
