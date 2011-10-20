// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/RegistLibrary.hpp"

#include "UI/Graphics/LibGraphics.hpp"

namespace cf3 {
namespace UI {
namespace Graphics {

cf3::common::RegistLibrary<LibGraphics> libGraphics;

////////////////////////////////////////////////////////////////////////////////

void LibGraphics::initiate_impl()
{
}

void LibGraphics::terminate_impl()
{
}

////////////////////////////////////////////////////////////////////////////////

} // Client
} // UI
} // cf3
