// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/RegistLibrary.hpp"

#include "UI/Core/LibCore.hpp"

namespace CF {
namespace UI {
namespace Core {

CF::Common::RegistLibrary<LibCore> LibCore;

////////////////////////////////////////////////////////////////////////////////

void LibCore::initiate()
{
  cf_assert( !m_is_initiated );
  m_is_initiated = true;
}

void LibCore::terminate()
{
  m_is_initiated = false;
}

////////////////////////////////////////////////////////////////////////////////

} // Client
} // UI
} // CF
