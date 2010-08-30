#include <QMenu>
#include <QPoint>
#include <QStringList>
#include <QVariant>

#include <cstring>

#include "Common/CF.hpp"
#include "Common/XmlHelpers.hpp"

#include "GUI/Client/ClientRoot.hpp"
#include "GUI/Client/NArray.hpp"
#include "GUI/Client/NCore.hpp"
#include "GUI/Client/NElements.hpp"
#include "GUI/Client/NGroup.hpp"
#include "GUI/Client/NLog.hpp"
#include "GUI/Client/NLink.hpp"
#include "GUI/Client/NMesh.hpp"
#include "GUI/Client/NMeshReader.hpp"
#include "GUI/Client/NMethod.hpp"
#include "GUI/Client/NRegion.hpp"
#include "GUI/Client/NRoot.hpp"
#include "GUI/Client/NTable.hpp"
#include "GUI/Client/NTree.hpp"

#include "GUI/Client/CNode.hpp"

#define ADD_ARRAY_TO_XML(type) { \
std::vector<type> data;\
boost::shared_ptr<OptionArrayT<type> > array;\
array = boost::dynamic_pointer_cast<OptionArrayT<type> >(array);\
        \
for( ; itList != list.end() ; itList++)\
  data.push_back(itList->toStdString());\
  \
p.add_array(it.key().toStdString(), data);\
}


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
  XmlNode* node = options.first_node();
  for ( ; node != CFNULL ; node = node->next_sibling(  ) )
  {
    XmlAttr * keyAttr= node->first_attribute( XmlParams::tag_attr_key() );
    XmlAttr * descrAttr = node->first_attribute( XmlParams::tag_attr_descr() );

    const char * keyVal = keyAttr->value(); // option name

    if ( keyAttr != CFNULL )
    {
      if(std::strcmp(node->name(), "value")  == 0)
      {
        XmlNode * type_node = node->first_node();

        if( type_node != CFNULL)
        {
          const char * descrVal = (descrAttr != CFNULL) ? descrAttr->value() : "";
          const char * typeVal = type_node->name(); // type name

          if(std::strcmp(typeVal, "bool") == 0)
            addOption<bool>(keyVal, descrVal, *type_node);
          else if(std::strcmp(typeVal, "integer") == 0)
            addOption<int>(keyVal, descrVal, *type_node);
          else if(std::strcmp(typeVal, "unsigned") == 0)
            addOption<CF::Uint>(keyVal, descrVal, *type_node);
          else if(std::strcmp(typeVal, "real") == 0)
            addOption<CF::Real>(keyVal, descrVal, *type_node);
          else if(std::strcmp(typeVal, "string") == 0)
            addOption<std::string>(keyVal, descrVal, *type_node);
          else
            throw ShouldNotBeHere(FromHere(), std::string(typeVal) + ": Unknown type");
        }
      }
      else if(std::strcmp(node->name(), "array")  == 0)
      {
        XmlParams p(*options.parent());

        XmlAttr * typeAttr= node->first_attribute( XmlParams::tag_attr_type() );

        if( typeAttr != CFNULL)
        {
          const char * descrVal = (descrAttr != CFNULL) ? descrAttr->value() : "";
          const char * typeVal = typeAttr->value(); // element type

          if(std::strcmp(typeVal, "bool") == 0)
          {
            std::vector<bool> data = p.get_array<bool>(keyVal);
            m_option_list.add< OptionArrayT<bool> >(keyVal, descrVal, data);
          }
          else if(std::strcmp(typeVal, "integer") == 0)
          {
            std::vector<int> data = p.get_array<int>(keyVal);
            m_option_list.add< OptionArrayT<int> >(keyVal, descrVal, data);
          }
          else if(std::strcmp(typeVal, "unsigned") == 0)
          {
            std::vector<unsigned int> data = p.get_array<unsigned int>(keyVal);
            m_option_list.add< OptionArrayT<unsigned int> >(keyVal, descrVal, data);
          }
          else if(std::strcmp(typeVal, "real") == 0)
          {
            std::vector<CF::Real> data = p.get_array<CF::Real>(keyVal);
            m_option_list.add< OptionArrayT<CF::Real> >(keyVal, descrVal, data);
          }
          else if(std::strcmp(typeVal, "string") == 0)
          {
            std::vector<std::string> data = p.get_array<std::string>(keyVal);
            m_option_list.add< OptionArrayT<std::string> >(keyVal, descrVal, data);
          }
          else
            throw ShouldNotBeHere(FromHere(), std::string(typeVal) + ": Unknown array type");
        }

      }
    }
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CNode::modifyOptions(const QMap<QString, QString> options)
{
  QMap<QString, QString>::const_iterator it = options.begin();

  if(isClientComponent())
  {
    for ( ; it != options.end() ; it++)
    {
//			if(m_options.contains(it.key()))
//				m_options[it.key()].m_paramValue = it.value();
    }
  }
  else if(m_type == LINK_NODE)
  {
    CPath path = ((NLink*)this)->getTargetPath();

    CNode::Ptr target = ClientRoot::getTree()->getNodeByPath(path);

    if(target.get() != CFNULL)
      target->modifyOptions(options);
    else
      throw InvalidPath(FromHere(), path.string() + ": path does not exist");
  }
  else
  {
    boost::shared_ptr<XmlDoc> docnode = XmlOps::create_doc();
    XmlNode * rootNode = XmlOps::goto_doc_node(*docnode.get());
    XmlNode * signalNode = XmlOps::add_signal_frame(*rootNode, "configure", full_path(), full_path(), true);
    XmlParams p(*signalNode);
    bool valid = true;

    for ( ; it != options.end() ; it++)
    {
      Option::Ptr option = m_option_list.getOption(it.key().toStdString());

      if(option->tag() != "array")
      {
        if(option->type() == "bool")
          p.add_param(it.key().toStdString(), QVariant(it.value()).toBool());
        else if(option->type() == "integer")
          p.add_param(it.key().toStdString(), QVariant(it.value()).toInt());
        else if(option->type() == "unsigned")
          p.add_param(it.key().toStdString(), QVariant(it.value()).toUInt());
        else if(option->type() == "real")
          p.add_param(it.key().toStdString(), QVariant(it.value()).toDouble());
        else if(option->type() == "string")
          p.add_param(it.key().toStdString(), it.value().toStdString());
        else
          throw ValueNotFound(FromHere(), option->type() + ": Unknown type for option " + option->name() );
      }
      else if(option->tag() == "array")
      {
        boost::shared_ptr<OptionArray> optArray;
        QStringList list = it.value().split(":");
        QStringList::iterator itList = list.begin();

        optArray = boost::dynamic_pointer_cast<OptionArray>(option);
        const char * elemType = optArray->elem_type();

        if(std::strcmp(elemType, "string") == 0)
          ADD_ARRAY_TO_XML(std::string)
        else
          throw ValueNotFound(FromHere(), std::string(elemType) + ": Unknown type for option array " + option->name());
      }
    }

    if(valid)
      ClientRoot::getCore()->sendSignal(*docnode.get());
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CNode::Ptr CNode::createFromXml(CF::Common::XmlNode & node)
{
  QMap<NLink::Ptr, CPath> linkTargets;
  QMap<NLink::Ptr, CPath>::iterator it;
  CNode::Ptr rootNode;

  rootNode = createFromXmlRec(node, linkTargets);

  it = linkTargets.begin();

  for( ; it != linkTargets.end() ; it++)
    it.key()->setTargetPath(it.value());

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

void CNode::listChildPaths(QStringList & list, bool recursive) const
{
  ComponentIterator<const CNode> it = this->begin<const CNode>();
  ComponentIterator<const CNode> itEnd = this->end<const CNode>();

  if(this->checkType(ROOT_NODE))
  {
    CRoot::ConstPtr root = this->convertTo<const NRoot>()->root();
    it = root->begin<const CNode>();
    itEnd = root->end<const CNode>();

    list << root->full_path().string().c_str();
  }

  for( ; it != itEnd ; it++)
  {
    list << it->full_path().string().c_str();

    if(recursive)
      it->listChildPaths(list, recursive);
  }
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

void CNode::removeNode(const QString & nodeName)
{
  try
  {
    if(checkType(ROOT_NODE))
      ((NRoot *)this)->root()->remove_component(nodeName.toStdString());
    else
      this->remove_component(nodeName.toStdString());

    m_notifier->notifyChildCountChanged();
  }
  catch(CF::Common::ValueNotFound & ve)
  {
    throw;
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CNode::configure(CF::Common::XmlNode & node)
{
  ConfigObject::configure(node);
  ClientRoot::getTree()->optionsChanged(this->full_path());
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
      bool success = true;
      NodeOption nodeOpt;
      OptionType::Type optionType = OptionType::Convert::to_enum(it->second->type());

      nodeOpt.m_paramAdv= true;
      nodeOpt.m_paramName = it->first.c_str();
      nodeOpt.m_paramValue = it->second->value_str().c_str();
      nodeOpt.m_paramDescr = it->second->description().c_str();

      if(optionType != OptionType::INVALID)
        nodeOpt.m_paramType = OptionType::Convert::to_enum(it->second->type());
      else
      {
        boost::shared_ptr<OptionArray> optArray;
        optArray = boost::dynamic_pointer_cast<OptionArray>(it->second);

        if(std::strcmp(optArray->elem_type(), "string") == 0)
          nodeOpt.m_paramType = OptionType::TYPE_FILES;
        else
        {
          success = false;
          ClientRoot::getLog()->addError(QString("Unable to process %1 option array")
                                         .arg(optArray->elem_type()));
        }
      }

      if(success)
        options.append(nodeOpt);
    }
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CNode::Ptr CNode::createFromXmlRec(XmlNode & node, QMap<NLink::Ptr, CPath> & linkTargets)
{
  char * nodeType = node.name();
  char * nodeName = node.first_attribute("name")->value();
  XmlNode * child = node.first_node();

  cf_assert(nodeType != CFNULL);
  cf_assert(nodeName != CFNULL);
  cf_assert(std::strlen(nodeType) > 0);

  CNode::Ptr rootNode;

  if(std::strcmp(nodeType, "CCore") == 0 || std::strcmp(nodeType, "CSimulator") == 0)
    return rootNode;

  if(std::strcmp(nodeType, "CLink") == 0)
  {
    NLink::Ptr link = boost::shared_ptr<NLink>(new NLink(nodeName));
    rootNode = link;
    linkTargets[link] = node.value();
  }
  else if(std::strcmp(nodeType, "CMesh") == 0)
    rootNode = boost::shared_ptr<NMesh>(new NMesh(nodeName));
  else if(std::strcmp(nodeType, "CReader") == 0)
    rootNode = boost::shared_ptr<NMeshReader>(new NMeshReader(nodeName));
  else if(std::strcmp(nodeType, "CMethod") == 0)
    rootNode = boost::shared_ptr<NMethod>(new NMethod(nodeName));
  else if(std::strcmp(nodeType, "CGroup") == 0)
    rootNode = boost::shared_ptr<NGroup>(new NGroup(nodeName));
  else if(std::strcmp(nodeType, "CArray") == 0)
    rootNode = boost::shared_ptr<NArray>(new NArray(nodeName));
  else if(std::strcmp(nodeType, "CTable") == 0)
    rootNode = boost::shared_ptr<NTable>(new NTable(nodeName));
  else if(std::strcmp(nodeType, "CRegion") == 0)
    rootNode = boost::shared_ptr<NRegion>(new NRegion(nodeName));
  else if(std::strcmp(nodeType, "CElements") == 0)
    rootNode = boost::shared_ptr<NElements>(new NElements(nodeName));
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
        CNode::Ptr node = createFromXmlRec(*child, linkTargets);

        if(node.get() != CFNULL)
        {
          if(rootNode->checkType(ROOT_NODE))
            rootNode->convertTo<NRoot>()->root()->add_component(node);
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

#undef ADD_ARRAY_TO_XML
