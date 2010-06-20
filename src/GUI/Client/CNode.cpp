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

void CNode::setTextData(const QString & text)
{
  m_textData = text;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CNode::setParams(const QDomNodeList & list)
{
  for(int i = 0 ; i < list.size() ; i++)
  {
    QDomElement elt = list.at(i).toElement();

    if(!elt.isNull())
    {
      NodeParams np;

      np.m_paramAdv = elt.attribute("mode") != "basic";
      np.m_paramType = OptionType::Convert::to_enum(elt.nodeName().toStdString());
      np.m_paramName = elt.attribute("key");
      np.m_paramDescr = elt.attribute("desc");
      np.m_paramValue = elt.firstChild().toText().nodeValue();

      m_params.append(np);
    }
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CNode::getParams(QList<NodeParams> & params) const
{
  params = m_params;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CNode::Ptr CNode::createFromXml(const QDomElement & element)
{
  QString type = element.nodeName();
  QString name = element.attribute("name");
  QDomNode child = element.firstChild();

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
    if(child.nodeType() == QDomNode::ElementNode)
    {
      if(child.nodeName() == "params")
        node->setParams(child.childNodes());
      else
        node->add_component( createFromXml(child.toElement()) );
    }
    else if(child.nodeType() == QDomNode::TextNode)
      node->setTextData(child.nodeValue());

    child = child.nextSibling();
  }

  return node;
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
