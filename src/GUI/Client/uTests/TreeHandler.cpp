#include <QtCore>

//#include "Common/Component"

#include "GUI/Client/ClientRoot.hpp"

#include "GUI/Client/uTests/TreeHandler.hpp"

using namespace CF::Common;
using namespace CF::GUI::Client;
using namespace CF::GUI::ClientTest;

TreeHandler::~TreeHandler()
{
  QStringList::iterator it = names.begin();

  for( ; it != names.end() ; it++)
    ClientRoot::getTree()->getRoot()->removeNode(*it);
}

void TreeHandler::add(CNode::Ptr node)
{
  ClientRoot::getTree()->getRoot()->addNode(node);
  names << node->name().c_str();
}

void TreeHandler::addChildren(CNode::Ptr node)
{
  ComponentIterator<CNode> it = node->begin<CNode>();
  ComponentIterator<CNode> itEnd = node->end<CNode>();

  if(node->checkType(CNode::ROOT_NODE))
  {
    it = node->convertTo<NRoot>()->root()->begin<CNode>();
    itEnd = node->convertTo<NRoot>()->root()->end<CNode>();
  }

  for( ; it != itEnd ; it++)
    add(it.get());
}
