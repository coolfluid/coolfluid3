#include "Common/ObjectProvider.hpp"

#include "Mesh/MeshAPI.hpp"
#include "Mesh/CTable.hpp"

namespace CF {
namespace Mesh {

using namespace Common;

Common::ObjectProvider < CTable, Component, MeshLib, NB_ARGS_1 >
CTable_Provider ( CTable::getClassName() );

////////////////////////////////////////////////////////////////////////////////

CTable::CTable ( const CName& name  ) :
  Component ( name ),
  m_table(boost::extents[0][0])
{
  build_component(this);
}

////////////////////////////////////////////////////////////////////////////////

CTable::~CTable()
{
  m_table.resize(boost::extents[0][0]);
}

//////////////////////////////////////////////////////////////////////////////

void CTable::initialize(const Uint nbCols) 
{
  m_table.resize(boost::extents[0][nbCols]); 
}

//////////////////////////////////////////////////////////////////////////////

CTable::Buffer CTable::create_buffer(const size_t buffersize)
{ 
  // make sure the connectivity table has its columnsize defined
  cf_assert(m_table.shape()[1] > 0);
  return CTable::Buffer(m_table,buffersize); 
}

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
