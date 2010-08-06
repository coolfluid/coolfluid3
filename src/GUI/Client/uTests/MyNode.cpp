#include <QtCore>
#include <QtGui>

#include "GUI/Client/uTests/MyNode.hpp"

using namespace CF::GUI::ClientTest;

MyNode::MyNode(const QString & name)
  : CNode(name, "CGroup", CNode::GROUP_NODE)
{
  BUILD_COMPONENT;
}

////////////////////////////////////////////////////////////////////////////

QIcon MyNode::getIcon() const
{
  return QFileIconProvider().icon(QFileIconProvider::File);
}

////////////////////////////////////////////////////////////////////////////

QString MyNode::getToolTip() const
{
  return this->getComponentType();
}

////////////////////////////////////////////////////////////////////////////

