// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QtCore>

//#include "Common/Component"

#include "GUI/Client/Core/ClientRoot.hpp"

#include "GUI/Client/uTests/TreeHandler.hpp"

using namespace CF::Common;
using namespace CF::GUI::ClientCore;
using namespace CF::GUI::ClientTest;

TreeHandler::~TreeHandler()
{
  QStringList::iterator it = names.begin();

  for( ; it != names.end() ; it++)
    ClientRoot::tree()->treeRoot()->removeNode(*it);
}

void TreeHandler::add(CNode::Ptr node)
{
  ClientRoot::tree()->treeRoot()->addNode(node);
  names << node->name().c_str();
}

void TreeHandler::addChildren(CNode::Ptr node)
{
  ComponentIterator<CNode> it = node->begin<CNode>();
  ComponentIterator<CNode> itEnd = node->end<CNode>();

  if(node->checkType(CNode::ROOT_NODE))
  {
    it = node->castTo<NRoot>()->root()->begin<CNode>();
    itEnd = node->castTo<NRoot>()->root()->end<CNode>();
  }

  for( ; it != itEnd ; it++)
    add(it.get());
}
