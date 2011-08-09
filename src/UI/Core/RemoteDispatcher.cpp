// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QList>

#include "Common/ConnectionManager.hpp"
#include "Common/Signal.hpp"
#include "Common/XML/SignalFrame.hpp"
#include "Common/XML/SignalOptions.hpp"
#include "Common/XML/FileOperations.hpp"

#include "Common/Log.hpp"

#include "UI/Core/NetworkThread.hpp"
#include "UI/Core/NetworkQueue.hpp"
#include "UI/Core/NLog.hpp"
#include "UI/Core/NRoot.hpp"
#include "UI/Core/ThreadManager.hpp"

#include "UI/Core/RemoteDispatcher.hpp"

using namespace CF::Common;
using namespace CF::Common::XML;

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {
namespace Core {

//////////////////////////////////////////////////////////////////////////////

RemoteDispatcher::RemoteDispatcher( NRoot & root )
{
  root.signal( "ack" )
      ->connect( boost::bind( &RemoteDispatcher::send_next_signal, this, _1)/*, m_connectionManager*/ );
}

//////////////////////////////////////////////////////////////////////////////

void RemoteDispatcher::dispatch_signal(const std::string &target,
                                       const URI &receiver,
                                       SignalArgs &args)
{
  m_pendingSignals.append(args);
}

//////////////////////////////////////////////////////////////////////////////

void RemoteDispatcher::run ()
{
  if( !m_running && !m_pendingSignals.isEmpty() )
  {
    NLog::globalLog()->addMessage( QString("Executing %1 frame(s)")
                                       .arg(m_pendingSignals.count()) );
    m_nextIndex = 1;
    m_running = true;
    send_signal(0);
  }
}

//////////////////////////////////////////////////////////////////////////////

void RemoteDispatcher::send_next_signal( SignalArgs & args )
{
  if( m_running )
  {
    SignalOptions & options = args.options();
    std::string frameid = options.value<std::string>( "frameid" );

    if( m_currentFrameId == frameid )
    {
      if ( m_nextIndex <= m_pendingSignals.count() )
      {
        bool success = options.value<bool>( "success" );

        if( !success )
        {
          std::string message = options.value<std::string>( "message" );
          QString msg("Error while running the script. The following error occured : %1"
                      "\n When executing the signal \n%2");

          std::string str;
          to_string( m_pendingSignals[m_nextIndex-1].node, str );

          NLog::globalLog()->addException( msg.arg(message.c_str()).arg(str.c_str()) );

          m_running = false;
          emit finished();
        }
        else
        {
          NLog::globalLog()->addMessage( QString("Frame %1 was ack'ed").arg(frameid.c_str()) );
          send_signal( m_nextIndex );
          m_nextIndex++;
        }
      }
      else
      {
        m_running = false;
        //      m_connectionManager->manage_connection("ack")->disconnect();
        NLog::globalLog()->addMessage("Script is finished");
        emit finished();
      }
    }
  }
}

//////////////////////////////////////////////////////////////////////////////

void RemoteDispatcher::send_signal ( Uint index )
{
  NetworkThread & thread = ThreadManager::instance().network();

  if( thread.isConnected() && index < m_pendingSignals.size() )
  {
    SignalFrame & frame = m_pendingSignals[index];
    m_currentFrameId = frame.node.attribute_value("frameid");
    NetworkQueue::global_queue()->send( frame );
  }
}

//////////////////////////////////////////////////////////////////////////////

} // Core
} // UI
} // CF

