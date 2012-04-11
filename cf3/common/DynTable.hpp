// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_DynTable_hpp
#define cf3_common_DynTable_hpp

////////////////////////////////////////////////////////////////////////////////

#include <deque>
#include "common/BasicExceptions.hpp"
#include "common/Component.hpp"
#include "common/StringConversion.hpp"
#include "common/Foreach.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {

////////////////////////////////////////////////////////////////////////////////

template <typename T>
class DynArrayBufferT;

/// Component holding a connectivity table with variable row-size per row
/// @author Willem Deconinck
template<typename T>
class DynTable : public common::Component {

public:

  typedef std::vector< std::vector<T> > ArrayT;
  typedef DynArrayBufferT<T> Buffer;
  typedef std::vector<T>& Row;
  typedef const std::vector<T>& ConstRow;

  /// Contructor
  /// @param name of the component
  DynTable ( const std::string& name ) : Component(name) { }

  ~DynTable () {}

  /// Get the class name
  static std::string type_name () { return "DynTable<"+common::class_name<T>()+">"; }

  Uint size() const { return m_array.size(); }

  void resize(const Uint new_size)
  {
    m_array.resize(new_size);
//    Uint difference = new_size - size();
//    if (difference > 0)
//    {
//      for (Uint i=0; i<difference; ++i)
//        m_array.push_back(std::vector<T>());
//    }
//    else
//    {
//      difference = -difference;
//      for (Uint i=0; i<difference; ++i)
//        m_array.pop_back();
//    }
  }

  Uint row_size(const Uint i) const {return m_array[i].size();}

  void set_row_size(const Uint i, const Uint s) { m_array[i].resize(s); }

  Buffer create_buffer(const size_t buffersize=16384)
  {
    return Buffer(m_array,buffersize);
  }

  boost::shared_ptr<Buffer> create_buffer_ptr(const size_t buffersize=16384)
  {
    return boost::shared_ptr<Buffer> ( new Buffer (m_array,buffersize) );
  }

  template<typename VectorT>
  void set_row(const Uint array_idx, const VectorT& row)
  {
    if (row.size() != row_size(array_idx))
      m_array[array_idx].resize(row.size());

    Uint j=0;
    boost_foreach( const typename VectorT::value_type& v, row)
      m_array[array_idx][j++] = v;
  }

  Row operator[] (const Uint idx)
  {
    return Row(m_array[idx]);
  }

  ConstRow operator[] (const Uint idx) const
  {
    return ConstRow(m_array[idx]);
  }

  /// @return A reference to the array data
  ArrayT& array() { return m_array; }

  /// @return A const reference to the array data
  const ArrayT& array() const { return m_array; }

private: // data

  ArrayT m_array;

};

//////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, DynTable<bool>::ConstRow row);
std::ostream& operator<<(std::ostream& os, DynTable<Uint>::ConstRow row);
std::ostream& operator<<(std::ostream& os, DynTable<int>::ConstRow row);
std::ostream& operator<<(std::ostream& os, DynTable<Real>::ConstRow row);
std::ostream& operator<<(std::ostream& os, DynTable<std::string>::ConstRow row);

std::ostream& operator<<(std::ostream& os, const DynTable<bool>& table);
std::ostream& operator<<(std::ostream& os, const DynTable<Uint>& table);
std::ostream& operator<<(std::ostream& os, const DynTable<int>& table);
std::ostream& operator<<(std::ostream& os, const DynTable<Real>& table);
std::ostream& operator<<(std::ostream& os, const DynTable<std::string>& table);

////////////////////////////////////////////////////////////////////////////////


template <typename T>
class DynArrayBufferT
{
public:
  typedef std::vector< std::vector<T> > Array_t;
  typedef std::vector<T>& Row;
  typedef const std::vector<T>& ConstRow;

private:

  struct Buffer
  {
    Buffer() {}
    Buffer(const Uint size) { resize(size); }
    Array_t rows;
    std::vector<bool> is_not_empty;
    void resize(const Uint size)
    {
      rows.resize(size);
      is_not_empty.resize(size);
      is_not_empty.assign(size,false);
    }
    void reset()
    {
      is_not_empty.assign(size(),false);
    }
    Uint size() const { return rows.size(); }
  };

public:

  DynArrayBufferT(Array_t& array, const size_t nb_rows) :
    m_array(array),
    m_buffersize(nb_rows)
  {}

  ~DynArrayBufferT()
  {
    flush();
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

    m_empty_array_rows.clear();
    m_empty_buffer_rows.clear();
  }

  std::string string()
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


  template <typename VectorT>
  Uint add_row(const VectorT& row)
  {
    if (m_new_buffer_rows.empty())
      add_buffer();
    Uint idx = m_new_buffer_rows.front();
    set_row(idx,row);
    m_new_buffer_rows.pop_front();
    return idx;
  }

  template<typename vectorType>
  void set_row(const Uint array_idx, const vectorType& row)
  {
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
          buffer.rows[array_idx-cummulative_size].resize(row.size());
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


 void rm_row(const Uint array_idx)
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

  Row get_row(const Uint idx)
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


  void flush()
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
      m_array.resize(new_size);

      // copy each buffer into the array
      Uint array_idx=old_array_size;
      BOOST_FOREACH (Buffer& buffer, m_buffers)
      {
        for (Uint row_idx=0; row_idx<buffer.size(); ++row_idx)
        {
          Row row = buffer.rows[row_idx];
          if (buffer.is_not_empty[row_idx])   // for each non-empty row from all buffers
          {
            // first find empty rows inside the old part array
            if (!m_empty_array_rows.empty())
            {
              Row empty_array_row = get_row(m_empty_array_rows.front());
              m_empty_array_rows.pop_front();
              empty_array_row = row;
            }
            else // then select the new array rows to be filled
            {
              cf3_assert(array_idx < m_array.size());
              Row empty_array_row=m_array[array_idx++];
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
          Row row = buffer.rows[row_idx];
          if (buffer.is_not_empty[row_idx])   // for each non-empty row from all buffers
          {
            Uint empty_array_row_idx = m_empty_array_rows.front();
            m_empty_array_rows.pop_front();
            Row empty_array_row = get_row(empty_array_row_idx);
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
      m_array.resize(new_size);
    }

    // clear all buffers
    reset();

  }

  Array_t& get_appointed() { return m_array; }

  Uint total_allocated()
  {
    Uint allocated=m_array.size();
    BOOST_FOREACH(const Buffer& buffer, m_buffers)
      allocated += buffer.size();
    return allocated;
  }

private:

  void add_buffer()
  {
    Uint idx = total_allocated();
    m_buffers.push_back(Buffer(m_buffersize));
    for (Uint i=0; i<m_buffersize; ++i)
      m_new_buffer_rows.push_back(idx++);
    cf3_assert(total_allocated()==idx);
  }

  bool is_array_row_empty(const Uint row) const
  {
    return (std::find(m_empty_array_rows.begin(),m_empty_array_rows.end(),row) != m_empty_array_rows.end());
  }

private:

  /// reference to the object the buffer works on
  Array_t& m_array;

  /// The size newly created buffers will have
  /// @note it is safe to change in the middle of buffer operations
  Uint m_buffersize;

  /// buffer storage;
  std::deque< std::vector<T> > m_buffer;

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


};

//////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_DynTable_hpp
