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
#include "UI/Core/NPlugin.hpp"
#include "UI/Core/NPlugins.hpp"
#include "UI/Core/NLog.hpp"
#include "UI/Graphics/TabBuilder.hpp"
#include "UI/Core/CNodeBuilders.hpp"
#include "UI/ParaViewTab/N3DView.hpp"
#include "UI/ParaViewTab/LibParaViewTab.hpp"

using namespace CF::Common;
using namespace CF::UI::Core;
using namespace CF::UI::Graphics;

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {
namespace ParaViewTab {

CF::Common::RegistLibrary<LibParaViewTab> libParaViewTab;

////////////////////////////////////////////////////////////////////////////////

void LibParaViewTab::initiate_impl()
{
  Common::Core & core = Common::Core::instance();
  m_argc = core.argc();

  m_appCore = new pqApplicationCore(m_argc, core.argv());

  NPlugins::globalPlugins()->registerPlugin<LibParaViewTab>();
  CNodeBuilders::instance().registerBuilder<N3DView>("CF.UI.ParaView.C3DView");
}

void LibParaViewTab::terminate_impl()
{
  delete m_appCore;
}

////////////////////////////////////////////////////////////////////////////////

} // ParaViewTab
} // UI
} // CF
