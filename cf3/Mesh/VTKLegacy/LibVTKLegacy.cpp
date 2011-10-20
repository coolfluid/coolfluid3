// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/RegistLibrary.hpp"

#include "Mesh/VTKLegacy/LibVTKLegacy.hpp"

namespace cf3 {
namespace Mesh {
namespace VTKLegacy {

cf3::common::RegistLibrary<LibVTKLegacy> libVTKLegacy;

////////////////////////////////////////////////////////////////////////////////

void LibVTKLegacy::initiate_impl()
{
}

void LibVTKLegacy::terminate_impl()
{
}

////////////////////////////////////////////////////////////////////////////////

} // VTKLegacy
} // Mesh
} // cf3
