// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/RegistLibrary.hpp"

#include "Tools/MeshGeneration/LibMeshGeneration.hpp"

namespace cf3 {
namespace Tools {
namespace MeshGeneration {

cf3::common::RegistLibrary<LibMeshGeneration> libMeshGeneration;

////////////////////////////////////////////////////////////////////////////////

void LibMeshGeneration::initiate_impl()
{
}

void LibMeshGeneration::terminate_impl()
{
}

////////////////////////////////////////////////////////////////////////////////

} // LibMeshGeneration
} // Tools
} // cf3
