// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QtCore>
#include <string>
#include <cstring>

#include "Common/XML/FileOperations.hpp"

#include "GUI/Client/Core/NBrowser.hpp"
#include "GUI/Client/Core/NLog.hpp"
#include "GUI/Client/Core/NRoot.hpp"
#include "GUI/Client/Core/NTree.hpp"
#include "GUI/Client/Core/NCore.hpp"

#include "GUI/Client/Core/ProcessingThread.hpp"

#include "GUI/Network/ComponentNames.hpp"

#include "GUI/Client/Core/ClientRoot.hpp"

using namespace CF::Common;
using namespace CF::Common::XML;
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
    m_root(new NRoot(CLIENT_ROOT))
{
  CRoot::Ptr realRoot = m_root->root();

  NLog::Ptr log(new NLog());
  NBrowser::Ptr browser(new NBrowser());
  NTree::Ptr tree(new NTree(m_root));
  NCore::Ptr core(new NCore());

  // add components to the root
  realRoot->add_component(core);
  realRoot->add_component(log);
  realRoot->add_component(browser);
  realRoot->add_component(tree);

  // mark all components as basic
  m_root->mark_basic();
  core->mark_basic();
  log->mark_basic();
  browser->mark_basic();
  tree->mark_basic();

  // set the root as model root
  tree->setRoot(m_root);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ClientRoot::processSignalString(const QString & signal)
{
  XmlDoc::Ptr xmldoc = XML::parse_string( signal.toStdString() );

  ProcessingThread * pt = new ProcessingThread(xmldoc);

  m_threads[pt] = xmldoc;

  m_currentDocs[xmldoc->content] = xmldoc;

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
    m_currentDocs.remove( m_threads[pt]->content );
    m_threads.remove(pt);

    delete pt;
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

boost::shared_ptr<XmlDoc> ClientRoot::docFromPtr(const XmlDoc *doc) const
{
  if(m_currentDocs.contains(doc->content))
    return m_currentDocs[doc->content];

  return boost::shared_ptr<XmlDoc>();
}

////////////////////////////////////////////////////////////////////////////

} // ClientCore
} // GUI
} // CF
