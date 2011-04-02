// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

// ParaView headers
#include <pqApplicationCore.h>

// CF headers
#include "Common/Core.hpp"
#include "Common/RegistLibrary.hpp"

#include "UI/Graphics/TabBuilder.hpp"

#include "UI/ParaView/Widget3D.hpp"
#include "UI/ParaView/LibParaView.hpp"

using namespace CF::Common;
using namespace CF::UI::Graphics;

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {
namespace ParaView {

CF::Common::RegistLibrary<LibParaView> libParaView;

////////////////////////////////////////////////////////////////////////////////

void LibParaView::initiate_impl()
{
  Core & core = Core::instance();
  m_argc = core.argc();

  m_appCore = new pqApplicationCore(m_argc, core.argv());

  m_tabIndex = TabBuilder::instance()->addTab(new Widget3D(), "3D-View");
}

void LibParaView::terminate_impl()
{
  delete m_appCore;
}

////////////////////////////////////////////////////////////////////////////////

} // Client
} // UI
} // CF
