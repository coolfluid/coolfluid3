// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Core.hpp"
#include "Common/CRoot.hpp"

#include "Common/StringConversion.hpp"
#include "Common/OptionT.hpp"

#include "Common/XML/SignalFrame.hpp"
#include "Common/XML/SignalOptions.hpp"

#include "Tools/Solver/LogForwarder.hpp"

#include "Common/Log.hpp"

using namespace cf3::common;
using namespace cf3::common::PE;
using namespace cf3::common::XML;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace Tools {
namespace Solver {

////////////////////////////////////////////////////////////////////////////

LogForwarder::LogForwarder()
{
  m_manager = Core::instance().root().access_component_ptr_checked("//Root/Tools/PEManager")->as_ptr<CPEManager>();
}

////////////////////////////////////////////////////////////////////////////

void LogForwarder::message(const std::string &data)
{
  /// @todo remove those hardcoded URIs
  SignalFrame frame("message", "cpath://Root/UI/Log", "cpath://Root/UI/Log");
  SignalOptions options(frame);
  std::string header = "Worker[" + to_str( Comm::instance().rank() ) + "] ";

  options.add_option< OptionT<std::string> >("type", "Info");
  options.add_option< OptionT<std::string> >("text", header + data);
//  frame.set_option<std::string>("message", data);

  options.flush();

  m_manager->send_to_parent( frame );
}

////////////////////////////////////////////////////////////////////////////

} // Tools
} // Tools
} // cf3
