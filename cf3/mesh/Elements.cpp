// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <set>

#include "common/Log.hpp"
#include "common/Link.hpp"
#include "common/Group.hpp"
#include "common/Builder.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"

#include "mesh/Elements.hpp"
#include "mesh/Connectivity.hpp"
#include "common/List.hpp"
#include "mesh/ElementData.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Field.hpp"
#include "mesh/Space.hpp"

namespace cf3 {
namespace mesh {

using namespace common;

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < Elements, Entities, LibMesh > Elements_Builder;

////////////////////////////////////////////////////////////////////////////////

Elements::Elements ( const std::string& name ) :
  Entities ( name )
{
  properties()["brief"] = std::string("Holds information of elements of one type");
  properties()["description"] = std::string("Container component that stores the element to node connectivity,\n")
  +std::string("a link to node storage, a list of used nodes, and global numbering unique over all processors");
}

////////////////////////////////////////////////////////////////////////////////

Elements::~Elements()
{
}

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3
