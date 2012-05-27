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

#include "common/OptionT.hpp"
#include "common/Signal.hpp"
#include "common/XML/FileOperations.hpp"
#include "common/XML/SignalOptions.hpp"

#include "Tools/Shell/BasicCommands.hpp"
#include "Tools/Shell/Interpreter.hpp"

#include "ui/core/NScriptEngine.hpp"
#include "ui/core/NetworkThread.hpp"
#include "ui/core/NLog.hpp"

#include "ui/core/ThreadManager.hpp"
#include "ui/core/TreeThread.hpp"

#include "ui/uicommon/ComponentNames.hpp"

#include "ui/core/NetworkQueue.hpp"


using namespace cf3::common;
using namespace cf3::common::XML;
using namespace cf3::Tools::Shell;

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace core {

//////////////////////////////////////////////////////////////////////////////

NetworkQueue::NetworkQueue()
  : CNode( CLIENT_NETWORK_QUEUE, "NetworkQueue", CNode::DEBUG_NODE ),
    m_current_index(-1)
{
  m_script_file = new QFile();
  m_script_stream = new QTextStream();

  boost::program_options::options_description desc;

  desc.add( BasicCommands::description() );

  m_interpreter = new Interpreter( desc );

  regist_signal( "ack" )
      .description("Frame execution (non)aknowledgment (ACK/NACK)")
      .hidden( true )
      .connect( boost::bind( &NetworkQueue::signal_ack, this, _1 ) );
}


//////////////////////////////////////////////////////////////////////////////

NetworkQueue::~NetworkQueue()
{
  delete m_script_stream;
  delete m_script_file;
  delete m_interpreter;
}

//////////////////////////////////////////////////////////////////////////////

QString NetworkQueue::tool_tip() const
{
  return component_type();
}

//////////////////////////////////////////////////////////////////////////////

Transaction * NetworkQueue::send ( SignalArgs & args, Priority priority )
{
  Transaction * transaction = nullptr;
  std::string python_repr=args.to_python_script();
  if (python_repr.size())
    NScriptEngine::global().get()->append_false_command_to_python_console(python_repr);
  if( priority == IMMEDIATE ){
    ThreadManager::instance().network().send( args );
  }else
  {
    QString uuid = start_transaction();
    transaction = m_new_transactions[uuid];
    add_to_transaction( uuid, args );
    insert_transaction( uuid, priority );
  }
  return transaction;
}

//////////////////////////////////////////////////////////////////////////////

QString NetworkQueue::start_transaction()
{
  QString uuid( QUuid::createUuid().toString() );

  m_new_transactions[uuid] = new Transaction( uuid );

  return uuid;
}

//////////////////////////////////////////////////////////////////////////////

void NetworkQueue::add_to_transaction( const QString & uuid, SignalArgs & args )
{
  if ( m_new_transactions.contains(uuid) )
    m_new_transactions[uuid]->actions.append(args);
  else
    throw ValueNotFound( FromHere(), "No transaction found with UuiD [" +
                         uuid.toStdString() + "].");
}

//////////////////////////////////////////////////////////////////////////////

void NetworkQueue::insert_transaction( const QString & uuid,
                                       NetworkQueue::Priority priority )
{
  if ( !m_new_transactions.contains(uuid) )
    throw ValueNotFound( FromHere(), "No transaction found with UuiD [" +
                         uuid.toStdString() + "].");
  else
  {
    Transaction * transaction = m_new_transactions[uuid];

    switch ( priority )
    {
    case HIGH:
      m_transactions.prepend( transaction );
      m_current_index++;
      break;

    case MEDIUM:

      if( m_current_index == -1 )
        m_transactions.append( transaction );
      else
      {
        m_transactions.insert( m_current_index, transaction );
        m_current_index++;
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

    m_new_transactions.remove( uuid );
  }
}

//////////////////////////////////////////////////////////////////////////////

Handle< NetworkQueue > NetworkQueue::global()
{
  static Handle< NetworkQueue > queue = ThreadManager::instance().tree().root_child<NetworkQueue>(CLIENT_NETWORK_QUEUE);
  cf3_assert( is_not_null(queue.get()) );
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
  if( !is_running() && !m_transactions.isEmpty() )
  {
    m_current_index = 0;
    send_next_frame();
  }
}

//////////////////////////////////////////////////////////////////////////////

void NetworkQueue::signal_ack ( common::SignalArgs & args )
{
//  if( isRunning() )
  {
    SignalOptions & options = args.options();
    std::string frameid = options.value<std::string>( "frameid" );

//    if( m_currentFrameID == frameid )
    {
      bool success = options.value<bool>( "success" );

      if( !success )
      {
        std::string message = options.value<std::string>( "message" );
        QString msg("Error while running the script.\n\nThe following error occured:\n %1"
                    "\n\nWhen executing the signal:\n\n%2");

        XML::SignalFrame f = m_transactions[m_current_index]->actions[0].node;

        NLog::global()->add_exception( msg.arg(message.c_str()).arg(f.to_script().c_str()) );

        // if the signal was comming from a script, we close the script file
        if ( m_transactions[m_current_index]->from_script )
          m_script_file->close();

        m_transactions.removeAt( m_current_index );

        if( m_transactions.isEmpty() )
          m_current_index = -1;
      }
      else
      {
        m_transactions.removeAt( m_current_index );

        if( m_transactions.isEmpty() )
          m_current_index = -1;

        if( !m_transactions.isEmpty() )
          send_next_frame();
        else if( m_script_file->isOpen() )
          send_next_command();
      }
    }
//    else
//      NLog::global()->addWarning(QString("Bad uuid! Received \"%1\" but \"%2\" was excpeted")
//                                    .arg(frameid.c_str()).arg(m_currentFrameID.c_str()));
  }
//  else
//    NLog::global()->addWarning(QString("Received ACK while not running."));

}

//////////////////////////////////////////////////////////////////////////////

void NetworkQueue::send_next_frame()
{
  if( is_running() )
  {
    if( m_transactions.count() == 0 )
      m_current_index = -1;
    else
    {

      SignalFrame frame = m_transactions[0]->actions[0];
      m_current_index = 0;
      m_current_frame_id = frame.node.attribute_value("frameid");
      ThreadManager::instance().network().send( frame );
    }
  }
}

//////////////////////////////////////////////////////////////////////////////

void NetworkQueue::execute_script ( const QString & filename )
{
  if( m_script_file->isOpen() )
    throw IllegalCall( FromHere(), "Another script is already in execution." );

  m_script_file->setFileName( filename );

  if( !m_script_file->exists() )
    throw FileSystemError( FromHere(), "The file [" + filename.toStdString() +
                           "] does not exist." );

  BasicCommands::dispatcher = this;
  BasicCommands::tree_root = ThreadManager::instance().tree().root();
  BasicCommands::current_component = ThreadManager::instance().tree().root();

  if( !m_script_file->open( QIODevice::ReadOnly ) )
    NLog::global()->add_error( m_script_file->errorString() );
  else
  {
    NLog::global()->add_message("Running script: " + filename);
    m_script_stream->setDevice( m_script_file );
    if(!filename.endsWith(".py")) // non-python, assume CFscript
    {
      send_next_command();
    }
    else // python
    {
      const URI script_engine_path("//Tools/Python/ScriptEngine", common::URI::Scheme::CPATH);
      
      SignalOptions options;
      options.add("script", m_script_stream->readAll().toStdString());
      SignalFrame frame = options.create_frame("execute_script", script_engine_path, script_engine_path);
      
      dispatch_signal("execute_script", script_engine_path, frame);
    }
  }
}

//////////////////////////////////////////////////////////////////////////////

void NetworkQueue::send_next_command()
{
  if( m_script_stream->atEnd() )
    m_script_file->close();
  else
  {
    QString command = m_script_stream->readLine();

    while( command.endsWith( "\\" ) )
    {
      QString newLine( m_script_stream->readLine() );
      command.remove( command.length() - 1, 1 );     // remove the trailing \
      command.append( newLine.trimmed() );  // read and append the next line
      command = QString::fromStdString( command.trimmed().toStdString() + " " + newLine.trimmed().toStdString() );
    }


    int index = command.indexOf('#');

    if( index != -1 )
      command.remove( index, command.length() - index ); // remove comments

    if( !command.isEmpty() )
    {
      NLog::global()->add_message( "Executing command: " + command );
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
} // ui
} // cf3
