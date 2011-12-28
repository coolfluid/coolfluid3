// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QMutex>

#include "rapidxml/rapidxml.hpp"

#include "common/Log.hpp"

#include "common/XML/Protocol.hpp"
#include "common/XML/XmlDoc.hpp"

#include "ui/core/NBrowser.hpp"
#include "ui/core/NetworkQueue.hpp"
#include "ui/core/NetworkThread.hpp"
#include "ui/core/NGeneric.hpp"
#include "ui/core/NLog.hpp"
#include "ui/core/NPlugins.hpp"
#include "ui/core/NRoot.hpp"
#include "ui/core/NTree.hpp"
#include "ui/core/ThreadManager.hpp"

#include "ui/core/NPlugin.hpp"
#include "ui/core/CNodeBuilders.hpp"

#include "ui/uicommon/ComponentNames.hpp"

#include "common/XML/FileOperations.hpp"

#include "ui/core/TreeThread.hpp"

using namespace cf3::common;
using namespace cf3::common::XML;
using namespace cf3::ui::uiCommon;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace core {

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
    delete m_mutex;
  }
}

////////////////////////////////////////////////////////////////////////////

void TreeThread::set_mutex(QMutex* mutex)
{
  m_mutex = mutex;
}


void TreeThread::run()
{
  m_root = boost::shared_ptr< NRoot >(new NRoot("Root"));

  Handle< NRoot > realRoot(m_root);

  boost::shared_ptr< NLog > log(new NLog());
  boost::shared_ptr< NBrowser > browser(new NBrowser());
  boost::shared_ptr< NTree > tree(new NTree(realRoot));
  boost::shared_ptr< NPlugins > plugins(new NPlugins(CLIENT_PLUGINS));
  boost::shared_ptr< NGeneric > uidir( new NGeneric( CLIENT_UI_DIR, "cf3.common.Group", CNode::LOCAL_NODE ) );
  boost::shared_ptr< NetworkQueue > networkQueue( new NetworkQueue() );

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
  tree->set_tree_root(realRoot);

  ThreadManager::instance().network().signal( "network_new_frame" )
      ->connect( boost::bind(&TreeThread::new_signal, this, _1) );


//  ThreadManager::instance().network().newSignal.connect(
//      boost::bind(&TreeThread::new_signal, this, _1) );

  m_mutex->unlock();
//  m_waitCondition.wakeAll();

  // execute the event loop
  exec();
}

////////////////////////////////////////////////////////////////////////////

void TreeThread::new_signal( SignalArgs & args)
{
  const char * tag = Protocol::Tags::node_frame();
//  XmlNode nodedoc = *doc.xml_doc.get();
//  rapidxml::xml_node<char>* nodeToProcess = nodedoc.content->first_node(tag);

  NLog::global()->add_message("In TreeThread::new_signal()");

    if( args.node.is_valid() )
    {
      SignalFrame real_frame;

      if( args.has_reply() )
        real_frame = args.get_reply();
      else
        real_frame = args;

    std::string type = real_frame.node.attribute_value("target");
    std::string receiver = real_frame.node.attribute_value("receiver");

      try
      {
        Handle< Component > realRoot = root();
//        SignalFrame frame(nodeToProcess);

        if(realRoot->uri().path() == URI(receiver).path())
          root()->call_signal(type, real_frame);
        else
          realRoot->access_component(receiver)->call_signal(type, real_frame);
      }
      catch(cf3::common::Exception & cfe)
      {
        NLog::global()->add_exception(/*QString("%1 %2").arg(type.c_str()).arg(receiver.c_str()) +  */cfe.what());
      }
      catch(std::exception & stde)
      {
        NLog::global()->add_exception(stde.what());
      }
      catch(...)
      {
        CFerror << "Unknown exception thrown during execution of action [" << type
            << "] on component " << " [" << receiver << "]." << CFendl;
      }

    }

//  if(nodeToProcess != nullptr)
//  {
//    rapidxml::xml_node<>* tmpNode = nodeToProcess->next_sibling( tag );

//    // check this is a reply
//    if(tmpNode != nullptr && std::strcmp(tmpNode->first_attribute("type")->value(), "reply") == 0)
//      nodeToProcess = tmpNode;

//    std::string type = nodeToProcess->first_attribute("target")->value();
//    std::string receiver = nodeToProcess->first_attribute("receiver")->value();

//    try
//    {
//      Handle< Component > realRoot = root();
//      SignalFrame frame(nodeToProcess);

//      if(realRoot->uri().path() == URI(receiver).path())
//        root()->call_signal(type, frame);
//      else
//        realRoot->access_component(receiver)->call_signal(type, frame);
//    }
//    catch(cf3::common::Exception & cfe)
//    {
//      NLog::global()->add_exception(/*QString("%1 %2").arg(type.c_str()).arg(receiver.c_str()) +  */cfe.what());
//    }
//    catch(std::exception & stde)
//    {
//      NLog::global()->add_exception(stde.what());
//    }
//    catch(...)
//    {
//      CFerror << "Unknown exception thrown during execution of action [" << type
//          << "] on component " << " [" << receiver << "]." << CFendl;
//    }

//  }

}

////////////////////////////////////////////////////////////////////////////

} // Core
} // ui
} // cf3
