// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Core.hpp"
#include "Common/CRoot.hpp"

#include "Common/OptionT.hpp"

#include "Common/XML/SignalOptions.hpp"

#include "Tools/Solver/LogForwarder.hpp"

#include "Common/Log.hpp"

using namespace CF::Common;
using namespace CF::Common::mpi;
using namespace CF::Common::XML;

////////////////////////////////////////////////////////////////////////////

namespace CF {
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
  std::string path = m_manager->uri().string();
  SignalFrame frame("message", path, path);
  SignalOptions options(frame);

  options.add_option< OptionT<std::string> >("message", data);
//  frame.set_option<std::string>("message", data);

  m_manager->send_to_parent( frame );
}

////////////////////////////////////////////////////////////////////////////

} // Tools
} // Tools
} // CF
