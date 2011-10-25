// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QList>
#include <QMutex>

#include "rapidxml/rapidxml.hpp"

#include "common/Signal.hpp"
#include "common/Log.hpp"
#include "common/Group.hpp"
#include "common/Core.hpp"
#include "common/Root.hpp"
#include "common/NotificationQueue.hpp"
#include "common/PE/Manager.hpp"
//#include "common/XML/SignalFrame.hpp"
#include "common/XML/Protocol.hpp"

#include "mesh/Table.hpp"

#include "solver/CPlotter.hpp"

#include "UI/UICommon/ComponentNames.hpp"

#include "UI/Server/Notifier.hpp"
#include "UI/Server/ProcessingThread.hpp"

#include "UI/Server/ServerRoot.hpp"

using namespace cf3::common;
using namespace cf3::common::PE;
using namespace cf3::common::XML;
using namespace cf3::mesh;
using namespace cf3::solver;

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace UI {
namespace Server {

//////////////////////////////////////////////////////////////////////////////

ServerRoot::ServerRoot()
  : m_queue(nullptr),
    m_notifier(nullptr),
    m_thread(nullptr),
    m_root( Core::instance().root().as_ptr<Root>() ),
    m_core( new CCore() ),
    m_journal( common::allocate_component<Journal>("Journal") ),
    m_manager( common::allocate_component<Manager>("PEManager") ),
    m_plotter( common::allocate_component<CPlotter>("Plotter") )
{
  m_root->add_component(m_core);

  Component::Ptr tools = m_root->get_child_ptr("Tools");

  tools->add_component( m_journal );
  tools->add_component( m_manager );
  tools->add_component( m_plotter );

  m_local_components << URI( SERVER_CORE_PATH, URI::Scheme::CPATH );

  m_manager->mark_basic();
  m_plotter->mark_basic();

  Table<Real>::Ptr table = tools->create_component_ptr< Table<Real> >("MyTable");
  table->set_row_size(8); // reserve 8 columns
  Table<Real>::Buffer buffer = table->create_buffer(8000);

  table->mark_basic();
  m_plotter->set_data_set( table->uri() );

  // fill the table
  std::vector<Real> row(8);

  for(Real value = 0.0 ; value != 1000.0 ; value += 1.0 )
  {
    row[0] = value / 1000;          // x
    row[1] = 0;                     // y
    row[2] = (value / 1000) - 1;    // z
    row[3] = std::sin(4 * row[0]);  // u
    row[4] = 0;                     // v
    row[5] = std::cos(4 * row[2]);  // w
    row[6] = 1000;                  // p
    row[7] = 278 * row[0];          // t

    buffer.add_row( row );
  }

  m_manager->signal("signal_to_forward")->connect( boost::bind(&ServerRoot::signal_to_forward, this, _1) );

  buffer.flush();
}

//////////////////////////////////////////////////////////////////////////////

ServerRoot::~ServerRoot()
{
  delete m_queue;
  delete m_notifier;

  if( is_not_null(m_thread) )
    m_thread->quit();

  delete m_thread;
}

//////////////////////////////////////////////////////////////////////////////

ServerRoot & ServerRoot::instance()
{
  static ServerRoot root;
  return root;
}

//////////////////////////////////////////////////////////////////////////////

void ServerRoot::process_signal( const std::string & target,
                                 const URI & receiver,
                                 const std::string & clientid,
                                 const std::string & frameid,
                                 SignalArgs & signal )
{
  if( m_local_components.contains(receiver) )
  {
    if( m_mutex.tryLock() )
    {
      m_doc.swap(signal.xml_doc);
      m_current_client_id = clientid;
      m_current_frame_id = frameid;
      Component::Ptr receivingCompo = m_root->access_component_ptr_checked(receiver);

      m_thread = new ProcessingThread(signal, target, receivingCompo);
      QObject::connect(m_thread, SIGNAL(finished()), this, SLOT(finished()));
      journal()->add_signal( signal );
      m_thread->start();
    }
    else
    {
      std::string message;
      bool success = false;
      //    bool

      try
      {
        Component::Ptr comp = m_root->access_component_ptr_checked(receiver);

        if( comp->signal(target)->is_read_only() )
        {
          comp->call_signal(target, signal );
          m_journal->add_signal(signal);

          SignalFrame reply = signal.get_reply();

          if( reply.node.is_valid() )
          {
            m_core->sendSignal( *signal.xml_doc.get() );
            m_journal->add_signal( reply );
          }

          success = true;
        }
        else
          message = "Server is busy.";
      }
      catch(cf3::common::Exception & cfe)
      {
        message = cfe.what();
      }
      catch(std::exception & stde)
      {
        message = stde.what();
      }
      catch(...)
      {
        message = "Unknown exception thrown during execution of action [" +
            target + "] on component [" + receiver.path() + "].";
      }

      m_core->sendACK( clientid, frameid, success, message );
    }
  }
  else // the receiver is not a local component
  {
    m_manager->send_to( "Workers", signal );
  }
}

//////////////////////////////////////////////////////////////////////////////

void ServerRoot::listen_to_events ()
{
  if(m_queue == nullptr)
  {
    m_queue = new NotificationQueue(m_root);
    m_notifier = new Notifier(m_queue);

    m_notifier->listenToEvent("tree_updated", true);

    QObject::connect(m_notifier, SIGNAL(eventOccured(std::string,cf3::common::URI)),
                     m_core.get(), SLOT(newEvent(std::string,cf3::common::URI)));
  }
}

//////////////////////////////////////////////////////////////////////////////

void ServerRoot::signal_to_forward( common::SignalArgs & args )
{
  m_core->sendSignal( *args.xml_doc.get() );
}

//////////////////////////////////////////////////////////////////////////////

void ServerRoot::finished ()
{
  XmlNode nodedoc = Protocol::goto_doc_node(*m_doc.get());
  SignalFrame frame(nodedoc.content->first_node());
  bool success = m_thread->success();
  std::string message( m_thread->message() );

  SignalFrame reply = frame.get_reply();

  if( reply.node.is_valid() )
  {
    rapidxml::xml_attribute<>* attr = reply.node.content->first_attribute("type");
    if( is_not_null(attr) && std::strcmp(attr->value(), Protocol::Tags::node_type_reply()) == 0)
    {
      core()->sendSignal(*m_doc.get());
      journal()->add_signal( reply );
    }
  }


  // clean processing environment
  delete m_thread;
  m_thread = nullptr;
  m_doc.reset();
  m_mutex.unlock();

  m_queue->flush();

  m_core->sendACK( m_current_client_id, m_current_frame_id, success, message );
  m_current_client_id.clear();
  m_current_frame_id.clear();
}

//////////////////////////////////////////////////////////////////////////////

} // Server
} // UI
} // cf3

