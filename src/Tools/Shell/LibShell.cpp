// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/RegistLibrary.hpp"

#include "Tools/Shell/LibShell.hpp"

namespace CF {
namespace Tools {
namespace Shell {

CF::Common::RegistLibrary<LibShell> libShell;

////////////////////////////////////////////////////////////////////////////////

void LibShell::initiate_impl()
{
}

void LibShell::terminate_impl()
{
}

////////////////////////////////////////////////////////////////////////////////

} // Shell
} // Tools
} // CF
