// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/RegistLibrary.hpp"

#include "Tools/Prowl/LibProwl.hpp"

namespace CF {
namespace Tools {
namespace Prowl {

CF::Common::RegistLibrary<LibProwl> libProwl;

////////////////////////////////////////////////////////////////////////////////

void LibProwl::initiate_impl()
{
}

void LibProwl::terminate_impl()
{
}

////////////////////////////////////////////////////////////////////////////////

} // Prowl
} // Tools
} // CF
