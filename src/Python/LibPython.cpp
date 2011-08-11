// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/RegistLibrary.hpp"

#include "Python/LibPython.hpp"

namespace CF {
namespace Python {

CF::Common::RegistLibrary<LibPython> libPython;

////////////////////////////////////////////////////////////////////////////////

void LibPython::initiate_impl()
{
}

void LibPython::terminate_impl()
{
}

////////////////////////////////////////////////////////////////////////////////

} // Python
} // CF
