// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Tools_Solver_LogForwarder_hpp
#define CF_Tools_Solver_LogForwarder_hpp

#include "Common/MPI/CPEManager.hpp"
#include "Common/LogStringForwarder.hpp"

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Tools {
namespace Solver {

////////////////////////////////////////////////////////////////////////////

  /// Appends LoggingEvents to the remote client log window.

  class LogForwarder : public Common::LogStringForwarder
  {

  public:

    LogForwarder();

  protected:

    virtual void message(const std::string & data);

  private:

    Common::Comm::CPEManager::Ptr m_manager;

  };

////////////////////////////////////////////////////////////////////////////

} // Tools
} // Tools
} // CF

//////////////////////////////////////////////////////////////////////////////

#endif // CF_Tools_Solver_LogForwarder_hpp
