// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_server_RemoteClientAppender_hpp
#define cf3_server_RemoteClientAppender_hpp

#include "common/LogStringForwarder.hpp"

#include <QObject>

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace server {
      
////////////////////////////////////////////////////////////////////////////

  /// Appends LoggingEvents to the remote client log window.
  
  class RemoteClientAppender : public QObject, public cf3::common::LogStringForwarder
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
} // ui
} // cf3

//////////////////////////////////////////////////////////////////////////////

#endif // cf3_server_RemoteClientAppender_hpp
