//#include "logcpp/PortabilityImpl.hpp"

#include "GUI/Server/RemoteClientAppender.hpp"

using namespace CF::Common;
using namespace CF::GUI::Server;

RemoteClientAppender::RemoteClientAppender() : LogStringForwarder()
{}


void RemoteClientAppender::message(const std::string & data)
{
  //emit newData( data.c_str() );
}


