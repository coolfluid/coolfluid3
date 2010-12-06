// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QtCore>

#include "GUI/Network/ComponentNames.hpp"

#include "Common/Core.hpp"
#include "Common/CRoot.hpp"
#include "Common/NotificationQueue.hpp"
#include "Common/XmlHelpers.hpp"

#include "Solver/CMethod.hpp"
#include "Solver/ScalarAdvection.hpp"
#include "Solver/LoadMesh.hpp"

#include "GUI/Server/Notifier.hpp"
#include "GUI/Server/ProcessingThread.hpp"

#include "GUI/Server/ServerRoot.hpp"

using namespace std;
using namespace CF::Common;
using namespace CF::GUI::Server;

NotificationQueue * ServerRoot::m_queue;
Notifier * ServerRoot::m_notifier;

ProcessingThread * ServerRoot::m_thread = nullptr;
QMutex ServerRoot::m_mutex;
SignalCatcher * ServerRoot::m_catcher = new SignalCatcher();
boost::shared_ptr<XmlDoc> ServerRoot::m_doc;

void SignalCatcher::finished()
{
  XmlNode& nodedoc = *XmlOps::goto_doc_node(*ServerRoot::m_doc.get());
  XmlNode& frameNode = *nodedoc.first_node();

  if(frameNode.next_sibling() != nullptr)
  {
    ServerRoot::core()->sendSignal(*ServerRoot::m_doc.get());
    ServerRoot::journal()->add_signal(*frameNode.next_sibling());
  }

  delete ServerRoot::m_thread;
  ServerRoot::m_thread = nullptr;
  ServerRoot::m_doc.reset();
  ServerRoot::m_mutex.unlock();

  ServerRoot::m_queue->flush();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CRoot::Ptr ServerRoot::root()
{
  static bool created = false;
  CRoot::Ptr root = Core::instance().root();

  if(!created)
  {
    CCore::Ptr core(new CCore());

    m_queue = new NotificationQueue(root);
    m_notifier = new Notifier(m_queue);

    m_notifier->listenToEvent("tree_updated", true);

    QObject::connect(m_notifier, SIGNAL(eventOccured(std::string,CF::Common::CPath)),
                     core.get(), SLOT(newEvent(std::string,CF::Common::CPath)));

    m_thread = nullptr;
    root->add_component(core);

    created = true;

    Component::Ptr tools = root->get_child("Tools");

    tools->create_component_type<Solver::ScalarAdvection>( "SetupScalarAdvection" )->mark_basic();
    tools->create_component_type<Solver::LoadMesh>( "LoadMesh" )->mark_basic();
    tools->create_component_type<CJournal>("Journal")->mark_basic();
  }

  return root;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ServerRoot::processSignal(const string & target,
                               const CPath & receiver,
                               const string & clientid,
                               const string & frameid,
                               XmlNode & node, boost::shared_ptr<XmlDoc> doc)
{
  if(m_mutex.tryLock())
  {
    m_doc.swap(doc);
    Component::Ptr receivingCompo = root()->access_component(receiver);
    m_thread = new ProcessingThread(node, target, receivingCompo);
    QObject::connect(m_thread, SIGNAL(finished()), m_catcher, SLOT(finished()));
    m_thread->start();
    journal()->add_signal(node);
  }
  else
  {
    try
    {
      Component::Ptr comp = root()->access_component(receiver);

      if( comp->signal(target).is_read_only )
      {
        comp->call_signal(target, *node.first_node() );

        XmlNode& nodedoc = *XmlOps::goto_doc_node(*doc.get());
        XmlNode& frameNode = *nodedoc.first_node();

        if(frameNode.next_sibling() != nullptr)
          core()->sendSignal(*doc.get());
      }
      else
        core()->sendFrameRejected(clientid, frameid, SERVER_CORE_PATH, "Server is busy.");
    }
    catch(CF::Common::Exception & cfe)
    {
      core()->sendException(cfe.what());
    }
    catch(std::exception & stde)
    {
      core()->sendException(stde.what());
    }
    catch(...)
    {
      CFerror << "Unknown exception thrown during execution of action [" << target
          << "] on component " << " [" << receiver << "]." << CFendl;
    }
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CCore::Ptr ServerRoot::core()
{
  return root()->access_component<CCore>(SERVER_CORE_PATH);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CJournal::Ptr ServerRoot::journal()
{
  return root()->access_component<CJournal>(SERVER_JOURNAL_PATH);
}
