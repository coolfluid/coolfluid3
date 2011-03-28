// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QMutex>

#include "rapidxml/rapidxml.hpp"

#include "Common/Log.hpp"

#include "Common/XML/Protocol.hpp"

#include "UI/Core/NBrowser.hpp"
#include "UI/Core/NetworkThread.hpp"
#include "UI/Core/NLog.hpp"
#include "UI/Core/NRoot.hpp"
#include "UI/Core/NTree.hpp"
#include "UI/Core/ThreadManager.hpp"


#include "UI/UICommon/ComponentNames.hpp"

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

  // add components to the root
  realRoot->add_component(log);
  realRoot->add_component(browser);
  realRoot->add_component(tree);

  // mark all components as basic
  m_root->mark_basic();
  log->mark_basic();
  browser->mark_basic();
  tree->mark_basic();

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

      if(realRoot->full_path().path() == URI(receiver).path())
        root()->call_signal(type, frame);
      else
        realRoot->retrieve_component(receiver)->call_signal(type, frame);
    }
    catch(CF::Common::Exception & cfe)
    {
      NLog::globalLog()->addException(cfe.what());
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
