// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/RegistLibrary.hpp"

#include "GUI/Client/Core/LibClientCore.hpp"

namespace CF {
namespace GUI {
namespace ClientCore {

CF::Common::RegistLibrary<LibClientCore> libClientCore;

////////////////////////////////////////////////////////////////////////////////

void LibClientCore::initiate()
{
  cf_assert( !m_is_initiated );
  m_is_initiated = true;
}

void LibClientCore::terminate()
{
  m_is_initiated = false;
}

////////////////////////////////////////////////////////////////////////////////

} // Client
} // GUI
} // CF
