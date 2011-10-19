// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/RegistLibrary.hpp"

#include "UI/Core/LibCore.hpp"

namespace cf3 {
namespace UI {
namespace Core {

cf3::common::RegistLibrary<LibCore> LibCore;

////////////////////////////////////////////////////////////////////////////////

void LibCore::initiate_impl()
{
}

void LibCore::terminate_impl()
{
}

////////////////////////////////////////////////////////////////////////////////

} // Client
} // UI
} // cf3
