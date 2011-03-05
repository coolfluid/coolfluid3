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

CF::Common::RegistLibrary<LibShell> libCommandLineInterpreter;

////////////////////////////////////////////////////////////////////////////////

void LibShell::initiate()
{
}

void LibShell::terminate()
{
}

////////////////////////////////////////////////////////////////////////////////

} // Shell
} // Tools
} // CF
