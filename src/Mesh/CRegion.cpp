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

CElements& CRegion::create_elements(const std::string& element_type_name, CArray::Ptr coordinates)
{
  std::string name = "elements_" + element_type_name;
  CElements::Ptr elements = create_component_type<CElements>(name);

  // if no argument is passed for coordinates, take the coordinates defined in this region.
  // throws error if not found.
  if (!coordinates.get())
  {
    coordinates = get_child_type<CArray>("coordinates");
    if (!coordinates.get())
      throw ValueNotFound(FromHere(), "Component with name 'coordinates' does not exist in component '"
                                      + this->name() + "' with path ["
                                      + m_path.string() + "]");
  }
  
  elements->initialize(element_type_name,coordinates);
  return *elements;
}

//////////////////////////////////////////////////////////////////////////////

CArray& CRegion::create_coordinates(const Uint& dim)
{
  CArray::Ptr coordinates = create_component_type<CArray>("coordinates");
  coordinates->initialize(dim);
  return *coordinates;
}
  
//////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
