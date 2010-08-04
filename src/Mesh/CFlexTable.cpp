#include "Common/ObjectProvider.hpp"

#include "Mesh/MeshAPI.hpp"
#include "Mesh/CFlexTable.hpp"

namespace CF {
namespace Mesh {

using namespace Common;

Common::ObjectProvider < CFlexTable, Component, MeshLib, NB_ARGS_1 >
CFlexTable_Provider ( CFlexTable::type_name() );

////////////////////////////////////////////////////////////////////////////////

CFlexTable::CFlexTable ( const CName& name  ) :
  Component ( name )
{
  BUILD_COMPONENT;
}

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
