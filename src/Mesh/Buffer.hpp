#ifndef CF_Mesh_Buffer_hpp
#define CF_Mesh_Buffer_hpp

////////////////////////////////////////////////////////////////////////////////

#define BOOST_MULTI_ARRAY_NO_GENERATORS false
#include "boost/multi_array.hpp" 
#undef BOOST_MULTI_ARRAY_NO_GENERATORS

#include "Mesh/MeshAPI.hpp"

/// @todo remove following header
#include "Common/Log.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

////////////////////////////////////////////////////////////////////////////////

/// Component holding a connectivity Buffer
/// The Buffer has to be filled through a buffer.
/// Before using the Buffer one has to be sure that
/// the buffer is flushed.
/// @author Willem Deconinck
template < typename T >
class Mesh_API Buffer {

public:
  
  typedef boost::multi_array<T,2> Array_t;
  
  /// Contructor
  /// @param name of the component
  Buffer (Array_t& array, size_t nbRows);  

  /// Virtual destructor
  virtual ~Buffer();

  /// Get the class name
  static std::string getClassName () { return "Buffer"; }

  // functions specific to the Buffer component
  
  /// Change the buffer to the new size
  void change_buffersize(const size_t nbRows);
  
  /// flush the buffer in the connectivity Buffer
  void flush();
  // 
  /// Add a row to the table if it has empty rows.
  /// It is then added instead of the last removed row.
  /// If it doesn't have empty rows, it is added
  /// to the buffer.
  /// @param 
  template<typename vectorType>
  void add_row(const vectorType& row);
  
  /// remove a row from the table. Rows are only marked as empty.
  void rm_row(const Uint array_idx);
  
  void compact();
  
  template <typename TValue, boost::detail::multi_array::size_type K>
  void swap(boost::detail::multi_array::sub_array<TValue, K> lhs,
            boost::detail::multi_array::sub_array<TValue, K> rhs);
  
  
/// private data
/// @todo change this to private, just for debugging
public:
      
  /// number of rows written in the buffer
  Uint m_nbFilledBufferRows;
  
  /// number of rows allocated in the buffer
  Uint m_nbAllocatedBufferRows;
  
  /// reference to the array that is buffered
  Array_t& m_array;
  
  /// the buffer used to fill the array
  Array_t m_addBuffer;
  
  /// the storage of all empty rows in the array
  std::vector<Uint> m_emptyArrayRows;
  std::vector<Uint> m_emptyBufferRows;
  
  Uint m_nbAllocatedArrayRows;
  Uint m_nbEmptyArrayRows;
  Uint m_nbEmptyBufferRows;
  
  static const T NOVALUE;

}; // end of class ConnectivityTable

//////////////////////////////////////////////////////////////////////////////

template<typename T>
const T Buffer<T>::NOVALUE = std::numeric_limits<T>::max();
  
////////////////////////////////////////////////////////////////////////////////

template<typename T>
  Buffer<T>::Buffer (Buffer<T>::Array_t& array, size_t nbRows) :
  m_nbFilledBufferRows(0),
  m_nbAllocatedBufferRows(nbRows),
  m_array(array),
  m_addBuffer(boost::extents[m_nbAllocatedBufferRows][m_array.shape()[1]]),
  m_nbAllocatedArrayRows(m_array.size()),
  m_nbEmptyArrayRows(0),
  m_nbEmptyBufferRows(0)
{
  m_emptyArrayRows.resize(m_nbAllocatedBufferRows);
  m_emptyBufferRows.resize(m_nbAllocatedBufferRows);
}

////////////////////////////////////////////////////////////////////////////////

template<typename T>
Buffer<T>::~Buffer()
{
  // make sure to flush and compact the table before deleting the buffer
  compact();
}

////////////////////////////////////////////////////////////////////////////////

template<typename T>
void Buffer<T>::flush()
{
  Uint iRow = m_nbAllocatedArrayRows;
  const Uint nbCols=m_array.shape()[1];
  const Uint size_increase = (Uint) std::max((int) 0, (int) (m_nbFilledBufferRows - m_nbEmptyBufferRows - m_nbEmptyArrayRows));
  
  m_nbAllocatedArrayRows += size_increase;
  m_array.resize(boost::extents[m_nbAllocatedArrayRows][nbCols]);
  Uint addBuffer_idx = 0;
  while (addBuffer_idx < m_nbFilledBufferRows)
  {
    // Only add valid buffer rows. This means that empty bufferRows will be skipped
    while (m_addBuffer[addBuffer_idx][0] == NOVALUE)
    {
      addBuffer_idx++;
      m_nbEmptyBufferRows--;
    }
    
    // when there are empty array rows, overwrite them
    if (m_nbEmptyArrayRows)
    {
      Uint row_to_overwrite = m_emptyArrayRows[--m_nbEmptyArrayRows];
      for (Uint j=0; j<nbCols; j++)
        m_array[row_to_overwrite][j] = m_addBuffer[addBuffer_idx][j];
      addBuffer_idx++;
    }
    // otherwise copy in newly allocated part
    else
    {
      for (Uint j=0; j<nbCols; j++)
        m_array[iRow][j] = m_addBuffer[addBuffer_idx][j];
      addBuffer_idx++;
      iRow++;
    }

  }
  m_nbFilledBufferRows = 0;
  cf_assert(iRow == m_nbAllocatedArrayRows);
  
}

//////////////////////////////////////////////////////////////////////////////

template<typename T>
template<typename vectorType>
void Buffer<T>::add_row(const vectorType& row)
{
  const Uint nbCols=m_array.shape()[1];
  cf_assert(row.size() == nbCols);
  
  // 3 cases:
  // 1) There are empty rows in the table --> replace the last empty table row
  if (m_nbEmptyArrayRows)
  {
    const Uint row_to_overwrite = m_emptyArrayRows[--m_nbEmptyArrayRows];
    for(Uint j=0; j<nbCols; ++j)
      m_array[row_to_overwrite][j] = row[j];
  }
  
  // 2) There are empty rows in the buffer --> replace the empty buffer row
  else if (m_nbEmptyBufferRows)
  {
    const Uint row_to_overwrite = m_emptyBufferRows[--m_nbEmptyBufferRows];
    for(Uint j=0; j<nbCols; ++j)
      m_addBuffer[row_to_overwrite][j] = row[j];
  }
  
  // 3) No empty rows in the buffer or the table --> add to buffer as a new row
  else
  {
    for(Uint j=0; j<nbCols; ++j)
      m_addBuffer[m_nbFilledBufferRows][j] = row[j];
    m_nbFilledBufferRows++;
    if (m_nbFilledBufferRows == m_nbAllocatedBufferRows)
      flush();
  }
}

//////////////////////////////////////////////////////////////////////////////

template<typename T>
void Buffer<T>::rm_row(const Uint array_idx)
{
  // two cases:
  // 1) the empty row is in the table
  if (array_idx < m_nbAllocatedArrayRows)
  {
    m_emptyArrayRows[m_nbEmptyArrayRows++] = array_idx;
    if (m_nbEmptyArrayRows == m_nbAllocatedArrayRows)
      flush();
  }
  
  // 2) the empty row is still in the buffer
  else
  {
    m_emptyBufferRows[m_nbEmptyBufferRows++] = m_nbAllocatedArrayRows - array_idx;
    m_addBuffer[array_idx-m_nbAllocatedArrayRows][0] = NOVALUE;
    if (m_nbEmptyBufferRows == m_nbAllocatedBufferRows)
      flush();
    
  }

}

////////////////////////////////////////////////////////////////////////////////

template<typename T>
void Buffer<T>::compact()
{
  flush();
  if (m_nbEmptyArrayRows)
  {
    const Uint new_size = m_nbAllocatedArrayRows - m_nbEmptyArrayRows;
    
    for (Uint i=0; i<m_nbEmptyArrayRows; i++)
      m_array[m_emptyArrayRows[i]][0] = NOVALUE;
    
    Uint m_extraRow = new_size;
    
    for (Uint i=0; i<m_nbEmptyArrayRows; i++)
    {
      // swap only necessary if it is in the smaller-sized array
      if (m_emptyArrayRows[i]<new_size)
      {        
        // swap this row with another VALID one with index above new_size
        while(m_array[m_extraRow][0] == NOVALUE)
          m_extraRow++;          
        swap(m_array[m_emptyArrayRows[i]],m_array[m_extraRow]);
        m_extraRow++;
      }
    }
    // resize
    m_nbAllocatedArrayRows = new_size;
    m_array.resize(boost::extents[m_nbAllocatedArrayRows][m_array.shape()[1]]);
    m_nbEmptyArrayRows = 0;
  }
}

//////////////////////////////////////////////////////////////////////////////

template<typename T>
void Buffer<T>::change_buffersize(const size_t buffersize)
{
  flush();
  m_nbAllocatedBufferRows = buffersize;
  m_addBuffer.resize(boost::extents[m_nbAllocatedBufferRows][m_array.shape()[1]]);
  m_emptyBufferRows.resize(m_nbAllocatedBufferRows);
  m_emptyArrayRows.resize(m_nbAllocatedBufferRows);
}

//////////////////////////////////////////////////////////////////////////////

template<typename T>
template <typename TValue, boost::detail::multi_array::size_type K>
void Buffer<T>::swap(
    boost::detail::multi_array::sub_array<TValue, K> lhs,
    boost::detail::multi_array::sub_array<TValue, K> rhs)
{
  boost::multi_array<TValue, K> tmp = lhs;
  lhs = rhs;
  rhs = tmp;
}


////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_Buffer_hpp
