// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QMenu>
#include <QPoint>
#include <QStringList>
#include <QVariant>
#include <QDebug>

#include <boost/filesystem/path.hpp>

#include <cstring>

#include "Common/CF.hpp"
#include "Common/XmlHelpers.hpp"
#include "Common/String/Conversion.hpp"

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
using namespace CF::Common::String;
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
    m_type(type),
    m_notifier(new CNodeNotifier()),
    m_componentType(componentType)
{
  BUILD_COMPONENT;

  regist_signal("configure", "Update component options")->connect(boost::bind(&CNode::configure, this, _1));

  m_property_list.add_property("originalComponentType", m_componentType.toStdString());

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CNode::defineConfigProperties ( CF::Common::PropertyList& props )
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

void CNode::setOptions(XmlNode & options)
{
  XmlParams p(options);

  if(p.option_map != CFNULL)
  {
    // iterate through options
    XmlNode* node = p.option_map->first_node();
    for ( ; node != CFNULL ; node = node->next_sibling(  ) )
    {
      bool advanced;
      XmlAttr * keyAttr= node->first_attribute( XmlParams::tag_attr_key() );
      XmlAttr * descrAttr = node->first_attribute( XmlParams::tag_attr_descr() );
      XmlAttr * modeAttr = node->first_attribute( "mode" );

      advanced = modeAttr != CFNULL && std::strcmp(modeAttr->value(), "adv") == 0;

      if ( keyAttr != CFNULL )
      {
        const char * keyVal = keyAttr->value(); // option name

        if(std::strcmp(node->name(), XmlParams::tag_node_value())  == 0)
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
            else if(std::strcmp(typeVal, "file") == 0)
              addOption<boost::filesystem::path>(keyVal, descrVal, *type_node);
            else
              throw ShouldNotBeHere(FromHere(), std::string(typeVal) + ": Unknown type parent is " + node->name());
          }
        }
        else if(std::strcmp(node->name(), "array")  == 0)
        {
          XmlAttr * typeAttr= node->first_attribute( XmlParams::tag_attr_type() );

          if( typeAttr != CFNULL)
          {
            const char * descrVal = (descrAttr != CFNULL) ? descrAttr->value() : "";
            const char * typeVal = typeAttr->value(); // element type

            if(std::strcmp(typeVal, "bool") == 0)
            {
              std::vector<bool> data = p.get_array<bool>(keyVal);
              m_property_list.add_option< OptionArrayT<bool> >(keyVal, descrVal, data);
            }
            else if(std::strcmp(typeVal, "integer") == 0)
            {
              std::vector<int> data = p.get_array<int>(keyVal);
              m_property_list.add_option< OptionArrayT<int> >(keyVal, descrVal, data);
            }
            else if(std::strcmp(typeVal, "unsigned") == 0)
            {
              std::vector<unsigned int> data = p.get_array<unsigned int>(keyVal);
              m_property_list.add_option< OptionArrayT<unsigned int> >(keyVal, descrVal, data);
            }
            else if(std::strcmp(typeVal, "real") == 0)
            {
              std::vector<CF::Real> data = p.get_array<CF::Real>(keyVal);
              m_property_list.add_option< OptionArrayT<CF::Real> >(keyVal, descrVal, data);
            }
            else if(std::strcmp(typeVal, "string") == 0)
            {
              std::vector<std::string> data = p.get_array<std::string>(keyVal);
              m_property_list.add_option< OptionArrayT<std::string> >(keyVal, descrVal, data);
            }
            else if(std::strcmp(typeVal, "file") == 0)
            {
              std::vector<boost::filesystem::path> data;
              data = p.get_array<boost::filesystem::path>(keyVal);
              m_property_list.add_option< OptionArrayT<boost::filesystem::path> >(keyVal, descrVal, data);
            }
            else
              throw ShouldNotBeHere(FromHere(), std::string(typeVal) + ": Unknown array type");
          }

        }

        if(!advanced)
          m_property_list[keyVal].as_option().mark_basic();
      }
    }
  }
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CNode::setProperties(XmlNode & options)
{
  XmlParams p(options);

  if(p.property_map != CFNULL)
  {
    // iterate through properties
    XmlNode* node = p.property_map->first_node();
    for ( ; node != CFNULL ; node = node->next_sibling(  ) )
    {
      XmlAttr * keyAttr= node->first_attribute( XmlParams::tag_attr_key() );

      if ( keyAttr != CFNULL )
      {
        const char * keyVal = keyAttr->value(); // option name

        if(std::strcmp(node->name(), XmlParams::tag_node_value())  == 0)
        {
          XmlNode * type_node = node->first_node();

          if( type_node != CFNULL)
          {
            const char * typeVal = type_node->name(); // type name

            if(m_property_list.check(keyVal))
            {
              if(std::strcmp(typeVal, "bool") == 0)
                configure_property(keyVal, from_str<bool>(type_node->value()));
              else if(std::strcmp(typeVal, "integer") == 0)
                configure_property(keyVal, from_str<int>(type_node->value()));
              else if(std::strcmp(typeVal, "unsigned") == 0)
                configure_property(keyVal, from_str<CF::Uint>(type_node->value()));
              else if(std::strcmp(typeVal, "real") == 0)
                configure_property(keyVal, from_str<CF::Real>(type_node->value()));
              else if(std::strcmp(typeVal, "string") == 0)
                configure_property(keyVal, std::string(type_node->value()));
              else if(std::strcmp(typeVal, "file") == 0)
                configure_property(keyVal, boost::filesystem::path(type_node->value()));
              else
                throw ShouldNotBeHere(FromHere(), std::string(typeVal) + ": Unknown type parent is " + node->name());
            }
            else
            {
              if(std::strcmp(typeVal, "bool") == 0)
                m_property_list.add_property(keyVal, from_str<bool>(type_node->value()));
              else if(std::strcmp(typeVal, "integer") == 0)
                m_property_list.add_property(keyVal, from_str<int>(type_node->value()));
              else if(std::strcmp(typeVal, "unsigned") == 0)
                m_property_list.add_property(keyVal, from_str<CF::Uint>(type_node->value()));
              else if(std::strcmp(typeVal, "real") == 0)
                m_property_list.add_property(keyVal, from_str<CF::Real>(type_node->value()));
              else if(std::strcmp(typeVal, "string") == 0)
                m_property_list.add_property(keyVal, std::string(type_node->value()));
              else if(std::strcmp(typeVal, "file") == 0)
                m_property_list.add_property(keyVal, boost::filesystem::path(type_node->value()));
              else
                throw ShouldNotBeHere(FromHere(), std::string(typeVal) + ": Unknown type parent is " + node->name());

            }
          }
        }
      }
    }
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

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
  else
  {
    boost::shared_ptr<XmlDoc> docnode = XmlOps::create_doc();
    XmlNode * rootNode = XmlOps::goto_doc_node(*docnode.get());
    XmlNode * signalNode = XmlOps::add_signal_frame(*rootNode, "configure", full_path(), full_path(), true);
    XmlParams p(*signalNode);
    bool valid = true;

    for ( ; it != options.end() ; it++)
    {
      Property * prop = &m_property_list[it.key().toStdString()].as_option();

      if( prop != CFNULL && strcmp( prop->tag() , "array" ) )
      {
        std::string name = it.key().toStdString();
        QString value = it.value();

        if(prop->type() == "bool")
          p.add_option(name, QVariant(value).toBool());
        else if(prop->type() == "integer")
          p.add_option(name, value.toInt());
        else if(prop->type() == "unsigned")
          p.add_option(name, value.toUInt());
        else if(prop->type() == "real")
          p.add_option(name, value.toDouble());
        else if(prop->type() == "string")
          p.add_option(name, value.toStdString());
        else if(prop->type() == "file")
          p.add_option(name, boost::filesystem::path(value.toStdString()));
        else
          throw ValueNotFound(FromHere(), prop->type() + ": Unknown type for option " + name );
      }
      else if( prop != CFNULL && !strcmp ( prop->tag() , "array" ))
      {
        OptionArray * optArray;
        QStringList list = it.value().split(":");
        QStringList::iterator itList = list.begin();

        optArray = static_cast<OptionArray*>(prop);

        cf_assert(optArray != CFNULL);

        const char * elemType = optArray->elem_type();

        if(std::strcmp(elemType, "file") == 0)
          ADD_ARRAY_TO_XML(boost::filesystem::path)
        else
          throw ValueNotFound(FromHere(), std::string(elemType) + ": Unknown type for option array " + optArray->name());
      }
    }

    if(valid)
      ClientRoot::core()->sendSignal(*docnode.get());
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
  Component::Ptr compo = shared_from_this();
  CF::Uint i;

  if(checkType(CNode::ROOT_NODE))
    compo = this->convertTo<NRoot>()->root();

  ComponentIterator<CNode> it = compo->begin<CNode>();

  cf_assert(index < compo->get_child_count());

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

void CNode::listChildPaths(QStringList & list, bool recursive, bool clientNodes) const
{
  ComponentIterator<const CNode> it = this->begin<const CNode>();
  ComponentIterator<const CNode> itEnd = this->end<const CNode>();

  if(this->checkType(ROOT_NODE))
  {
    CRoot::ConstPtr root = this->convertTo<const NRoot>()->root();
    it = root->begin<const CNode>();
    itEnd = root->end<const CNode>();

    if(it->get_child_count() > 0)
      list << QString(root->full_path().string().c_str()) + '/';
    else
      list << root->full_path().string().c_str();
  }

  for( ; it != itEnd ; it++)
  {
    if(!it->isClientComponent() || clientNodes)
    {
      if(it->get_child_count() > 0)
        list << QString(it->full_path().string().c_str()) + '/';
      else
        list << it->full_path().string().c_str();
    }

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
  ClientRoot::tree()->optionsChanged(this->full_path());
  ClientRoot::log()->addMessage(QString("Node \"%1\" options updated.").arg(full_path().string().c_str()));
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CNode::getOptions(QList<NodeOption> & options) const
{
  PropertyList::PropertyStorage_t::const_iterator it = m_property_list.m_properties.begin();

  options.clear();

  for( ; it != m_property_list.m_properties.end() ; it++)
  {
    if(it->second->is_option())
    {
      Option::Ptr opt = boost::dynamic_pointer_cast<Option>(it->second);
      bool success = true;
      NodeOption nodeOpt;
      //OptionType::Type optionType = OptionType::Convert::to_enum(opt->type());

      nodeOpt.m_paramAdv= !opt->has_tag("basic");
      nodeOpt.m_paramName = it->first.c_str();
      nodeOpt.m_paramValue = opt->value_str().c_str();
      nodeOpt.m_paramDescr = opt->description().c_str();

      if(std::strcmp(opt->tag(), "array") != 0)
        nodeOpt.m_paramType = OptionType::Convert::to_enum(opt->type());
      else
      {
        boost::shared_ptr<OptionArray> optArray;
        optArray = boost::dynamic_pointer_cast<OptionArray>(opt);

        if(std::strcmp(optArray->elem_type(), "file") == 0)
          nodeOpt.m_paramType = OptionType::TYPE_FILES;
        else
        {
          success = false;
          ClientRoot::log()->addError(QString("Unable to process %1 option array")
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

void CNode::getProperties(QMap<QString, QString> & props) const
{
  PropertyList::PropertyStorage_t::const_iterator it = m_property_list.m_properties.begin();

  props.clear();

  for( ; it != m_property_list.m_properties.end() ; it++)
  {
    if(!it->second->is_option())
      props[ it->first.c_str() ] = it->second->value<std::string>().c_str();
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
    throw XmlError(FromHere(), QString("%1: Unknown type parent is %2").arg(nodeType).arg(node.parent()->name()).toStdString().c_str());

  while(child != CFNULL)
  {
    try
    {
      if(std::strcmp(child->name(), XmlParams::tag_node_valuemap()) == 0)
      {
        rootNode->setOptions(node);
        rootNode->setProperties(node);
      }
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
      ClientRoot::log()->addException(snbh.msg().c_str());
    }

    child = child->next_sibling();
  }

  return rootNode;
}

#undef ADD_ARRAY_TO_XML
