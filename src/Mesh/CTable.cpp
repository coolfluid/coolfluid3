#include "Mesh/CTable.hpp"

namespace CF {
namespace Mesh {

using namespace Common;

////////////////////////////////////////////////////////////////////////////////

CTable::CTable ( const CName& name  ) :
  Component ( name ),
  m_nbRows(0),
  m_nbCols(0),
  m_table(boost::extents[m_nbRows][m_nbCols]),
  m_buffersize(0),
  m_maxBuffersize(0),
  m_buffer(boost::extents[m_maxBuffersize][m_nbCols])
{
}

////////////////////////////////////////////////////////////////////////////////

CTable::~CTable()
{
}

////////////////////////////////////////////////////////////////////////////////

void CTable::flush()
{
  m_table.resize(boost::extents[m_nbRows+m_buffersize][m_nbCols]);
  for (Uint i=0; i<m_buffersize; ++i)
    for (Uint j=0; j<m_nbCols; ++j)
  {
    m_table[m_nbRows+i][j] = m_buffer[i][j];
  }
  
  m_nbRows += m_buffersize;
  m_buffersize = 0;
}

//////////////////////////////////////////////////////////////////////////////

void CTable::initialize(const Uint& cols, const Uint& buffersize)
{
  m_maxBuffersize = buffersize;
  m_nbCols = cols;
  m_nbRows = 0;
  m_buffersize = 0;
  m_buffer.resize(boost::extents[m_maxBuffersize][m_nbCols]);
}

//////////////////////////////////////////////////////////////////////////////

void CTable::clear()
{
  m_nbRows = 0;
  m_table.resize(boost::extents[m_nbRows][m_nbCols]);
  m_buffersize = 0;
  m_buffer.resize(boost::extents[m_buffersize][m_nbCols]);
}

//////////////////////////////////////////////////////////////////////////////

void CTable::add_row(const std::vector<Uint>& row)
{
  cf_assert(row.size() == m_nbCols);
  
  for(Uint j=0; j<m_nbCols; ++j)
    m_buffer[m_buffersize][j] = row[j];
  
  m_buffersize++;
  
  
  if (m_buffersize == m_maxBuffersize)
    flush();
}
  
//////////////////////////////////////////////////////////////////////////////

void CTable::set_row(Uint iRow, std::vector<Uint>& row) const
{
  cf_assert(row.size() <= m_nbCols);
  for (Uint jCol = 0; jCol < m_nbCols; ++jCol) {
    row[jCol] = m_table[iRow][jCol];
  }
}

//////////////////////////////////////////////////////////////////////////////

void CTable::change_buffersize(const Uint& buffersize)
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
