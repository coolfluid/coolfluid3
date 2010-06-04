#include "Common/ObjectProvider.hpp"

#include "Mesh/MeshAPI.hpp"
#include "Mesh/CArray.hpp"

namespace CF {
namespace Mesh {

using namespace Common;

Common::ObjectProvider < CArray, Component, MeshLib, NB_ARGS_1 >
CArray_Provider ( CArray::getClassName() );

////////////////////////////////////////////////////////////////////////////////

CArray::CArray ( const CName& name  ) :
  Component ( name ),
  m_array(boost::extents[0][0])
{
  BUILD_COMPONENT;
}

////////////////////////////////////////////////////////////////////////////////

CArray::~CArray()
{
}

//////////////////////////////////////////////////////////////////////////////

void CArray::initialize(const Uint nbCols)
{
  m_array.resize(boost::extents[0][nbCols]); 
}

////////////////////////////////////////////////////////////////////////////////

CArray::Buffer CArray::create_buffer(const size_t buffersize)
{ 
  // make sure the array has its columnsize defined
  cf_assert(m_array.shape()[1] > 0);
  return CArray::Buffer(m_array,buffersize); 
}

////////////////////////////////////////////////////////////////////////////////
  
} // Mesh
} // CF
