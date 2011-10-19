// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Tools_Solver_LogForwarder_hpp
#define cf3_Tools_Solver_LogForwarder_hpp

#include "Common/PE/CPEManager.hpp"
#include "Common/LogStringForwarder.hpp"

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace Tools {
namespace Solver {

////////////////////////////////////////////////////////////////////////////

  /// Appends LoggingEvents to the remote client log window.

  class LogForwarder : public common::LogStringForwarder
  {

  public:

    LogForwarder();

  protected:

    virtual void message(const std::string & data);

  private:

    common::PE::CPEManager::Ptr m_manager;

  };

////////////////////////////////////////////////////////////////////////////

} // Tools
} // Tools
} // cf3

//////////////////////////////////////////////////////////////////////////////

#endif // CF3_Tools_Solver_LogForwarder_hpp
