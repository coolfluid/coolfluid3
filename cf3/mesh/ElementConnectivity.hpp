// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_ElementConnectivity_hpp
#define cf3_mesh_ElementConnectivity_hpp

#include "common/Table_fwd.hpp"
#include "mesh/Entities.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {


////////////////////////////////////////////////////////////////////////////////

typedef common::Table<Entity> ElementConnectivity;

std::ostream& operator<<(std::ostream& os, common::TableConstRow<Entity> row);
std::ostream& operator<<(std::ostream& os, const ElementConnectivity& table);

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_ElementConnectivity_hpp
