#ifndef CF_Mesh_Buffer_hpp
#define CF_Mesh_Buffer_hpp

////////////////////////////////////////////////////////////////////////////////

#define BOOST_MULTI_ARRAY_NO_GENERATORS false
#include "boost/multi_array.hpp" 
#undef BOOST_MULTI_ARRAY_NO_GENERATORS

#include "Mesh/MeshAPI.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

////////////////////////////////////////////////////////////////////////////////

/// Component holding a connectivity Buffer
/// The Buffer has to be filled through a buffer.
/// Before using the Buffer one has to be sure that
/// the buffer is flushed.
/// @author Willem Deconinck
template < typename Array_t >
class Mesh_API Buffer {

public:
    
  /// Contructor
  /// @param name of the component
  Buffer (Array_t& array, size_t nbRows);
  
  void initialize();
  

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
  /// add a row to the buffer. If the buffer is full, 
  /// it is flushed into the array
  template<typename vectorType>
  void add_row(const vectorType& row);
  
/// private data
private:
      
  /// number of rows written in the buffer
  Uint m_nbFilledRows;
  
  /// number of rows allocated in the buffer
  Uint m_nbAllocatedRows;
  
  /// reference to the array that is buffered
  Array_t& m_array;
  
  /// the buffer used to fill the array
  Array_t m_buffer;
    
};

////////////////////////////////////////////////////////////////////////////////

template<typename Array_t>
Buffer<Array_t>::Buffer (Array_t& array, size_t nbRows) :
  m_nbFilledRows(0),
  m_nbAllocatedRows(nbRows),
  m_array(array),
  m_buffer(boost::extents[m_nbAllocatedRows][m_array.shape()[1]])
{
}
  
////////////////////////////////////////////////////////////////////////////////

template<typename Array_t>
void Buffer<Array_t>::initialize ()
{
  m_nbFilledRows=0;
  m_buffer.resize(boost::extents[m_nbAllocatedRows][m_array.shape()[1]]);
}

////////////////////////////////////////////////////////////////////////////////

template<typename Array_t>
Buffer<Array_t>::~Buffer()
{
  // make sure to flush before deleting the buffer
  flush();
}

////////////////////////////////////////////////////////////////////////////////

template<typename Array_t>
void Buffer<Array_t>::flush()
{
  const Uint nRows=m_array.shape()[0];
  const Uint nCols=m_array.shape()[1];
  m_array.resize(boost::extents[nRows+m_nbFilledRows][nCols]);
  for (Uint i=0; i<m_nbFilledRows; ++i)
    for (Uint j=0; j<nCols; ++j)
  {
    m_array[nRows+i][j] = m_buffer[i][j];
  }
  
  m_nbFilledRows = 0;
}

//////////////////////////////////////////////////////////////////////////////

template<typename Array_t>
template<typename vectorType>
void Buffer<Array_t>::add_row(const vectorType& row)
{
  const Uint nbCols=m_array.shape()[1];
  cf_assert(row.size() == nbCols);
  
  for(Uint j=0; j<nbCols; ++j)
    m_buffer[m_nbFilledRows][j] = row[j];
  
  m_nbFilledRows++;
  
  if (m_nbFilledRows == m_buffer.size())
    flush();
}
  
//////////////////////////////////////////////////////////////////////////////

template<typename Array_t>
void Buffer<Array_t>::change_buffersize(const size_t buffersize)
{
  flush();
  m_nbAllocatedRows = buffersize;
  m_buffer.resize(boost::extents[m_nbAllocatedRows][m_array.shape()[1]]);
}

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_Buffer_hpp
