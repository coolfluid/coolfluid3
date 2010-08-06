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

bool NodeOption::operator==(const NodeOption & option)
{
  return m_paramAdv ^ option.m_paramAdv
      && m_paramName == option.m_paramName
      && m_paramType == option.m_paramType
      && m_paramValue == option.m_paramValue
      && m_paramDescr == option.m_paramDescr;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CNodeNotifier::CNodeNotifier(QObject * parent)
  : QObject(parent)
{ }

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CNodeNotifier::notifyChildCountChanged()
{
  emit childCountChanged();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CNode::CNode(const QString & name, const QString & componentType, CNode::Type type)
  : Component(name.toStdString()),
    m_contextMenu(new QMenu("Node")),
    m_notifier(new CNodeNotifier()),
    m_type(type),
    m_componentType(componentType)
{
  BUILD_COMPONENT;

  regist_signal("configure", "Update component options")->connect(boost::bind(&CNode::configure, this, _1));
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

void CNode::setOptions(const XmlNode & options)
{
  // iterate through options
  XmlNode* node = options.first_node( "value" );
  for ( ; node != CFNULL ; node = node->next_sibling( "value" ) )
  {
    XmlAttr * keyAttr= node->first_attribute( XmlParams::tag_attr_key() );
    XmlAttr * descrAttr = node->first_attribute( XmlParams::tag_attr_descr() );

    if ( keyAttr != CFNULL )
    {
      XmlNode * type_node = node->first_node();

      if( type_node != CFNULL)
      {
        const char * descrVal = (descrAttr != CFNULL) ? descrAttr->value() : "";
        const char * keyVal = keyAttr->value(); // option name
        const char * typeVal = type_node->name(); // type name

        if(std::strcmp(typeVal, "bool") == 0)
          addOption<bool>(keyVal, descrVal, *type_node);
        else if(std::strcmp(typeVal, "int") == 0)
          addOption<int>(keyVal, descrVal, *type_node);
        else if(std::strcmp(typeVal, "unsigned") == 0)
          addOption<CF::Uint>(keyVal, descrVal, *type_node);
        else if(std::strcmp(typeVal, "double") == 0)
          addOption<CF::Real>(keyVal, descrVal, *type_node);
        else if(std::strcmp(typeVal, "string") == 0)
          addOption<std::string>(keyVal, descrVal, *type_node);
        else
          throw ShouldNotBeHere(FromHere(), std::string(typeVal) + ": Unknown type");
      }
    }
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
//			if(m_options.contains(it.key()))
//				m_options[it.key()].m_paramValue = it.value();
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
      Option::Ptr option = m_option_list.getOption(it.key().toStdString());

      if(option->type() == "bool")
        p.add_param(it.key().toStdString(), QVariant(it.value()).toBool());
      else if(option->type() == "int")
        p.add_param(it.key().toStdString(), QVariant(it.value()).toInt());
      else if(option->type() == "unsigned")
        p.add_param(it.key().toStdString(), QVariant(it.value()).toUInt());
      else if(option->type() == "double")
        p.add_param(it.key().toStdString(), QVariant(it.value()).toDouble());
      else if(option->type() == "string")
        p.add_param(it.key().toStdString(), it.value().toStdString());
      else
        throw ValueNotFound(FromHere(), std::string(option->type()) + ": Unknown type id");
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
    try
    {
      if(std::strcmp(child->name(), XmlParams::tag_node_valuemap()) == 0)
        rootNode->setOptions(*child);
      else
      {
        CNode::Ptr node = createFromXml(*child);

        if(node.get() != CFNULL)
        {
          if(rootNode->checkType(ROOT_NODE))
            convertTo<NRoot>(rootNode)->root()->add_component(node);
          else
            rootNode->add_component(node);
        }
      }
    }
    catch (ShouldNotBeHere & snbh)
    {
      ClientRoot::getLog()->addException(snbh.msg().c_str());
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

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CNode::connectNotifier(QObject * reciever, const char * signal, const char * slot)
{
  QObject::connect(m_notifier, signal, reciever, slot);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CNodeNotifier * CNode::getNotifier() const
{
  return m_notifier;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CNode::addNode(CNode::Ptr node)
{
  try
  {
    if(checkType(ROOT_NODE))
      ((NRoot *)this)->root()->add_component(node);
    else
      this->add_component(node);

    m_notifier->notifyChildCountChanged();
  }
  catch(CF::Common::ValueExists & ve)
  {
    throw;
  }

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CNode::configure(CF::Common::XmlNode & node)
{
  ConfigObject::configure(node);
  ClientRoot::getLog()->addMessage(QString("Node \"%1\" options updated.").arg(full_path().string().c_str()));
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CNode::getOptions(QList<NodeOption> & options) const
{
  if(m_type == LINK_NODE)
  {
    CPath path = ((NLink*)this)->getTargetPath();

    CNode::Ptr target = ClientRoot::getTree()->getNodeByPath(path);

    if(target.get() != CFNULL)
      target->getOptions(options);
    else
      throw InvalidPath(FromHere(), path.string() + ": path does not exist");
  }
  else
  {
    OptionList::OptionStorage_t::const_iterator it = m_option_list.m_options.begin();

    options.clear();

    for( ; it != m_option_list.m_options.end() ; it++)
    {
      NodeOption nodeOpt;

      nodeOpt.m_paramName = it->first.c_str();
      nodeOpt.m_paramValue = it->second->value_str().c_str();
      nodeOpt.m_paramDescr = it->second->description().c_str();
      nodeOpt.m_paramType = OptionType::Convert::to_enum(it->second->type());

      options.append(nodeOpt);
    }
  }
}
