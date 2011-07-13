// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "DynamicModel.hpp"

namespace CF {
namespace Physics {

using namespace Common;

////////////////////////////////////////////////////////////////////////////////

DynamicModel::DynamicModel( const std::string& name ) : Physics::PhysModel(name)
{
}

DynamicModel::~DynamicModel()
{
}



////////////////////////////////////////////////////////////////////////////////

} // Physics
} // CF
