#include "Common/ObjectProvider.hpp"
#include "Common/StreamHelpers.hpp"

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

std::ostream& operator<<(std::ostream& os, const CArray::ConstRow& row)
{
  print_vector(os, row);
  return os;
}


////////////////////////////////////////////////////////////////////////////////
  
} // Mesh
} // CF
