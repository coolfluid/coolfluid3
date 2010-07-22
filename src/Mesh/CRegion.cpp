#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>

#include "Mesh/CRegion.hpp"

namespace CF {
namespace Mesh {

using namespace Common;

////////////////////////////////////////////////////////////////////////////////

CRegion::CRegion ( const CName& name  ) :
  Component ( name )
{
  BUILD_COMPONENT;
}

////////////////////////////////////////////////////////////////////////////////

CRegion::~CRegion()
{
}

////////////////////////////////////////////////////////////////////////////////

CRegion& CRegion::create_region( const CName& name )
{
  return *create_component_type<CRegion>(name);
}

////////////////////////////////////////////////////////////////////////////////

CElements& CRegion::create_elements(const std::string& element_type_name )
{
  std::string name = "elements_" + element_type_name;
  CElements::Ptr elements = create_component_type<CElements>(name);
  elements->initialize(element_type_name);
  return *elements;
}


//////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
