// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Core.hpp"

#include "common/StringConversion.hpp"
#include "common/OptionT.hpp"

#include "common/XML/SignalFrame.hpp"
#include "common/XML/SignalOptions.hpp"

#include "Tools/solver/LogForwarder.hpp"

#include "common/Log.hpp"

using namespace cf3::common;
using namespace cf3::common::PE;
using namespace cf3::common::XML;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace Tools {
namespace solver {

////////////////////////////////////////////////////////////////////////////

LogForwarder::LogForwarder()
{
  m_manager = Core::instance().root().access_component_checked("//Tools/PEManager")->handle<Manager>();
}

////////////////////////////////////////////////////////////////////////////

void LogForwarder::message(const std::string &data)
{
  /// @todo remove those hardcoded URIs
  SignalFrame frame("message", "cpath:/UI/Log", "cpath:/UI/Log");
  SignalOptions options(frame);
  std::string header = "Worker[" + to_str( Comm::instance().rank() ) + "] ";

  options.add("type", "Info");
  options.add("text", header + data);
//  frame.set_option<std::string>("message", data);

  options.flush();

  m_manager->send_to_parent( frame );
}

////////////////////////////////////////////////////////////////////////////

} // Tools
} // Tools
} // cf3
