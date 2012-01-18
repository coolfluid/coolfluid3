// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"

#include "SFDM/ShapeFunction.hpp"

namespace cf3 {
namespace SFDM {

////////////////////////////////////////////////////////////////////////////////

ShapeFunction::ShapeFunction(const std::string& name) : mesh::ShapeFunction(name)
{
}

////////////////////////////////////////////////////////////////////////////////

} // SFDM
} // cf3
