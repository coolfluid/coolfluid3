#include "Mesh/ConnectivityTable.hpp"

namespace CF {
namespace Mesh {

using namespace Common;

////////////////////////////////////////////////////////////////////////////////

ConnectivityTable::ConnectivityTable () :
  boost::multi_array<Uint,2>(boost::extents[0][0]),
  m_nbRows(0),
  m_nbCols(0),
  m_buffersize(0),
  m_maxBuffersize(0),
  m_buffer(boost::extents[m_maxBuffersize][m_nbCols])
{
}

////////////////////////////////////////////////////////////////////////////////

ConnectivityTable::~ConnectivityTable()
{
}

////////////////////////////////////////////////////////////////////////////////

void ConnectivityTable::flush()
{
  this->resize(boost::extents[m_nbRows+m_buffersize][m_nbCols]);
  for (Uint i=0; i<m_buffersize; ++i)
    for (Uint j=0; j<m_nbCols; ++j)
  {
    (*this)[m_nbRows+i][j] = m_buffer[i][j];
  }
  
  m_nbRows += m_buffersize;
  m_buffersize = 0;
}

//////////////////////////////////////////////////////////////////////////////

void ConnectivityTable::initialize(const Uint& cols, const Uint& buffersize)
{
  m_maxBuffersize = buffersize;
  m_nbCols = cols;
  m_nbRows = 0;
  m_buffersize = 0;
  m_buffer.resize(boost::extents[m_maxBuffersize][m_nbCols]);
}

//////////////////////////////////////////////////////////////////////////////

void ConnectivityTable::clear()
{
  m_nbRows = 0;
  this->resize(boost::extents[m_nbRows][m_nbCols]);
  m_buffersize = 0;
  m_buffer.resize(boost::extents[m_buffersize][m_nbCols]);
}

//////////////////////////////////////////////////////////////////////////////

void ConnectivityTable::add_row(const std::vector<Uint>& row)
{
  cf_assert(row.size() == m_nbCols);
  
  for(Uint j=0; j<m_nbCols; ++j)
    m_buffer[m_buffersize][j] = row[j];
  
  m_buffersize++;
  
  
  if (m_buffersize == m_maxBuffersize)
    flush();
}
  
//////////////////////////////////////////////////////////////////////////////

void ConnectivityTable::set_row(Uint iRow, std::vector<Uint>& row) const
{
  cf_assert(row.size() <= m_nbCols);
  for (Uint jCol = 0; jCol < m_nbCols; ++jCol) {
    row[jCol] = (*this)[iRow][jCol];
  }
}

//////////////////////////////////////////////////////////////////////////////

void ConnectivityTable::change_buffersize(const Uint& buffersize)
{
  if (buffersize != m_maxBuffersize) {
    flush();
    m_maxBuffersize = buffersize;
    m_buffer.resize(boost::extents[m_buffersize][m_nbCols]);
  }

}

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
