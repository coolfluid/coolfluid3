// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CDynTable_hpp
#define CF_Mesh_CDynTable_hpp

////////////////////////////////////////////////////////////////////////////////

#include <deque>
#include "Common/Component.hpp"
#include "Common/Log.hpp"
#include "Common/BasicExceptions.hpp"
#include "Common/Foreach.hpp"
#include "Mesh/LibMesh.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

////////////////////////////////////////////////////////////////////////////////

template <typename ArrayT>
class DynArrayBufferT;

/// Component holding a connectivity table with variable row-size per row
/// @author Willem Deconinck
template<typename T>
class Mesh_API CDynTable : public Common::Component {

public:
  typedef boost::shared_ptr<CDynTable> Ptr;
  typedef boost::shared_ptr<CDynTable const> ConstPtr;

  typedef std::deque< std::vector<T> > ArrayT;
  typedef DynArrayBufferT<T> Buffer;
  typedef std::vector<T>& Row;
  typedef const std::vector<T>& ConstRow;
  
  /// Contructor
  /// @param name of the component
  CDynTable ( const std::string& name ) : Component(name)
  {
     
  }

  ~CDynTable () {}

  /// Get the class name
  static std::string type_name () { return "CDynTable<"+class_name<T>()+">"; }

  Uint size() const { return m_array.size(); }
  
  Uint row_size(const Uint i) const {return m_array[i].size();}
  
  Buffer create_buffer()
  {
    return Buffer(m_array);
  }
  
  template <typename VectorT>
  Uint add_row(const VectorT& row)
  {
    m_array.push_back(std::vector<T>(row.size()));
    Uint array_idx = m_array.size()-1;
    Uint j=0;
    boost_foreach(const T row_elem, row)
      m_array[array_idx][j++] = row_elem;
    return array_idx;
  }

  template<typename DoubleVectorT>
  void add_rows(const DoubleVectorT& rows)
  {
    boost_foreach(const typename DoubleVectorT::value_type& row, rows)
    {
      add_row(row);
    }
  }
  
  template<typename VectorT>
  void set_row(const Uint array_idx, const VectorT& row)
  {
    cf_assert(row.size() == row_size(array_idx));
    for(Uint j=0; j<row.size(); ++j)
      m_array[array_idx][j] = row[j];
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

  std::deque< std::vector<T> > m_array;

};

//////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const CDynTable<bool>::ConstRow row);
std::ostream& operator<<(std::ostream& os, const CDynTable<Uint>::ConstRow row);
std::ostream& operator<<(std::ostream& os, const CDynTable<int>::ConstRow row);
std::ostream& operator<<(std::ostream& os, const CDynTable<Real>::ConstRow row);
std::ostream& operator<<(std::ostream& os, const CDynTable<std::string>::ConstRow row);

std::ostream& operator<<(std::ostream& os, const CDynTable<bool>& table);
std::ostream& operator<<(std::ostream& os, const CDynTable<Uint>& table);
std::ostream& operator<<(std::ostream& os, const CDynTable<int>& table);
std::ostream& operator<<(std::ostream& os, const CDynTable<Real>& table);
std::ostream& operator<<(std::ostream& os, const CDynTable<std::string>& table);
  
////////////////////////////////////////////////////////////////////////////////


template <typename T>
class DynArrayBufferT
{
public:
  typedef std::deque< std::vector<T> > Array_t;
  typedef std::vector<T>& Row;
  
  DynArrayBufferT(Array_t& array) : m_array(array) {}
  
  ~DynArrayBufferT()
  {
    flush();
  }
  
  template <typename VectorT>
  Uint add_row(const VectorT& row)
  {
    m_buffer.push_back(std::vector<T>(row.size()));
    Uint buffer_idx = m_buffer.size()-1;
    Uint j=0;
    boost_foreach(const T row_elem, row)
      m_buffer[buffer_idx][j++] = row_elem;
    return m_array.size() + buffer_idx;
  }
  
 void rm_row(const Uint array_idx)
  {
    set_empty(get_row(array_idx));
    if (array_idx < m_array.size()) 
      m_emptyArrayRows.push_back(array_idx);
    else
      m_emptyBufferRows.push_back(array_idx);
  }
  
  Row get_row(const Uint idx)
  {  
    Uint array_size = m_array.size();
    if (idx < array_size) 
    {
      return m_array[idx];
    }
    else 
    {
      if (idx<array_size+m_buffer.size())
        return m_buffer[idx-array_size];
    }
		cf_assert_desc("Trying to access index that is not allocated",false);
		return m_array[0];
  }
  
  /// @return true if the given row is empty
  bool is_empty(const Row row) const  
  { 
    return row.size()==0 ? false : row[0]==INVALID; 
  }

  /// mark the given row as empty
  /// @param [in] row the row to be marked as empty
  void set_empty(Row row) { row.resize(1); row[0]=INVALID; }  
  
  boost::shared_ptr< std::vector<Uint> > flush()
  {

    // get total number of allocated rows
    Uint old_array_size = m_array.size();

    // get total number of empty rows
    Uint nb_emptyRows = m_emptyArrayRows.size() + m_emptyBufferRows.size();
    Uint new_size = old_array_size + m_buffer.size() - nb_emptyRows;

  	// a vector of index changes compared to before the flush
  	boost::shared_ptr< std::vector<Uint> > old_indexes_sptr (new std::vector<Uint>(new_size));
  	std::vector<Uint>& old_indexes = *old_indexes_sptr;
  	for (Uint i=0; i<new_size; ++i)
  		old_indexes[i]=i;
  	Uint new_idx;
  	Uint old_idx = old_array_size;

    if (new_size > old_array_size) 
    {
      // make m_array bigger
      m_array.resize(new_size);

      // copy each buffer into the array
      Uint array_idx=old_array_size;
      boost_foreach (Row row, m_buffer)
  		{
  			if (!is_empty(row))   // for each non-empty row from all buffers
  			{
  				// first find empty rows inside the old part array
  				if (!m_emptyArrayRows.empty()) 
  				{
						new_idx = m_emptyArrayRows.front();
  					m_emptyArrayRows.pop_front();

  					Row empty_array_row = get_row(new_idx);
  					old_indexes[new_idx]=old_idx;

            empty_array_row = row;
  				}
  				else // then select the new array rows to be filled
  				{
  					Row empty_array_row=m_array[array_idx++];
            empty_array_row.resize(row.size());
            for (Uint j=0; j<row.size(); ++j)
              empty_array_row[j] = row[j];
  				}
  			}
  			++old_idx;
  		}
  	}
    else // More rows to be removed than added, now we need to swap rows
    {
      // copy all buffer rows in the m_array
			boost_foreach (Row row, m_buffer)
			if (!is_empty(row))   // for each non-empty row from all buffers
			{     
				new_idx = m_emptyArrayRows.front();
				m_emptyArrayRows.pop_front();

				Row empty_array_row = get_row(new_idx);
				old_indexes[new_idx]=old_idx;

				empty_array_row = row;
			}
			++old_idx;

      Uint full_row_idx = new_size;
      // The part of the table with rows > new_size will be deallocated
      // The empty rows from the allocated part must be swapped with filled 
      // rows from the part that will be deallocated
      boost_foreach(Uint empty_row_idx, m_emptyArrayRows)
      {
        // swap only necessary if it the empty row is in the allocated part
        if (empty_row_idx < new_size)
        { 
          // swap this empty row with a full one in the part that will be deallocated

          // 1) find next full row
					cf_assert_desc("Index out of bounds",full_row_idx < m_array.size()); 
          while(is_empty(m_array[full_row_idx]))
            full_row_idx++; 
          // 2) swap them 
					cf_assert_desc("Index out of bounds",full_row_idx < m_array.size()); 
          swap(m_array[empty_row_idx],m_array[full_row_idx]);
  				// 3) add to change_set
  				new_idx = empty_row_idx;
  				old_idx = full_row_idx;
  				old_indexes[new_idx]=old_idx;
          full_row_idx++;
        }
      }

      // make m_array smaller
      m_array.resize(new_size);
    }

    // clear all buffers
    m_buffer.clear();
    m_emptyArrayRows.clear();
    m_emptyBufferRows.clear();

  	return old_indexes_sptr;
  }
  
  
  void swap(Row lhs, Row rhs)
  {
    std::vector<T> tmp = lhs;
    lhs = rhs;
    rhs = tmp;
  }
  
  Array_t& get_appointed() { return m_array; }
  
private:
  
  /// reference to the object the buffer works on
  Array_t& m_array;
  
  /// storage of removed array rows
  std::deque<Uint> m_emptyArrayRows;
  
  /// storage of array rows where rows can be added directly using add_row_directly
  std::deque<Uint> m_newArrayRows;
  
  /// storage of removed buffer rows
  std::deque<Uint> m_emptyBufferRows;
  
  /// buffer storage;
  std::deque< std::vector<T> > m_buffer;
  
  /// definition of an invalid element
  static const T INVALID;
  
};

//////////////////////////////////////////////////////////////////////////////

template<typename T>
const T DynArrayBufferT<T>::INVALID = std::numeric_limits<T>::max();

//////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CDynTable_hpp
