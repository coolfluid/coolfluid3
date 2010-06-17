#include <QtCore>
#include <QtGui>

#include "Common/CF.hpp"

#include "GUI/Client/ClientRoot.hpp"
#include "GUI/Client/NGroup.hpp"
#include "GUI/Client/NLink.hpp"
#include "GUI/Client/NMesh.hpp"
#include "GUI/Client/NMethod.hpp"

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

//QList<NodeAction> CNode::getNodeActions() const
//{
//  static QList<NodeAction> list;

//  return list;
//}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//QIcon CNode::getIcon() const
//{
//  return QFileIconProvider().icon(QFileIconProvider::File);
//}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//QString CNode::getClassName() const
//{
//  return "CNode";
//}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//void CNode::setParams(const QDomNodeList & list)
//{
//  for(int i = 0 ; i < list.size() ; i++)
//  {
//    QDomElement elt = list.at(i).toElement();

//    if(!elt.isNull())
//    {
//      NodeParams np;

//      np.m_paramAdv = elt.attribute("mode") != "basic";
//      np.m_paramDescr = elt.attribute("desc");
//    }
//  }
//}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//CNode::Ptr CNode::createFromXml(const QDomDocument & doc)
//{
//  QDomElement elt = doc.firstChildElement();

//  cf_assert(!elt.isNull());
////  cf_assert(elt.nodeName() == "CRoot");

//  QString name = elt.attribute("name");

//  CNode::Ptr rootNode(new CNode(name, ""));

//  while(!elt.isNull())
//  {
//    rootNode->add_component( toTreeNode(elt) );
//    elt = elt.nextSiblingElement();
//  }

//  return rootNode;
//}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CNode::Ptr CNode::createFromXml(const QDomElement & element)
{
  QString type = element.nodeName();
  QString name = element.attribute("name");
  QDomElement child = element.firstChildElement();

  cf_assert(!name.isEmpty());

  CNode::Ptr node;

  if(type == "CLink")
    node = boost::shared_ptr<NLink>(new NLink(name));
  else if(type == "CMesh")
    node = boost::shared_ptr<NMesh>(new NMesh(name));
  else if(type == "CMethod")
    node = boost::shared_ptr<NMethod>(new NMethod(name));
  else if(type == "CGroup" || type == "CRoot")
    node = boost::shared_ptr<NGroup>(new NGroup(name));
  else
    throw ShouldNotBeHere(FromHere(), QString("%1: Unknown type").arg(type).toStdString().c_str());

  while(!child.isNull())
  {
    if(child.nodeName() == "params")
      node->setParams(child.childNodes());
    else
      node->add_component( createFromXml(child) );

    child = child.nextSiblingElement();
  }

  return node;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CNode::Ptr CNode::getNode(CF::Uint index)
{
  Component_iterator<CNode> it = this->recursive_begin<CNode>();
  CF::Uint i = 0;

  cf_assert(index < m_components.size());

  while(i < index)
  {
    it++;

    if(it->get_parent().get() == this)
      i++;
  }


  return boost::shared_ptr<CNode>(&(*it));
}
