#include <QtCore>
#include <QtGui>

#include "GUI/Client/ClientRoot.hpp"
#include "GUI/Client/NLink.hpp"

#include "GUI/Client/CNode.hpp"

using namespace CF::Common;
using namespace CF::GUI::Client;

CNode::CNode(const QString & name, const QString & componentType)
  : Component(name.toStdString()),
    m_componentType(componentType)
{

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString CNode::getComponentType() const
{
  return m_componentType;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int CNode::getNodeCount() const
{
  return m_components.size();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QList<NodeAction> CNode::getNodeActions() const
{
  static QList<NodeAction> list;

  return list;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QIcon CNode::getIcon() const
{
  return QIcon();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString CNode::getClassName() const
{
  return "CNode";
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CNode::setParams(const QDomNodeList & list)
{
  for(int i = 0 ; i < list.size() ; i++)
  {
    QDomElement elt = list.at(i).toElement();

    if(!elt.isNull())
    {
      NodeParams np;

      np.m_paramAdv = elt.attribute("mode") != "basic";
      np.m_paramDescr = elt.attribute("desc");
    }
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CNode::Ptr CNode::createFromXml(const QDomDocument & doc)
{
//  QDomElement elt = doc.firstChildElement();
//
//  cf_assert(!elt.isNull());
//  cf_assert(elt.nodeName() == "CRoot");
//
//  QString name = elt.attribute("name");
//
//  CNode::Ptr rootNode(new CNode(name, ""));
//
//  return rootNode;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CNode::Ptr CNode::toTreeNode(const QDomElement & node)
{
  QDomNodeList childNodes = node.childNodes();



}
