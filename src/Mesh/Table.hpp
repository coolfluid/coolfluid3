#ifndef CF_Mesh_Table_HH
#define CF_Mesh_Table_HH

////////////////////////////////////////////////////////////////////////////////

#define BOOST_MULTI_ARRAY_NO_GENERATORS false
#include "boost/multi_array.hpp" 
#undef BOOST_MULTI_ARRAY_NO_GENERATORS

#include "Mesh/MeshAPI.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

////////////////////////////////////////////////////////////////////////////////

/// Component holding a connectivity table
/// The table has to be filled through a buffer.
/// Before using the table one has to be sure that
/// the buffer is flushed.
/// @author Willem Deconinck Tiago Quintino
template < typename T >
class Mesh_API Table : public boost::multi_array<T, 2> {

public:
  
  /// a subarray of this table is a row.
  typedef typename Table<T>::template subarray<1>::type Row;
  
  /// Contructor
  /// @param name of the component
  Table ();

  /// Virtual destructor
  virtual ~Table();

  /// Get the class name
  static std::string getClassName () { return "Table"; }

  // functions specific to the Table component

  /// Initialize the connectivity table
  /// This will set the column size and allocate a buffer
  void initialize(const Uint& cols, const Uint& buffersize);

  /// Finalize the table
  /// This will deallocate the memory of the buffer
  void finalize();

  /// Change the buffer to the new size
  void change_buffersize(const Uint& buffersize);
  
  /// Clear the table
  void clear();
  
  /// flush the buffer in the connectivity table
  void flush();
  
  /// add a row to the buffer. If the buffer is full, 
  /// it is flushed into the connectivity table
  template<typename vectorType>
  void add_row(const vectorType& row);
  
  /// Set the row to the given values from the table
  template<typename vectorType>
  void set_row(Uint iRow, vectorType& row) const;
    
  /// Get the number of rows
  Uint nbRows() const {return this->size(); }

  /// Get the number of columns
  Uint nbCols() const {return m_nbCols; }
  
  Row get_row(const Uint idx) { return (*this)[idx]; }
  
/// private data
private:
    
  /// number of columns is fixed by user
  Uint m_nbCols;
  
  /// number of filled rows in the buffer
  Uint m_buffersize;
  
  /// number of rows allocated in the buffer
  Uint m_maxBuffersize;
  
  /// the buffer to be written in the connectivity table
  boost::multi_array<T,2> m_buffer;
  
};

////////////////////////////////////////////////////////////////////////////////

template<typename T>
Table<T>::Table () :
  boost::multi_array<T,2>(boost::extents[0][0]),
  m_nbCols(0),
  m_buffersize(0),
  m_maxBuffersize(0),
  m_buffer(boost::extents[m_maxBuffersize][m_nbCols])
{
}

////////////////////////////////////////////////////////////////////////////////

template<typename T>
Table<T>::~Table()
{
}

////////////////////////////////////////////////////////////////////////////////

template<typename T>
void Table<T>::flush()
{
  Uint nRows=nbRows();
  this->resize(boost::extents[nRows+m_buffersize][m_nbCols]);
  for (Uint i=0; i<m_buffersize; ++i)
    for (Uint j=0; j<m_nbCols; ++j)
  {
    (*this)[nRows+i][j] = m_buffer[i][j];
  }
  
  m_buffersize = 0;
}

//////////////////////////////////////////////////////////////////////////////

template<typename T>
void Table<T>::initialize(const Uint& cols, const Uint& buffersize)
{
  this->resize(boost::extents[0][m_nbCols]);
  m_maxBuffersize = buffersize;
  m_nbCols = cols;
  m_buffersize = 0;
  m_buffer.resize(boost::extents[m_maxBuffersize][m_nbCols]);
}
  
//////////////////////////////////////////////////////////////////////////////

template<typename T>
void Table<T>::finalize()
{
  m_maxBuffersize = 0;
  m_buffersize = 0;
  m_buffer.resize(boost::extents[0][0]);
}

//////////////////////////////////////////////////////////////////////////////

template<typename T>
void Table<T>::clear()
{
  this->resize(boost::extents[0][m_nbCols]);
  m_buffersize = 0;
  m_buffer.resize(boost::extents[m_buffersize][m_nbCols]);
}

//////////////////////////////////////////////////////////////////////////////

template<typename T>
template<typename vectorType>
void Table<T>::add_row(const vectorType& row)
{
  cf_assert(row.size() == m_nbCols);
    
  for(Uint j=0; j<m_nbCols; ++j)
    m_buffer[m_buffersize][j] = row[j];
  
  m_buffersize++;
  
  if (m_buffersize == m_maxBuffersize)
    flush();
}
  
//////////////////////////////////////////////////////////////////////////////

template<typename T>
template<typename vectorType>
void Table<T>::set_row(Uint iRow, vectorType& row) const
{
  cf_assert(row.size() <= m_nbCols);
  for (Uint jCol = 0; jCol < m_nbCols; ++jCol) {
    row[jCol] = (*this)[iRow][jCol];
  }
}

//////////////////////////////////////////////////////////////////////////////

template<typename T>
void Table<T>::change_buffersize(const Uint& buffersize)
{
  if (buffersize != m_maxBuffersize) {
    flush();
    m_maxBuffersize = buffersize;
    m_buffer.resize(boost::extents[m_maxBuffersize][m_nbCols]);
  }

}

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_Table_HH
