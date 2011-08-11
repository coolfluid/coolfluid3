// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/RegistLibrary.hpp"

#include "Tools/Testing/LibTesting.hpp"

using namespace CF::Common;

namespace CF {
namespace Tools {
namespace Testing {

CF::Common::RegistLibrary<LibTesting> libTesting;

////////////////////////////////////////////////////////////////////////////////

void LibTesting::initiate_impl()
{
}

void LibTesting::terminate_impl()
{
}

////////////////////////////////////////////////////////////////////////////////

} // Testing
} // Tools
} // CF
