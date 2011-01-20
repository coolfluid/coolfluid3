// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <sstream>

#include <QFileIconProvider>

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "Common/CF.hpp"
#include "Common/URI.hpp"

#include "GUI/Client/Core/ClientRoot.hpp"
#include "GUI/Client/Core/NRoot.hpp"

using namespace CF::Common;
using namespace CF::GUI::ClientCore;

NRoot::NRoot(const QString & name)
  : CNode(name, "CRoot", ROOT_NODE),
    m_uuid(boost::uuids::random_generator()())
{
  regist_signal( "save_tree_local", "Saves the server component tree.", "Save server tree" )->connect( boost::bind(&NRoot::save_tree_local, this, _1));

  m_localSignals << "save_tree_local";

  m_root = CRoot::create(name.toStdString());
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString NRoot::toolTip() const
{
  return this->getComponentType();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CNode::Ptr NRoot::childFromRoot(CF::Uint number) const
{
  ComponentIterator<CNode> it = m_root->begin<CNode>();
  CF::Uint i;

  for(i = 0 ; i < number && it != m_root->end<CNode>() ; i++)
    it++;

  // if number is bigger than the map size, it is equal to end()
  cf_assert(it != m_root->end<CNode>());

  return it.get();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool NRoot::pathExists() const
{
  return false;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

std::string NRoot::uuid() const
{
  std::ostringstream ss;
  ss << m_uuid;
  return ss.str();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NRoot::save_tree_local ( Common::XmlNode & node )
{
  if( !ClientRoot::instance().core()->isConnected() )
    ClientRoot::instance().log()->addError("The client needs to be connected to a server to do that.");
  else
  {
    boost::shared_ptr<XmlDoc> root = XmlOps::create_doc();
    XmlNode * docNode = XmlOps::goto_doc_node(*root.get());
    XmlNode * frameNode;
    XmlNode * mapNode;

    frameNode = XmlOps::add_signal_frame(*docNode, "save_tree", CLIENT_ROOT_PATH,
                                         SERVER_ROOT_PATH, true);

    mapNode = XmlOps::add_node_to(*frameNode, XmlParams::tag_node_map());

    XmlParams(*frameNode).add_option("filename", URI("./server-tree.xml", URI::Scheme::FILE));

    ClientRoot::instance().core()->sendSignal(*root.get());
  }
}
