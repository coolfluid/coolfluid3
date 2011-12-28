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
#include "common/PE/Manager.hpp"
#include "common/XML/Protocol.hpp"

#include "ui/uicommon/ComponentNames.hpp"

#include "ui/server/ProcessingThread.hpp"

#include "ui/server/ServerRoot.hpp"

using namespace cf3::common;
using namespace cf3::common::PE;
using namespace cf3::common::XML;

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace server {

//////////////////////////////////////////////////////////////////////////////

ServerRoot::ServerRoot()
  : m_thread(nullptr),
    m_root( Core::instance().root().handle<Component>() ),
    m_core( new CCore() ),
    m_manager( common::allocate_component<Manager>("PEManager") )
{
  m_root->add_component(m_core);

  Handle< Component > tools = m_root->get_child("Tools");

  tools->add_component( m_manager );

  m_local_components << URI( SERVER_CORE_PATH, URI::Scheme::CPATH );

  m_manager->mark_basic();

  m_manager->signal("signal_to_forward")
      ->connect( boost::bind(&ServerRoot::signal_to_forward, this, _1) );

}

//////////////////////////////////////////////////////////////////////////////

ServerRoot::~ServerRoot()
{
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
      Handle< Component > receivingCompo = m_root->access_component_checked(receiver);

      m_thread = new ProcessingThread(signal, target, receivingCompo);
      QObject::connect(m_thread, SIGNAL(finished()), this, SLOT(finished()));
      m_thread->start();
    }
    else
    {
      std::string message;
      bool success = false;
      //    bool

      try
      {
        Handle< Component > comp = m_root->access_component_checked(receiver);

        if( comp->signal(target)->is_read_only() )
        {
          comp->call_signal(target, signal );

          SignalFrame reply = signal.get_reply();

          if( reply.node.is_valid() )
          {
            m_core->send_signal( signal );
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

      m_core->send_ack( clientid, frameid, success, message );
    }
  }
  else // the receiver is not a local component
  {
    m_manager->send_to( "Workers", signal );
  }
}

//////////////////////////////////////////////////////////////////////////////

void ServerRoot::signal_to_forward( common::SignalArgs & args )
{
  m_core->send_signal( args );
}

//////////////////////////////////////////////////////////////////////////////

void ServerRoot::finished ()
{
  XmlNode nodedoc = Protocol::goto_doc_node(*m_doc.get());
  SignalFrame frame(nodedoc.content->first_node());
  bool success = m_thread->success();
  std::string message( m_thread->message() );

  frame.xml_doc = m_doc;

  SignalFrame reply = frame.get_reply();

  if( reply.node.is_valid() )
  {
    rapidxml::xml_attribute<>* attr = reply.node.content->first_attribute("type");
    if( is_not_null(attr) && std::strcmp(attr->value(), Protocol::Tags::node_type_reply()) == 0)
    {
      m_core->send_signal( frame );
    }
  }


  // clean processing environment
  delete m_thread;
  m_thread = nullptr;
  m_doc.reset();
  m_mutex.unlock();

  m_core->send_ack( m_current_client_id, m_current_frame_id, success, message );
  m_current_client_id.clear();
  m_current_frame_id.clear();
}

//////////////////////////////////////////////////////////////////////////////

} // Server
} // ui
} // cf3

