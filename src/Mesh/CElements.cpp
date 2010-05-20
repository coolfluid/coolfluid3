#include "Common/Log.hpp"
#include "Common/Factory.hpp"

#include "Mesh/CElements.hpp"
#include "Mesh/ElementType.hpp"
#include "Mesh/GeoShape.hpp"

namespace CF {
namespace Mesh {

using namespace Common;

////////////////////////////////////////////////////////////////////////////////

CElements::CElements ( const CName& name  ) :
  Component ( name )
{
  build_component(this);
}

////////////////////////////////////////////////////////////////////////////////

CElements::~CElements()
{
}

////////////////////////////////////////////////////////////////////////////////

void CElements::set_elementType(const std::string& etype_name)
{
  Common::SafePtr< ElementType::PROVIDER > prov =
      Factory<ElementType>::getInstance().getProvider( etype_name );

  m_elementType = prov->create();
}

//////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
