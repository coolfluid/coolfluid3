#include <boost/assign/list_of.hpp> // for map_list_of

#include "Mesh/GeoShape.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

////////////////////////////////////////////////////////////////////////////////

GeoShape::Convert::FwdMap_t GeoShape::Convert::all_fwd = boost::assign::map_list_of
    ( GeoShape::INVALID, "Invalid" )
    ( GeoShape::POINT,   "Point"   )
    ( GeoShape::LINE,    "Line"    )
    ( GeoShape::TRIAG,   "Triag"   )
    ( GeoShape::QUAD,    "Quad"    )
    ( GeoShape::TETRA,   "Tetra"   )
    ( GeoShape::PYRAM,   "Pyram"   )
    ( GeoShape::PRISM,   "Prism"   )
    ( GeoShape::HEXA,    "Hexa"    );

GeoShape::Convert::BwdMap_t GeoShape::Convert::all_rev = boost::assign::map_list_of
    ("Invalid",  GeoShape::INVALID )
    ("Point",    GeoShape::POINT   )
    ("Line",     GeoShape::LINE    )
    ("Triag",    GeoShape::TRIAG   )
    ("Quad",     GeoShape::QUAD    )
    ("Tetra",    GeoShape::TETRA   )
    ("Pyram",    GeoShape::PYRAM   )
    ("Prism",    GeoShape::PRISM   )
    ("Hexa",     GeoShape::HEXA    );

////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<< ( std::ostream& os, const GeoShape::Type& in )
{
  os << GeoShape::Convert::to_str(in);
  return os;
}

std::istream& operator>> (std::istream& is, GeoShape::Type& in )
{
  std::string tmp;
  is >> tmp;
  in = GeoShape::Convert::to_enum(tmp);
  return is;
}

////////////////////////////////////////////////////////////////////////////////

} // namespace Mesh
} // namespace CF

////////////////////////////////////////////////////////////////////////////////
