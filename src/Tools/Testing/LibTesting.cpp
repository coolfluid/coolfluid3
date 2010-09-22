// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CoreEnv.hpp"

#include "Tools/Testing/LibTesting.hpp"

namespace CF {
namespace Tools {
namespace Testing {

  CF::Common::ForceLibRegist<LibTesting> libTesting;

////////////////////////////////////////////////////////////////////////////////

void LibTesting::initiate()
{
}

void LibTesting::terminate()
{
}

////////////////////////////////////////////////////////////////////////////////

} // Testing
} // Tools
} // CF
