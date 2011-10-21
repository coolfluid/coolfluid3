// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/RegistLibrary.hpp"

#include "mesh/Neu/LibNeu.hpp"

namespace cf3 {
namespace mesh {
namespace Neu {

cf3::common::RegistLibrary<LibNeu> libNeu;

////////////////////////////////////////////////////////////////////////////////

void LibNeu::initiate_impl()
{
}

void LibNeu::terminate_impl()
{
}

////////////////////////////////////////////////////////////////////////////////

} // Neu
} // mesh
} // cf3
