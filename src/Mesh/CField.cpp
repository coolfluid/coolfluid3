#include "Common/ObjectProvider.hpp"

#include "Mesh/MeshAPI.hpp"
#include "Mesh/CField.hpp"

namespace CF {
namespace Mesh {

using namespace Common;

Common::ObjectProvider < CField, Component, MeshLib, NB_ARGS_1 >
CField_Provider ( CField::getClassName() );

////////////////////////////////////////////////////////////////////////////////

CField::CField ( const CName& name  ) :
  Component ( name )
{
}

////////////////////////////////////////////////////////////////////////////////

CField::~CField()
{
}

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
