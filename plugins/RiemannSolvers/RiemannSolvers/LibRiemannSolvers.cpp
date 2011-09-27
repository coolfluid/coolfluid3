// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/RegistLibrary.hpp"
#include "RiemannSolvers/LibRiemannSolvers.hpp"

namespace CF {
namespace RiemannSolvers {

  using namespace Common;

CF::Common::RegistLibrary<LibRiemannSolvers> libRiemannSolvers;

////////////////////////////////////////////////////////////////////////////////

void LibRiemannSolvers::initiate_impl()
{
}

void LibRiemannSolvers::terminate_impl()
{
}

////////////////////////////////////////////////////////////////////////////////

} // RiemannSolvers
} // CF
