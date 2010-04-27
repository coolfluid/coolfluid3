//#include "logcpp/PortabilityImpl.hpp"

#include "GUI/Server/RemoteClientAppender.hpp"

using namespace CF::GUI::Server;

    RemoteClientAppender::RemoteClientAppender(const std::string& name)// : logcpp::LayoutAppender(name)
    {}

    RemoteClientAppender::~RemoteClientAppender()
    {
      close();
    }

    void RemoteClientAppender::close()
    {
      // empty
    }

  //  void RemoteClientAppender::_append(const logcpp::LoggingEvent& event)
//    {
//      emit newData( _getLayout().format(event).c_str() );
//    }

    bool RemoteClientAppender::reopen()
    {
      return true;
    }

