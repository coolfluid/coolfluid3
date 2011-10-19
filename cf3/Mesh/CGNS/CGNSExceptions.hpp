// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Mesh_CGNS_CGNSException_hpp
#define cf3_Mesh_CGNS_CGNSException_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Exception.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace Mesh {
namespace CGNS {

////////////////////////////////////////////////////////////////////////////////

/// Exception thrown when CGNS throws an exception.
/// @author Willem Deconinck
struct Mesh_CGNS_API CGNSException : public common::Exception {

 /// Constructor
 CGNSException (const common::CodeLocation& where, const std::string& what)
   : common::Exception(where, what, "CGNSException") {}

 virtual ~CGNSException() throw() {}
 
}; // end CGNSException

////////////////////////////////////////////////////////////////////////////////

} // CGNS
} // Mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_Mesh_CGNS_CGNSException_hpp

