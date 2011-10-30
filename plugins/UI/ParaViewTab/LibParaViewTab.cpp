// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

// ParaView headers
#include <pqApplicationCore.h>

// CF headers
#include "common/Core.hpp"
#include "common/RegistLibrary.hpp"
#include "UI/Core/NPlugin.hpp"
#include "UI/Core/NPlugins.hpp"
#include "UI/Core/NLog.hpp"
#include "UI/Graphics/TabBuilder.hpp"
#include "UI/Core/CNodeBuilders.hpp"
#include "UI/ParaViewTab/N3DView.hpp"
#include "UI/ParaViewTab/LibParaViewTab.hpp"

using namespace cf3::common;
using namespace cf3::UI::Core;
using namespace cf3::UI::Graphics;

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace UI {
namespace ParaViewTab {

cf3::common::RegistLibrary<LibParaViewTab> libParaViewTab;

////////////////////////////////////////////////////////////////////////////////

LibParaViewTab::~LibParaViewTab()
{
  if(m_is_initiated)
    terminate_impl();
}

void LibParaViewTab::initiate()
{
  if(m_is_initiated)
    return;
  
  initiate_impl();
  m_is_initiated = true;
}

void LibParaViewTab::terminate()
{
  if(!m_is_initiated)
    return;
  
  terminate_impl();
  m_is_initiated = false;
}


void LibParaViewTab::initiate_impl()
{
  common::Core & core = common::Core::instance();
  m_argc = core.argc();

  m_appCore = new pqApplicationCore(m_argc, core.argv());

  NPlugins::global()->register_plugin<LibParaViewTab>();
  CNodeBuilders::instance().register_builder<N3DView>("cf3.UI.ParaView.C3DView");
}

void LibParaViewTab::terminate_impl()
{
  delete m_appCore;
}

////////////////////////////////////////////////////////////////////////////////

} // ParaViewTab
} // UI
} // CF
