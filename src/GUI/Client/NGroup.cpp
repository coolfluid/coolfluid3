#include <QtCore>
#include <QtGui>

#include "GUI/Client/NGroup.hpp"

using namespace CF::GUI::Client;

NGroup::NGroup(const QString & name) :
    CNode(name, "CGroup")
{

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

NGroup::NGroup(const QDomElement & node) :
    CNode(node.attribute("name"), "CGroup")
{

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QList<NodeAction> NGroup::getNodeActions() const
{
  static QList<NodeAction> list;

  return list;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QIcon NGroup::getIcon() const
{
  return QFileIconProvider().icon(QFileIconProvider::Folder);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString NGroup::getClassName() const
{
  return "NGroup";
}

void NGroup::setParams(const QDomNodeList & list)
{

}
