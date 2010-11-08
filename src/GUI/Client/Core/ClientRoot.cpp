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

#include "GUI/Network/ComponentNames.hpp"

#include "GUI/Client/Core/ClientRoot.hpp"

using namespace CF::Common;
using namespace CF::GUI::ClientCore;
using namespace CF::GUI::Network;

NRoot::Ptr ClientRoot::root()
{
  static bool rootCreated = false;
  static NRoot::Ptr root(new NRoot(CLIENT_ROOT));

  static NLog::Ptr log(new NLog());
  static NBrowser::Ptr browser(new NBrowser());
  static NTree::Ptr tree(new NTree(root));
  static NCore::Ptr core(new NCore());

  // if the function is called for the first time, we add the components
  if(!rootCreated)
  {
    rootCreated = true;
    root->root()->add_component(core);
    root->root()->add_component(log);
    root->root()->add_component(browser);
    root->root()->add_component(tree);

    root->mark_basic();

    tree->setRoot(root);
  }

  return root;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ClientRoot::processSignal(const QDomDocument & signal)
{
  boost::shared_ptr<XmlDoc> xmldoc = XmlOps::parse ( signal.toString().toStdString() );

  XmlNode& nodedoc = *XmlOps::goto_doc_node(*xmldoc.get());
  XmlNode * nodeToProcess = nodedoc.first_node(XmlParams::tag_node_frame());

  if(nodeToProcess != nullptr)
  {
    XmlNode * tmpNode = nodeToProcess->next_sibling(XmlParams::tag_node_frame());

    // check this is a reply
    if(tmpNode != nullptr && std::strcmp(tmpNode->first_attribute("type")->value(), "reply") == 0)
      nodeToProcess = tmpNode;

    std::string type = nodeToProcess->first_attribute("target")->value();
    std::string receiver = nodeToProcess->first_attribute("receiver")->value();

    try
    {
      if(root()->root()->full_path().string() == receiver)
        root()->call_signal(type, *nodeToProcess);
      else
        root()->root()->access_component(receiver)->call_signal(type, *nodeToProcess);
    }
    catch(InvalidPath ip)
    {
      log()->addException(ip.what());
    }
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ClientRoot::processSignalString(const QString & signal)
{
  QDomDocument doc;

  doc.setContent(signal);
  processSignal(doc);
}
