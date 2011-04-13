// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/RegistLibrary.hpp"

#include "Common/XML/SignalOptions.hpp"

#include "UI/Core/NPlugin.hpp"
#include "UI/Core/NPlugins.hpp"

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
  Common::Core & core = Common::Core::instance();
  m_argc = core.argc();

  NPlugin::Ptr plugin = NPlugins::globalPlugins()->registerPlugin<LibQwtTab>();


  plugin->addSignal("new_plot", "Creates a new 2D plot.", "New Plot")
      ->signature->connect( boost::bind(&LibQwtTab::new_plot_signature, this, _1) );


//  m_tabIndex = TabBuilder::instance()->addTab(new Widget3D(), "3D-View");
}

////////////////////////////////////////////////////////////////////////////////

void LibQwtTab::terminate_impl()
{

}

////////////////////////////////////////////////////////////////////////////////

void LibQwtTab::new_plot_signature( Common::SignalArgs & args )
{
  SignalOptions options( args );

  std::vector<URI::Scheme::Type> schemes(1);
  schemes[0] = URI::Scheme::CPATH;

  options.add("Name", std::string(), "New plot name.");
  options.add("Parent", URI("cpath://Root"), "Parent for the new component.", schemes);
}

////////////////////////////////////////////////////////////////////////////////

} // Client
} // UI
} // CF
