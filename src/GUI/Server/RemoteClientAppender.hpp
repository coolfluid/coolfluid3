// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_server_RemoteClientAppender_hpp
#define CF_server_RemoteClientAppender_hpp

#include <string>
#include <iostream>

#include "Common/LogStringForwarder.hpp"

#include <QObject>

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace Server {
      
////////////////////////////////////////////////////////////////////////////

  /// Appends LoggingEvents to the remote client log window.
  
  class RemoteClientAppender : public QObject, public CF::Common::LogStringForwarder
  {
    Q_OBJECT
    
  public:
    
    RemoteClientAppender();
    
  protected:
    virtual void message(const std::string & data);
    
  signals:
    void newData(const QString & data);
  };
  
////////////////////////////////////////////////////////////////////////////
  
} // Server
} // GUI
} // CF

//////////////////////////////////////////////////////////////////////////////

#endif // CF_server_RemoteClientAppender_hpp
