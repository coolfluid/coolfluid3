#ifndef CF_server_RemoteClientAppender_hh
#define CF_server_RemoteClientAppender_hh

#include <string>
#include <iostream>

#include <QObject>

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace Server {
      
////////////////////////////////////////////////////////////////////////////

  /// Appends LoggingEvents to the remote client log window.
  
  class RemoteClientAppender : public QObject//, public logcpp::LayoutAppender
  {
    Q_OBJECT
    
  public:
    
    RemoteClientAppender(const std::string& name);
    virtual ~RemoteClientAppender();
    
    virtual bool reopen();
    virtual void close();
    
  protected:
    //    virtual void _append(const logcpp::LoggingEvent& event);
    
  signals:
    void newData(const QString & m_data);
  };
  
////////////////////////////////////////////////////////////////////////////
  
} // namespace Server
} // namespace GUI
} // namespace CF

//////////////////////////////////////////////////////////////////////////////

#endif // CF_server_RemoteClientAppender_hh
