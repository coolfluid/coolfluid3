#include <QtCore>
#include <QtGui>

#include "Common/CF.hpp"

#include "GUI/Client/ClientRoot.hpp"
#include "GUI/Client/NGroup.hpp"
#include "GUI/Client/NLink.hpp"
#include "GUI/Client/NMesh.hpp"
#include "GUI/Client/NMethod.hpp"
#include "GUI/Client/NRoot.hpp"

#include "GUI/Client/CNode.hpp"

using namespace CF::Common;
using namespace CF::GUI::Client;

CNode::CNode(const QString & name, const QString & componentType, CNode::Type type)
  : Component(name.toStdString()),
    m_componentType(componentType),
    m_contextMenu(new QMenu("Node")),
    m_type(type)
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

CNode::Type CNode::getType() const
{
  return m_type;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CNode::setTextData(const QString & text)
{
  m_textData = text;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CNode::setOptions(const QDomNodeList & list)
{
  for(int i = 0 ; i < list.size() ; i++)
  {
    QDomElement elt = list.at(i).toElement();

    if(!elt.isNull())
    {
      NodeOption np;

      np.m_paramAdv = elt.attribute("mode") != "basic";
      np.m_paramType = OptionType::Convert::to_enum(elt.nodeName().toStdString());
      np.m_paramName = elt.attribute("key");
      np.m_paramDescr = elt.attribute("desc");
      np.m_paramValue = elt.firstChild().toText().nodeValue();

      m_options.append(np);
    }
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CNode::Ptr CNode::createFromXml(const QDomElement & element)
{
  QString type = element.nodeName();
  QString name = element.attribute("name");
  QDomElement child = element.firstChildElement();

  cf_assert(!name.isEmpty());

  CNode::Ptr rootNode;

  if(type == "CLink")
  {
    QDomText targetPath = element.firstChild().toText();
    rootNode = boost::shared_ptr<NLink>(new NLink(name, targetPath.nodeValue().toStdString()));
  }
  else if(type == "CMesh")
    rootNode = boost::shared_ptr<NMesh>(new NMesh(name));
  else if(type == "CMethod")
    rootNode = boost::shared_ptr<NMethod>(new NMethod(name));
  else if(type == "CGroup")
    rootNode = boost::shared_ptr<NGroup>(new NGroup(name));
  else if(type == "CRoot")
    rootNode = boost::shared_ptr<NRoot>(new NRoot(name));
  else
    throw ShouldNotBeHere(FromHere(), QString("%1: Unknown type").arg(type).toStdString().c_str());

  while(!child.isNull())
  {
    if(child.nodeName() == "params")
      rootNode->setOptions(child.childNodes());
    else
      rootNode->add_component( createFromXml(child) );

    child = child.nextSiblingElement();
  }

  return rootNode;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CNode::Ptr CNode::getNode(CF::Uint index)
{
  ComponentIterator<CNode> it = this->begin<CNode>();

  cf_assert(index < m_components.size());

  for(CF::Uint i = 0 ; i < index ; i++)
    it++;

  return it.get();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QMenu * CNode::getContextMenu() const
{
  return m_contextMenu;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CNode::showContextMenu(const QPoint & pos) const
{
  m_contextMenu->exec(pos);
}
