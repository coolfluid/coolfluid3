// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CGNS_CGNSException_hpp
#define CF_Mesh_CGNS_CGNSException_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Exception.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace CGNS {

////////////////////////////////////////////////////////////////////////////////

/// Exception thrown when CGNS throws an exception.
/// @author Willem Deconinck
struct Mesh_CGNS_API CGNSException : public Common::Exception {

 /// Constructor
 CGNSException (const Common::CodeLocation& where, const std::string& what)
   : Common::Exception(where, what, "CGNSException") {}

}; // end CGNSException

////////////////////////////////////////////////////////////////////////////////

} // CGNS
} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CGNS_CGNSException_hpp

