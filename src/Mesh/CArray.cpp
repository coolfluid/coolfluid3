#include "Common/ObjectProvider.hpp"

#include "Mesh/MeshAPI.hpp"
#include "Mesh/CArray.hpp"

namespace CF {
namespace Mesh {

using namespace Common;

Common::ObjectProvider < CArray, Component, MeshLib, NB_ARGS_1 >
CArray_Provider ( CArray::type_name() );

////////////////////////////////////////////////////////////////////////////////

CArray::CArray ( const CName& name  ) :
  Component ( name )
{
  BUILD_COMPONENT;
}

////////////////////////////////////////////////////////////////////////////////
  
} // Mesh
} // CF
