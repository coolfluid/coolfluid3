// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/RegistLibrary.hpp"

#include "Common/XML/SignalOptions.hpp"

#include "UI/Core/CNodeBuilders.hpp"
#include "UI/Core/NPlugin.hpp"
#include "UI/Core/NPlugins.hpp"

#include "UI/QwtTab/NPlotXY.hpp"

#include "UI/QwtTab/LibQwtTab.hpp"

using namespace CF::Common;
using namespace CF::Common::XML;
using namespace CF::UI::Core;
//using namespace CF::UI::Graphics;

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {
namespace QwtTab {

CF::Common::RegistLibrary<LibQwtTab> libQwtTab;

////////////////////////////////////////////////////////////////////////////////

void LibQwtTab::initiate_impl()
{
  NPlugins::globalPlugins()->registerPlugin<LibQwtTab>();
  CNodeBuilders::instance().registerBuilder<NPlotXY>("CF.Solver.CPlotXY");
}

////////////////////////////////////////////////////////////////////////////////

void LibQwtTab::terminate_impl()
{

}

////////////////////////////////////////////////////////////////////////////////

} // Client
} // UI
} // CF
