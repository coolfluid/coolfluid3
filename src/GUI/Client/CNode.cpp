#include <QtCore>
#include <QtGui>

#include <cstring>

#include "Common/CF.hpp"
#include "Common/XmlHelpers.hpp"

#include "GUI/Client/ClientCore.hpp"
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
    m_contextMenu(new QMenu("Node")),
    m_type(type),
    m_componentType(componentType)
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

  while(option != CFNULL )
  {
    XmlNode * type = option->first_node();

    if(type != CFNULL && type->value() != CFNULL)
    {
      char * name = option->first_attribute("key")->value();

      if(name != CFNULL && std::strlen(name) > 0)
      {
        NodeOption np;
        char * modeStr = option->first_attribute("mode")->value();

        //np.m_paramAdv = !((modeStr != CFNULL) && (std::strcmp(modeStr, "basic") == 0));
        np.m_paramType = OptionType::Convert::to_enum(type->name());
        np.m_paramName = name;
        //np.m_paramDescr = elt.attribute("desc");
        np.m_paramValue = type->value();

        m_options[name] = np;
      }
    }

    option = option->next_sibling("value");
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CNode::modifyOptions(const QHash<QString, QString> options)
{
	QHash<QString, QString>::const_iterator it = options.begin();
	
	if(isClientComponent())
	{
		for ( ; it != options.end() ; it++) 
		{
			if(m_options.contains(it.key()))
				m_options[it.key()].m_paramValue = it.value();
		}		
	}
	else 
	{
		boost::shared_ptr<XmlDoc> docnode = XmlOps::create_doc();
		XmlNode * rootNode = XmlOps::goto_doc_node(*docnode.get());
		XmlNode * signalNode = XmlOps::add_signal_frame(*rootNode, "configure", full_path(), full_path());
		XmlParams p(*signalNode);
		bool valid = true;
		
		for ( ; it != options.end() ; it++) 
		{
			if(m_options.contains(it.key()))
			{
				switch (m_options[it.key()].m_paramType) 
				{
					case OptionType::TYPE_BOOL:
						p.add_param(it.key().toStdString(), QVariant(it.value()).toBool());
						break;
					case OptionType::TYPE_INT:
						p.add_param(it.key().toStdString(), QVariant(it.value()).toInt());
						break;
					case OptionType::TYPE_UNSIGNED_INT:
						p.add_param(it.key().toStdString(), QVariant(it.value()).toUInt());
						break;
					case OptionType::TYPE_DOUBLE:
						p.add_param(it.key().toStdString(), QVariant(it.value()).toDouble());
						break;
					case OptionType::TYPE_STRING:
						p.add_param(it.key().toStdString(), it.value().toStdString());
						break;
						
					default:
						ClientRoot::getLog()->addError(QString("%1: Unknown type id").arg(m_options[it.value()].m_paramType));
						valid = false;
						break;
				}				
			}
		}
		
		if(valid)
			ClientCore::instance().sendSignal(*docnode.get());		
	}
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
