// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/RegistLibrary.hpp"

#include "GUI/UICommon/LibUICommon.hpp"

namespace CF {
namespace GUI {
namespace UICommon {

CF::Common::RegistLibrary<LibUICommon> LibUICommon;

////////////////////////////////////////////////////////////////////////////////

void LibUICommon::initiate()
{
  cf_assert( !m_is_initiated );
  m_is_initiated = true;
}

void LibUICommon::terminate()
{
  m_is_initiated = false;
}

////////////////////////////////////////////////////////////////////////////////

} // Network
} // GUI
} // CF
