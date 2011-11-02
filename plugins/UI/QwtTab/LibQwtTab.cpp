// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

//header
#include "common/RegistLibrary.hpp"
#include "common/XML/SignalOptions.hpp"
#include "UI/Core/CNodeBuilders.hpp"
#include "UI/Core/NPlugin.hpp"
#include "UI/Core/NPlugins.hpp"
#include "UI/QwtTab/NPlotXY.hpp"
#include "UI/QwtTab/LibQwtTab.hpp"

using namespace cf3::common;
using namespace cf3::common::XML;
using namespace cf3::ui::core;

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace QwtTab {

cf3::common::RegistLibrary<LibQwtTab> libQwtTab;

////////////////////////////////////////////////////////////////////////////////

void LibQwtTab::initiate()
{
  if(m_is_initiated)
    return;
  
  initiate_impl();
  m_is_initiated = true;
}


void LibQwtTab::initiate_impl()
{
  NPlugins::global()->register_plugin<LibQwtTab>();
  CNodeBuilders::instance().register_builder<NPlotXY>("cf3.solver.CPlotXY");
}

////////////////////////////////////////////////////////////////////////////////

} // Client
} // UI
} // cf3
