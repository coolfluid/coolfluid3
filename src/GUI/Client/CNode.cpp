#include <QtCore>
#include <QtGui>

#include <cstring>

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
  BUILD_COMPONENT;
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

void CNode::setOptions(const XmlNode & node)
{
  XmlNode * option = node.first_node("value");

  qDebug() << __LINE__ << option;

  while(option != CFNULL)
  {
    XmlNode * type = option->first_node();


    qDebug() << __LINE__ << option->name() << type;

    if(type != CFNULL && type->value() != CFNULL)
    {
      char * name = option->first_attribute("key")->value();

      if(name != CFNULL && std::strlen(name) > 0)
      {
        NodeOption np;
        char * modeStr = option->first_attribute("mode")->value();

        qDebug() << __LINE__ << option->name() << name;

//        np.m_paramAdv = !((modeStr != CFNULL) && (std::strcmp(modeStr, "basic") == 0));
        np.m_paramType = OptionType::Convert::to_enum(type->name());
        np.m_paramName = name;
        //np.m_paramDescr = elt.attribute("desc");
        np.m_paramValue = type->value();

        qDebug() << type->name() << np.m_paramType << type->name();

        m_options.append(np);
      }
    }

    option = option->next_sibling("value");
  }


//  for(int i = 0 ; i < list.size() ; i++)
//  {
//    QDomElement elt = list.at(i).toElement();

//    if(!elt.isNull())
//    {
//      NodeOption np;

//      np.m_paramAdv = elt.attribute("mode") != "basic";
//      np.m_paramType = OptionType::Convert::to_enum(elt.firstChildElement().nodeName().toStdString());
//      np.m_paramName = elt.attribute("key");
//      np.m_paramDescr = elt.attribute("desc");
//      np.m_paramValue = elt.firstChildElement().firstChild().toText().nodeValue();

//      m_options.append(np);
//    }
//  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CNode::Ptr CNode::createFromXml(CF::Common::XmlNode & node)
{
  char * nodeType = node.name();
  char * nodeName = node.first_attribute("name")->value();
  XmlNode * child = node.first_node();

  cf_assert(nodeType != CFNULL);
  cf_assert(nodeName != CFNULL);

  CNode::Ptr rootNode;

  if(std::strcmp(nodeType, "CCore") == 0 || std::strcmp(nodeType, "CSimulator") == 0)
    return rootNode;

  if(std::strcmp(nodeType, "CLink") == 0)
    rootNode = boost::shared_ptr<NLink>(new NLink(nodeName, node.value()));
  else if(std::strcmp(nodeType, "CMesh") == 0)
    rootNode = boost::shared_ptr<NMesh>(new NMesh(nodeName));
  else if(std::strcmp(nodeType, "CMethod") == 0)
    rootNode = boost::shared_ptr<NMethod>(new NMethod(nodeName));
  else if(std::strcmp(nodeType, "CGroup") == 0)
    rootNode = boost::shared_ptr<NGroup>(new NGroup(nodeName));
  else if(std::strcmp(nodeType, "CRoot") == 0)
    rootNode = boost::shared_ptr<NRoot>(new NRoot(nodeName));
  else
    throw XmlError(FromHere(), QString("%1: Unknown type").arg(nodeType).toStdString().c_str());

  while(child != CFNULL)
  {
    if(std::strcmp(child->name(), "valuemap") == 0)
      rootNode->setOptions(*child);
    else
    {
      CNode::Ptr node = createFromXml(*child);

      if(node.get() != CFNULL)
        rootNode->add_component(node);
    }
    child = child->next_sibling();
  }

  return rootNode;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CNode::Ptr CNode::getNode(CF::Uint index)
{
  ComponentIterator<CNode> it = this->begin<CNode>();
CF::Uint i;
  cf_assert(index < m_components.size());

  for(i = 0 ; i < index ; i++)
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
