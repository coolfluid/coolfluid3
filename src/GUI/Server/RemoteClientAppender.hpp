#ifndef CF_server_RemoteClientAppender_hh
#define CF_server_RemoteClientAppender_hh

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
  
} // namespace Server
} // namespace GUI
} // namespace CF

//////////////////////////////////////////////////////////////////////////////

#endif // CF_server_RemoteClientAppender_hh
