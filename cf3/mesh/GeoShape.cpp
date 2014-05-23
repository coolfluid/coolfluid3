// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iostream>

#include <boost/assign/list_of.hpp> // for map_list_of

#include "mesh/GeoShape.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {

////////////////////////////////////////////////////////////////////////////////

GeoShape::Convert& GeoShape::Convert::instance()
{
  static GeoShape::Convert instance;
  return instance;
}

GeoShape::Convert::Convert()
{
  all_fwd = boost::assign::map_list_of
    ( GeoShape::INVALID, "Invalid" )
    ( GeoShape::POINT,   "Point"   )
    ( GeoShape::LINE,    "Line"    )
    ( GeoShape::TRIAG,   "Triag"   )
    ( GeoShape::QUAD,    "Quad"    )
    ( GeoShape::TETRA,   "Tetra"   )
    ( GeoShape::PYRAM,   "Pyram"   )
    ( GeoShape::PRISM,   "Prism"   )
    ( GeoShape::HEXA,    "Hexa"    );

  all_rev = boost::assign::map_list_of
    ("Invalid",  GeoShape::INVALID )
    ("Point",    GeoShape::POINT   )
    ("Line",     GeoShape::LINE    )
    ("Triag",    GeoShape::TRIAG   )
    ("Quad",     GeoShape::QUAD    )
    ("Tetra",    GeoShape::TETRA   )
    ("Pyram",    GeoShape::PYRAM   )
    ("Prism",    GeoShape::PRISM   )
    ("Hexa",     GeoShape::HEXA    );
}

////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<< ( std::ostream& os, const GeoShape::Type& in )
{
  os << GeoShape::Convert::instance().to_str(in);
  return os;
}

std::istream& operator>> (std::istream& is, GeoShape::Type& in )
{
  std::string tmp;
  is >> tmp;
  in = GeoShape::Convert::instance().to_enum(tmp);
  return is;
}

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////
