// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Mesh/ElementType2.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

////////////////////////////////////////////////////////////////////////////////

ElementType2::ElementType2( const std::string& name ) : Common::Component(name)
{
   
}

ElementType2::~ElementType2()
{
}

////////////////////////////////////////////////////////////////////////////////

// void ElementType2::compute_normal(const NodesT& coord, RealVector& normal) const
// { 
//   throw Common::NotImplemented(FromHere(),"compute_normal not implemented for "+element_type_name()); 
// }
// 
// ////////////////////////////////////////////////////////////////////////////////
// 
// void ElementType2::compute_centroid(const NodesT& coord , RealVector& centroid) const
// { 
//   throw Common::NotImplemented(FromHere(),"compute_centroid not implemented for "+element_type_name()); 
// }

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
