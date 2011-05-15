// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Core.hpp"

using namespace CF::Common;

int main(int argc, char ** argv)
{
  // initiate the CF core environment
  Core::instance().initiate(argc, argv);

  // terminate the CF core environment
  Core::instance().terminate();

  return 0;
}
