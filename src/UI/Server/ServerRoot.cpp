// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QtCore>

#include "rapidxml/rapidxml.hpp"

#include "Common/Signal.hpp"
#include "Common/Log.hpp"
#include "Common/Core.hpp"
#include "Common/CRoot.hpp"
#include "Common/NotificationQueue.hpp"
#include "Common/MPI/CPEManager.hpp"
#include "Common/XML/Protocol.hpp"

#include "Mesh/CTable.hpp"

#include "Solver/CPlotter.hpp"

#include "UI/UICommon/ComponentNames.hpp"

#include "UI/Server/Notifier.hpp"
#include "UI/Server/ProcessingThread.hpp"

#include "UI/Server/ServerRoot.hpp"

using namespace CF::Common;
using namespace CF::Common::XML;
using namespace CF::Mesh;
using namespace CF::Solver;

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {
namespace Server {

//////////////////////////////////////////////////////////////////////////////

ServerRoot::ServerRoot()
  : m_queue(nullptr),
    m_notifier(nullptr),
    m_thread(nullptr),
    m_root( Core::instance().root().as_ptr<CRoot>() ),
    m_core( new CCore() ),
    m_journal( new CJournal("Journal") )
{
  m_root->add_component(m_core);

  Component::Ptr tools = m_root->get_child_ptr("Tools");

  tools->add_component( m_journal );
  tools->create_component_ptr<Comm::CPEManager>("PEManager")->mark_basic();

  CPlotter::Ptr plotter = tools->create_component_ptr<CPlotter>("Plotter");

  plotter->mark_basic();

  CTable<Real>::Ptr table = tools->create_component_ptr< CTable<Real> >("MyTable");
  table->set_row_size(8); // reserve 8 columns
  CTable<Real>::Buffer buffer = table->create_buffer(8000);

  table->mark_basic();
  plotter->set_data_set( table->uri() );

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

  buffer.flush();
}

//////////////////////////////////////////////////////////////////////////////

ServerRoot & ServerRoot::instance()
{
  static ServerRoot root;
  return root;
}

//////////////////////////////////////////////////////////////////////////////

void ServerRoot::processSignal(const std::string & target,
                               const URI & receiver,
                               const std::string & clientid,
                               const std::string & frameid,
                               SignalArgs & signal)
{
  if(m_mutex.tryLock())
  {
    m_doc.swap(signal.xml_doc);
    m_currentClientId = clientid;
    m_currentFrameId = frameid;
    Component::Ptr receivingCompo = m_root->retrieve_component_checked(receiver);

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
      Component::Ptr comp = m_root->retrieve_component_checked(receiver);

      if( comp->signal(target)->is_read_only() )
      {
        comp->call_signal(target, signal );
        m_journal->add_signal(signal);

        SignalFrame reply = signal.get_reply();

        if( reply.node.is_valid() )
        {
          m_core->sendSignal(*signal.xml_doc.get());
          m_journal->add_signal( reply );
        }

        success = true;
      }
      else
        message = "Server is busy.";
    }
    catch(CF::Common::Exception & cfe)
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

//////////////////////////////////////////////////////////////////////////////

void ServerRoot::listenToEvents()
{
  if(m_queue == nullptr)
  {
    m_queue = new NotificationQueue(m_root);
    m_notifier = new Notifier(m_queue);

    m_notifier->listenToEvent("tree_updated", true);

    QObject::connect(m_notifier, SIGNAL(eventOccured(std::string,CF::Common::URI)),
                     m_core.get(), SLOT(newEvent(std::string,CF::Common::URI)));
  }
}

//////////////////////////////////////////////////////////////////////////////

void ServerRoot::finished()
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

  m_core->sendACK( m_currentClientId, m_currentFrameId, success, message );
  m_currentClientId.clear();
  m_currentFrameId.clear();
}

//////////////////////////////////////////////////////////////////////////////

} // Server
} // UI
} // CF

