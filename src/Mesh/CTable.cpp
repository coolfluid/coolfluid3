#include "Common/ObjectProvider.hpp"

#include "Mesh/MeshAPI.hpp"
#include "Mesh/CTable.hpp"

namespace CF {
namespace Mesh {

using namespace Common;

Common::ObjectProvider < CTable, Component, MeshLib, NB_ARGS_1 >
CTable_Provider ( CTable::type_name() );

////////////////////////////////////////////////////////////////////////////////

CTable::CTable ( const CName& name  ) :
  Component ( name )
{
  BUILD_COMPONENT;
}

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
