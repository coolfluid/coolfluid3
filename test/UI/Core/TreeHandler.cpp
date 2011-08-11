// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QtCore>

//#include "Common/Component"

#include "UI/Core/NTree.hpp"

#include "test/UI/Core/TreeHandler.hpp"

using namespace CF::Common;
using namespace CF::UI::Core;
using namespace CF::UI::CoreTest;

TreeHandler::~TreeHandler()
{
  QStringList::iterator it = names.begin();

  for( ; it != names.end() ; it++)
    NTree::globalTree()->treeRoot()->removeNode(*it);
}

void TreeHandler::add(CNode::Ptr node)
{
  NTree::globalTree()->treeRoot()->addNode(node);
  names << node->name().c_str();
}

void TreeHandler::addChildren(CNode::Ptr node)
{
  ComponentIterator<CNode> it = node->realComponent()->begin<CNode>();
  ComponentIterator<CNode> itEnd = node->realComponent()->end<CNode>();

  for( ; it != itEnd ; it++)
    add(it.get());
}
