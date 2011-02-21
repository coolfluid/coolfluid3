// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QtCore>
#include <string>
#include <cstring>

#include "Common/XmlHelpers.hpp"

#include "GUI/Client/Core/NCore.hpp"
#include "GUI/Client/Core/ProcessingThread.hpp"

#include "GUI/Network/ComponentNames.hpp"

#include "GUI/Client/Core/ClientRoot.hpp"

using namespace CF::Common;
using namespace CF::GUI::Network;

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace ClientCore {

////////////////////////////////////////////////////////////////////////////

ClientRoot & ClientRoot::instance()
{
  static ClientRoot cr;
  return cr;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

ClientRoot::ClientRoot() :
    m_root(new NRoot(CLIENT_ROOT)),
    m_log(new NLog()),
    m_browser(new NBrowser()),
    m_tree(new NTree(m_root)),
    m_core(new NCore())
{
  CRoot::Ptr realRoot = m_root->root();

  // add components to the root
  realRoot->add_component(m_core);
  realRoot->add_component(m_log);
  realRoot->add_component(m_browser);
  realRoot->add_component(m_tree);

  // mark all components as basic
  m_root->mark_basic();
  m_core->mark_basic();
  m_log->mark_basic();
  m_browser->mark_basic();
  m_tree->mark_basic();

  // set the root as model root
  m_tree->setRoot(m_root);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ClientRoot::processSignalString(const QString & signal)
{
  boost::shared_ptr<XmlDoc> xmldoc = XmlOps::parse ( signal.toStdString() );

  std::string str;

  XmlOps::xml_to_string(*xmldoc.get(), str);

  ProcessingThread * pt = new ProcessingThread(xmldoc);

  m_threads[pt] = xmldoc;

  m_currentDocs[xmldoc.get()] = xmldoc;

  connect(pt, SIGNAL(finished()), this, SLOT(processingFinished()));

  pt->run();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ClientRoot::processingFinished()
{
  ProcessingThread * pt = static_cast<ProcessingThread*>(sender());

  if(pt != nullptr && m_threads.contains(pt))
  {
    m_currentDocs.remove( m_threads[pt].get() );
    m_threads.remove(pt);

    delete pt;
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

boost::shared_ptr<XmlDoc> ClientRoot::docFromPtr(const XmlDoc *doc) const
{
  if(m_currentDocs.contains(doc))
    return m_currentDocs[doc];

  return boost::shared_ptr<XmlDoc>();
}

////////////////////////////////////////////////////////////////////////////

} // ClientCore
} // GUI
} // CF
