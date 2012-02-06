// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QList>

#include "common/ConnectionManager.hpp"
#include "common/Signal.hpp"
#include "common/XML/SignalFrame.hpp"
#include "common/XML/SignalOptions.hpp"
#include "common/XML/FileOperations.hpp"

#include "common/Log.hpp"

#include "ui/core/NetworkThread.hpp"
#include "ui/core/NetworkQueue.hpp"
#include "ui/core/NLog.hpp"
#include "ui/core/NRoot.hpp"
#include "ui/core/ThreadManager.hpp"

#include "ui/core/RemoteDispatcher.hpp"

using namespace cf3::common;
using namespace cf3::common::XML;

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace core {

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
    NLog::global()->add_message( QString("Executing %1 frame(s)")
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

          NLog::global()->add_exception( msg.arg(message.c_str()).arg(str.c_str()) );

          m_running = false;
          emit finished();
        }
        else
        {
          NLog::global()->add_message( QString("Frame %1 was ack'ed").arg(frameid.c_str()) );
          send_signal( m_nextIndex );
          m_nextIndex++;
        }
      }
      else
      {
        m_running = false;
        //      m_connectionManager->manage_connection("ack")->disconnect();
        NLog::global()->add_message("Script is finished");
        emit finished();
      }
    }
  }
}

//////////////////////////////////////////////////////////////////////////////

void RemoteDispatcher::send_signal ( Uint index )
{
  NetworkThread & thread = ThreadManager::instance().network();

  if( thread.is_connected() && index < m_pendingSignals.size() )
  {
    SignalFrame & frame = m_pendingSignals[index];
    m_currentFrameId = frame.node.attribute_value("frameid");
    NetworkQueue::global()->send( frame );
  }
}

//////////////////////////////////////////////////////////////////////////////

} // Core
} // ui
} // cf3

