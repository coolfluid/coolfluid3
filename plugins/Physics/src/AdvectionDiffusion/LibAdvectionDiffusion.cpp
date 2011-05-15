// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/RegistLibrary.hpp"

#include "AdvectionDiffusion/LibAdvectionDiffusion.hpp"

namespace CF {
namespace AdvectionDiffusion {

  using namespace Common;

CF::Common::RegistLibrary<LibAdvectionDiffusion> LibAdvectionDiffusion;

////////////////////////////////////////////////////////////////////////////////

void LibAdvectionDiffusion::initiate_impl()
{
}

void LibAdvectionDiffusion::terminate_impl()
{
}

////////////////////////////////////////////////////////////////////////////////

} // AdvectionDiffusion
} // CF
