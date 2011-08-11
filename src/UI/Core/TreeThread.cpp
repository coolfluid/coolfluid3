// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QMutex>

#include "rapidxml/rapidxml.hpp"

#include "Common/Log.hpp"

#include "Common/XML/Protocol.hpp"
#include "Common/XML/XmlDoc.hpp"

#include "UI/Core/NBrowser.hpp"
#include "UI/Core/NetworkQueue.hpp"
#include "UI/Core/NetworkThread.hpp"
#include "UI/Core/NGeneric.hpp"
#include "UI/Core/NLog.hpp"
#include "UI/Core/NPlugins.hpp"
#include "UI/Core/NRoot.hpp"
#include "UI/Core/NTree.hpp"
#include "UI/Core/ThreadManager.hpp"

#include "UI/Core/NPlugin.hpp"
#include "UI/Core/CNodeBuilders.hpp"

#include "UI/UICommon/ComponentNames.hpp"

#include "Common/XML/FileOperations.hpp"

#include "UI/Core/TreeThread.hpp"

using namespace CF::Common;
using namespace CF::Common::XML;
using namespace CF::UI::UICommon;

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {
namespace Core {

////////////////////////////////////////////////////////////////////////////

TreeThread::TreeThread(QObject * parent) :
    QThread(parent)
{
}

////////////////////////////////////////////////////////////////////////////

TreeThread::~TreeThread()
{
  if(isRunning())
  {
    exit(0);
    wait();
  }
}

////////////////////////////////////////////////////////////////////////////

void TreeThread::setMutex(QMutex * mutex)
{
  m_mutex = mutex;
}

////////////////////////////////////////////////////////////////////////////

void TreeThread::run()
{
  m_root = NRoot::Ptr(new NRoot(CLIENT_ROOT));

  CRoot::Ptr realRoot = m_root->root();

  NLog::Ptr log(new NLog());
  NBrowser::Ptr browser(new NBrowser());
  NTree::Ptr tree(new NTree(m_root));
  NPlugins::Ptr plugins(new NPlugins(CLIENT_PLUGINS));
  NGeneric::Ptr uidir( new NGeneric("UI", "CF.Common.CGroup", CNode::LOCAL_NODE ) );
  NetworkQueue::Ptr networkQueue( new NetworkQueue() );

  Logger::instance().getStream(WARNING).addStringForwarder( log.get() );
  Logger::instance().getStream(ERROR).addStringForwarder( log.get() );
  Logger::instance().getStream(INFO).addStringForwarder( log.get() );

  Logger::instance().getStream(INFO).setStamp(LogStream::STRING, "%type% ");
  Logger::instance().getStream(ERROR).setStamp(LogStream::STRING, "%type% ");
  Logger::instance().getStream(WARNING).setStamp(LogStream::STRING, "%type% ");

  // add components to the root
  uidir->add_component(log);
  uidir->add_component(browser);
  uidir->add_component(tree);
  uidir->add_component(networkQueue);
  uidir->add_component(plugins);

  realRoot->add_component(uidir);

  // mark all components as basic
  m_root->mark_basic();
  uidir->mark_basic();
  log->mark_basic();
  browser->mark_basic();
  tree->mark_basic();
  networkQueue->mark_basic();
  plugins->mark_basic();

  // set the root as model root
  tree->setRoot(m_root);

  ThreadManager::instance().network().newSignal.connect(
      boost::bind(&TreeThread::newSignal, this, _1) );

  m_mutex->unlock();
//  m_waitCondition.wakeAll();

  // execute the event loop
  exec();
}

////////////////////////////////////////////////////////////////////////////

void TreeThread::newSignal(Common::XML::XmlDoc::Ptr doc)
{
  const char * tag = Protocol::Tags::node_frame();
  XmlNode nodedoc = Protocol::goto_doc_node(*doc.get());
  rapidxml::xml_node<char>* nodeToProcess = nodedoc.content->first_node(tag);

  if(nodeToProcess != nullptr)
  {
    rapidxml::xml_node<>* tmpNode = nodeToProcess->next_sibling( tag );

    // check this is a reply
    if(tmpNode != nullptr && std::strcmp(tmpNode->first_attribute("type")->value(), "reply") == 0)
      nodeToProcess = tmpNode;

    std::string type = nodeToProcess->first_attribute("target")->value();
    std::string receiver = nodeToProcess->first_attribute("receiver")->value();

    try
    {
      CRoot::Ptr realRoot = root()->root();
      SignalFrame frame(nodeToProcess);

      std::string str;
      to_string(frame.node, str);
      CFinfo << str << CFendl;

      if(realRoot->uri().path() == URI(receiver).path())
        root()->call_signal(type, frame);
      else
        realRoot->retrieve_component(receiver)->call_signal(type, frame);
    }
    catch(CF::Common::Exception & cfe)
    {
      NLog::globalLog()->addException(/*QString("%1 %2").arg(type.c_str()).arg(receiver.c_str()) +  */cfe.what());
    }
    catch(std::exception & stde)
    {
      NLog::globalLog()->addException(stde.what());
    }
    catch(...)
    {
      CFerror << "Unknown exception thrown during execution of action [" << type
          << "] on component " << " [" << receiver << "]." << CFendl;
    }

  }

}

////////////////////////////////////////////////////////////////////////////

} // Core
} // UI
} // CF
