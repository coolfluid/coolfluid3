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

CElements& CRegion::create_elements(const std::string& element_type_name, CArray& coordinates)
{
  std::string name = "elements_" + element_type_name;
  CElements& elements = *create_component_type<CElements>(name);

  elements.initialize(element_type_name,coordinates);
  return elements;
}

//////////////////////////////////////////////////////////////////////////////

CArray& CRegion::create_coordinates(const Uint& dim)
{
  CArray& coordinates = *create_component_type<CArray>("coordinates");
  coordinates.initialize(dim);
  return coordinates;
}
  
//////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
