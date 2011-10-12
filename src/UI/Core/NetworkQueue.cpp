// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QFile>
#include <QList>
#include <QTextStream>
#include <QVector>
#include <QUuid>

#include <boost/program_options.hpp>

#include "Common/OptionT.hpp"
#include "Common/Signal.hpp"
#include "Common/XML/FileOperations.hpp"
#include "Common/XML/SignalOptions.hpp"

#include "Tools/Shell/BasicCommands.hpp"
#include "Tools/Shell/Interpreter.hpp"

#include "UI/Core/NetworkThread.hpp"
#include "UI/Core/NLog.hpp"

#include "UI/Core/ThreadManager.hpp"
#include "UI/Core/TreeThread.hpp"

#include "UI/UICommon/ComponentNames.hpp"

#include "UI/Core/NetworkQueue.hpp"

using namespace CF::Common;
using namespace CF::Common::XML;
using namespace CF::Tools::Shell;

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {
namespace Core {

//////////////////////////////////////////////////////////////////////////////

NetworkQueue::NetworkQueue()
  : CNode( CLIENT_NETWORK_QUEUE, "NetworkQueue", CNode::DEBUG_NODE ),
    m_currentIndex(-1)
{
  m_scriptFile = new QFile();
  m_scriptStream = new QTextStream();

  boost::program_options::options_description desc;

  desc.add( BasicCommands::description() );

  m_interpreter = new Interpreter( desc );

  regist_signal( "ack" )
      ->description("Frame execution (non)aknowledgment (ACK/NACK)")
      ->hidden( true )
      ->connect( boost::bind( &NetworkQueue::signal_ack, this, _1 ) );
}


//////////////////////////////////////////////////////////////////////////////

NetworkQueue::~NetworkQueue()
{
  delete m_scriptStream;
  delete m_scriptFile;
  delete m_interpreter;
}

//////////////////////////////////////////////////////////////////////////////

QString NetworkQueue::toolTip() const
{
  return componentType();
}

//////////////////////////////////////////////////////////////////////////////

Transaction * NetworkQueue::send ( SignalArgs & args, Priority priority )
{
  Transaction * transaction = nullptr;

  if( priority == IMMEDIATE )
    ThreadManager::instance().network().send( args );
  else
  {
    QString uuid = start_transaction();
    transaction = m_newTransactions[uuid];
    add_to_transaction( uuid, args );
    insert_transaction( uuid, priority );
  }

  return transaction;
}

//////////////////////////////////////////////////////////////////////////////

QString NetworkQueue::start_transaction()
{
  QString uuid( QUuid::createUuid().toString() );

  m_newTransactions[uuid] = new Transaction( uuid );

  return uuid;
}

//////////////////////////////////////////////////////////////////////////////

void NetworkQueue::add_to_transaction( const QString & uuid, SignalArgs & args )
{
  if ( m_newTransactions.contains(uuid) )
    m_newTransactions[uuid]->actions.append(args);
  else
    throw ValueNotFound( FromHere(), "No transaction found with UUID [" +
                         uuid.toStdString() + "].");
}

//////////////////////////////////////////////////////////////////////////////

void NetworkQueue::insert_transaction( const QString & uuid,
                                       NetworkQueue::Priority priority )
{
  if ( !m_newTransactions.contains(uuid) )
    throw ValueNotFound( FromHere(), "No transaction found with UUID [" +
                         uuid.toStdString() + "].");
  else
  {
    Transaction * transaction = m_newTransactions[uuid];

    switch ( priority )
    {
    case HIGH:
      m_transactions.prepend( transaction );
      m_currentIndex++;
      break;

    case MEDIUM:

      if( m_currentIndex == -1 )
        m_transactions.append( transaction );
      else
      {
        m_transactions.insert( m_currentIndex, transaction );
        m_currentIndex++;
      }
      break;

    case LOW:
      m_transactions.append( transaction );
      break;

    case IMMEDIATE:
      throw BadValue( FromHere(), "Immediate priority cannot be applied to a transaction." );
    }

    start(); // run the transaction (ignored if another one is already being executed)

    SignalFrame args = transaction->actions[0];

    m_newTransactions.remove( uuid );
  }
}

//////////////////////////////////////////////////////////////////////////////

NetworkQueue::Ptr NetworkQueue::global_queue()
{
  static NetworkQueue::Ptr queue = ThreadManager::instance().tree().rootChild<NetworkQueue>(CLIENT_NETWORK_QUEUE);
  cf_assert( is_not_null(queue.get()) );
  return queue;
}

//////////////////////////////////////////////////////////////////////////////

void NetworkQueue::dispatch_signal( const std::string & target,
                                    const URI & receiver,
                                    SignalArgs &args )
{
  args.options().flush();
  Transaction * transaction = send( args, LOW );

  if( is_not_null(transaction) )
    transaction->from_script = true;
}

//////////////////////////////////////////////////////////////////////////////

void NetworkQueue::start()
{
  if( !isRunning() && !m_transactions.isEmpty() )
  {
    m_currentIndex = 0;
    send_next_frame();
  }
}

//////////////////////////////////////////////////////////////////////////////

void NetworkQueue::signal_ack ( Common::SignalArgs & args )
{
  if( isRunning() )
  {
    SignalOptions & options = args.options();
    std::string frameid = options.value<std::string>( "frameid" );

    if( m_currentFrameID == frameid )
    {
      bool success = options.value<bool>( "success" );

      if( !success )
      {
        std::string message = options.value<std::string>( "message" );
        QString msg("Error while running the script.\n\nThe following error occured:\n %1"
                    "\n\nWhen executing the signal:\n\n%2");

        XML::SignalFrame f = m_transactions[m_currentIndex]->actions[0].node;

        NLog::globalLog()->addException( msg.arg(message.c_str()).arg(f.to_script().c_str()) );

        // if the signal was comming from a script, we close the script file
        if ( m_transactions[m_currentIndex]->from_script )
          m_scriptFile->close();

        m_transactions.removeAt( m_currentIndex );

        if( m_transactions.isEmpty() )
          m_currentIndex = -1;
      }
      else
      {
        m_transactions.removeAt( m_currentIndex );

        if( m_transactions.isEmpty() )
          m_currentIndex = -1;

        if( !m_transactions.isEmpty() )
          send_next_frame();
        else if( m_scriptFile->isOpen() )
          send_next_command();
      }
    }
    else
      NLog::globalLog()->addWarning(QString("Bad uuid! Received \"%1\" but \"%2\" was excpeted")
                                    .arg(frameid.c_str()).arg(m_currentFrameID.c_str()));
  }
  else
    NLog::globalLog()->addWarning(QString("Received ACK while not running."));

}

//////////////////////////////////////////////////////////////////////////////

void NetworkQueue::send_next_frame()
{
  if( isRunning() )
  {
    if( m_transactions.count() == 0 )
      m_currentIndex = -1;
    else
    {

      SignalFrame frame = m_transactions[0]->actions[0];
      m_currentIndex = 0;
      m_currentFrameID = frame.node.attribute_value("frameid");
      ThreadManager::instance().network().send( frame );
    }
  }
}

//////////////////////////////////////////////////////////////////////////////

void NetworkQueue::execute_script ( const QString & filename )
{
  if( m_scriptFile->isOpen() )
    throw IllegalCall( FromHere(), "Another script is already in execution." );

  m_scriptFile->setFileName( filename );

  if( !m_scriptFile->exists() )
    throw FileSystemError( FromHere(), "The file [" + filename.toStdString() +
                           "] does not exist." );

  BasicCommands::dispatcher = this;
  BasicCommands::tree_root = ThreadManager::instance().tree().root()->root();
  BasicCommands::current_component = ThreadManager::instance().tree().root()->root();

  if( !m_scriptFile->open( QIODevice::ReadOnly ) )
    NLog::globalLog()->addError( m_scriptFile->errorString() );
  else
  {
    NLog::globalLog()->addMessage("Running script: " + filename);
    m_scriptStream->setDevice( m_scriptFile );
    if(!filename.endsWith(".py")) // non-python, assume CFscript
    {
      send_next_command();
    }
    else // python
    {
      const URI script_engine_path("//Root/Tools/Python/ScriptEngine", Common::URI::Scheme::CPATH);
      
      SignalOptions options;
      options.add_option< OptionT<std::string> >("script", m_scriptStream->readAll().toStdString());
      SignalFrame frame = options.create_frame("execute_script", script_engine_path, script_engine_path);
      
      dispatch_signal("execute_script", script_engine_path, frame);
    }
  }
}

//////////////////////////////////////////////////////////////////////////////

void NetworkQueue::send_next_command()
{
  if( m_scriptStream->atEnd() )
    m_scriptFile->close();
  else
  {
    QString command = m_scriptStream->readLine();

    while( command.endsWith( "\\" ) )
    {
      QString newLine( m_scriptStream->readLine() );
      command.remove( command.length() - 1, 1 );     // remove the trailing \
      command.append( newLine.trimmed() );  // read and append the next line
      command = QString::fromStdString( command.trimmed().toStdString() + " " + newLine.trimmed().toStdString() );
    }


    int index = command.indexOf('#');

    if( index != -1 )
      command.remove( index, command.length() - index ); // remove comments

    if( !command.isEmpty() )
    {
      NLog::globalLog()->addMessage( "Executing command: " + command );
      m_interpreter->handle_read_line( command.toStdString() );

      if( m_transactions.isEmpty() )
        send_next_command();

    }
    else
      send_next_command(); // if command is empty, try the next one
  }

//  return command_sent;
}

//////////////////////////////////////////////////////////////////////////////

} // Core
} // UI
} // CF
