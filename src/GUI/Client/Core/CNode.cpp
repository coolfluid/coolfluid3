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
#include "Common/OptionURI.hpp"
#include "Common/XmlHelpers.hpp"
#include "Common/String/Conversion.hpp"

#include "GUI/Client/Core/ClientRoot.hpp"
#include "GUI/Client/Core/NCore.hpp"
#include "GUI/Client/Core/NGeneric.hpp"
#include "GUI/Client/Core/NLog.hpp"
#include "GUI/Client/Core/NLink.hpp"
#include "GUI/Client/Core/NRoot.hpp"
#include "GUI/Client/Core/NTree.hpp"

#include "GUI/Client/Core/CNode.hpp"

#define ADD_ARRAY_TO_XML(type) { \
std::vector<type> data;\
boost::shared_ptr<OptionArrayT<type> > array;\
array = boost::dynamic_pointer_cast<OptionArrayT<type> >(array);\
        \
for( ; itList != list.end() ; itList++)\
 data.push_back( from_str<type>(itList->toStdString()) );\
\
 p.add_array(it.key().toStdString(), data);\
}


using namespace CF::Common;
using namespace CF::Common::String;
using namespace CF::GUI::ClientCore;

CNodeNotifier::CNodeNotifier(CNode * parent)
  : m_parent(parent)
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
    m_type(type),
    m_notifier(new CNodeNotifier(this)),
    m_componentType(componentType)
{

  regist_signal("configure", "Update component options")->connect(boost::bind(&CNode::configure_reply, this, _1));
  regist_signal("tree_updated", "Event that notifies a path has changed")->connect(boost::bind(&CNode::update_tree, this, _1));

  m_properties.add_property("originalComponentType", m_componentType.toStdString());
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString CNode::getComponentType() const
{
  return m_componentType;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CNode::Type CNode::type() const
{
  return m_type;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CNode::setOptions(XmlNode & options)
{
  XmlParams p(options);

  if(p.option_map != nullptr)
  {
    // iterate through options
    XmlNode* node = p.option_map->first_node();
    for ( ; node != nullptr ; node = node->next_sibling() )
    {
      Option::Ptr opt = makeOption(*node);
      m_properties.store[ opt->name() ] = opt;
    }
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CNode::setProperties(XmlNode & options)
{
  XmlParams p(options);

  if(p.property_map != nullptr)
  {
    // iterate through properties
    XmlNode* node = p.property_map->first_node();
    for ( ; node != nullptr ; node = node->next_sibling(  ) )
    {
      XmlAttr * keyAttr= node->first_attribute( XmlParams::tag_attr_key() );

      if ( keyAttr != nullptr )
      {
        const char * keyVal = keyAttr->value(); // option name

        if(std::strcmp(node->name(), XmlParams::tag_node_value())  == 0)
        {
          XmlNode * type_node = node->first_node();

          if( type_node != nullptr)
          {
            const char * typeVal = type_node->name(); // type name

            if(m_properties.check(keyVal))
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
              else if(std::strcmp(typeVal, "uri") == 0)
                configure_property(keyVal, from_str<URI>(type_node->value()));
              else
                throw ShouldNotBeHere(FromHere(), std::string(typeVal) + ": Unknown type parent is " + node->name());
            }
            else
            {
              const char * value = type_node->value();
              if(std::strcmp(typeVal, "bool") == 0)
                m_properties.add_property(keyVal, from_str<bool>(value));
              else if(std::strcmp(typeVal, "integer") == 0)
                m_properties.add_property(keyVal, from_str<int>(value));
              else if(std::strcmp(typeVal, "unsigned") == 0)
                m_properties.add_property(keyVal, from_str<CF::Uint>(value));
              else if(std::strcmp(typeVal, "real") == 0)
                m_properties.add_property(keyVal, from_str<CF::Real>(value));
              else if(std::strcmp(typeVal, "string") == 0)
                m_properties.add_property(keyVal, std::string(value));
              else if(std::strcmp(typeVal, "uri") == 0)
                m_properties.add_property(keyVal, from_str<URI>(value));
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

void CNode::setSignals(CF::Common::XmlNode & node)
{
  XmlParams p(node);

  if(p.signal_map != nullptr)
  {
    XmlNode * map = p.signal_map->parent()->first_node();

    m_actionSigs.clear();

    while(map != nullptr)
    {
      ActionInfo si;
      XmlAttr * key_attr = map->first_attribute( XmlParams::tag_attr_key() );
      XmlAttr * desc_attr = map->first_attribute( XmlParams::tag_attr_descr() );
      XmlAttr * name_attr = map->first_attribute( "name" );
      XmlAttr * hidden_attr = map->first_attribute( "hidden" );

      if(hidden_attr == nullptr || std::strcmp(hidden_attr->value(), "false") == 0)
      {
        cf_assert( key_attr != nullptr );
        cf_assert( key_attr->value_size() > 0 );

        si.m_name = key_attr->value();
        si.m_readableName = name_attr != nullptr ? name_attr->value() : "";
        si.m_description = desc_attr != nullptr ? desc_attr->value() : "";
        si.m_signature = XmlSignature(*map);
        si.m_isLocal = false;

        m_actionSigs.append(si);

      }

      map = map->next_sibling();
    }

  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CNode::modifyOptions(const QMap<QString, QString> & options)
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
      Property * prop = &m_properties[it.key().toStdString()].as_option();

      if( prop != nullptr && strcmp( prop->tag() , "array" ) )
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
        else if(prop->type() == "uri")
          p.add_option(name, URI(value.toStdString()));
        else
          throw ValueNotFound(FromHere(), prop->type() + ": Unknown type for option " + name );
      }
      else if( prop != nullptr && !strcmp ( prop->tag() , "array" ))
      {
        OptionArray * optArray;
        QStringList list = it.value().split("@@");
        QStringList::iterator itList = list.begin();

        optArray = static_cast<OptionArray*>(prop);

        cf_assert(optArray != nullptr);

        const char * elemType = optArray->elem_type();

        if(std::strcmp(elemType, "bool") == 0)
          ADD_ARRAY_TO_XML(bool)
        else if(std::strcmp(elemType, "integer") == 0)
          ADD_ARRAY_TO_XML(int)
        else if(std::strcmp(elemType, "unsigned") == 0)
          ADD_ARRAY_TO_XML(CF::Uint)
        else if(std::strcmp(elemType, "real") == 0)
          ADD_ARRAY_TO_XML(CF::Real)
        else if(std::strcmp(elemType, "string") == 0)
        {
          std::vector<std::string> data;
          boost::shared_ptr<OptionArrayT<std::string> > array;

          array = boost::dynamic_pointer_cast<OptionArrayT<std::string> >(array);

          for( ; itList != list.end() ; itList++)
            data.push_back( itList->toStdString() );

          p.add_array(it.key().toStdString(), data);
        }
        else if(std::strcmp(elemType, "uri") == 0)
        {
          std::vector<URI> data;
          boost::shared_ptr<OptionArrayT<URI> > array;

          array = boost::dynamic_pointer_cast<OptionArrayT<URI> >(array);

          for( ; itList != list.end() ; itList++)
          {
            data.push_back( URI(itList->toStdString()) );
          }

          p.add_array(it.key().toStdString(), data);
        }
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
  QMap<NLink::Ptr, URI> linkTargets;
  QMap<NLink::Ptr, URI>::iterator it;
  CNode::Ptr rootNode;

  rootNode = createFromXmlRec(node, linkTargets);

  it = linkTargets.begin();

  for( ; it != linkTargets.end() ; it++)
    it.key()->setTargetPath(it.value());

  return rootNode;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CNode::Ptr CNode::child(CF::Uint index)
{
  Component::Ptr compo = shared_from_this();
  CF::Uint i;

  if(checkType(CNode::ROOT_NODE))
    compo = this->castTo<NRoot>()->root();

  ComponentIterator<CNode> it = compo->begin<CNode>();

  cf_assert(index < compo->get_child_count());

  for(i = 0 ; i < index ; i++)
    it++;

  return it.get();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CNode::connectNotifier(QObject * reciever, const char * signal, const char * slot)
{
  QObject::connect(m_notifier, signal, reciever, slot);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CNodeNotifier * CNode::notifier() const
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
    CRoot::ConstPtr root = this->castTo<const NRoot>()->root();
    it = root->begin<const CNode>();
    itEnd = root->end<const CNode>();

    if(it->get_child_count() > 0)
      list << QString(root->full_path().string().c_str()) /*+ '/'*/;
    else
      list << root->full_path().string().c_str();
  }

  for( ; it != itEnd ; it++)
  {
    if(!it->isClientComponent() || clientNodes)
    {
      if(it->get_child_count() > 0)
        list << QString(it->full_path().string().c_str()) /*+ '/'*/;
      else
        list << it->full_path().string().c_str();
    }

    if(recursive)
      it->listChildPaths(list, recursive, clientNodes);
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CNode::addNode(CNode::Ptr node)
{
    if(checkType(ROOT_NODE))
      ((NRoot *)this)->root()->add_component(node);
    else
      this->add_component(node);

    m_notifier->notifyChildCountChanged();
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

void CNode::configure_reply(CF::Common::XmlNode & node)
{
  ClientRoot::tree()->optionsChanged(this->full_path());
  ClientRoot::log()->addMessage(QString("Node \"%1\" options updated.").arg(full_path().string().c_str()));
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CNode::options(QList<Option::ConstPtr> & list) const
{
  PropertyList::PropertyStorage_t::const_iterator it;

  it = m_properties.store.begin();

  for( ; it != m_properties.store.end() ; it++)
  {
    Property::Ptr prop = it->second;

    if(prop->is_option())
      list.append(boost::dynamic_pointer_cast<Option const>(prop));
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CNode::properties(QMap<QString, QString> & props) const
{
  PropertyList::PropertyStorage_t::const_iterator it = m_properties.store.begin();

  boost::any val = int(120);

  props.clear();

  for( ; it != m_properties.store.end() ; it++)
  {
    Property::Ptr prop = it->second;
    std::string valueStr;

    if(prop->is_option())
       valueStr = prop->value_str().c_str();
    else
    {
      std::string propType = prop->type();

      if(propType.compare( XmlTag<bool>::type() ) == 0)              // bool prop
        valueStr = to_str(prop->value<bool>());
      else if(propType.compare( XmlTag<int>::type() ) == 0)          // int prop
        valueStr = to_str(prop->value<int>());
      else if(propType.compare( XmlTag<CF::Uint>::type() ) == 0)     // Uint prop
        valueStr = to_str(prop->value<CF::Uint>());
      else if(propType.compare( XmlTag<CF::Real>::type() ) == 0)     // Real prop
        valueStr = to_str(prop->value<CF::Real>());
      else if(propType.compare( XmlTag<std::string>::type() ) == 0)  // string prop
        valueStr = prop->value_str();
      else if(propType.compare( XmlTag<URI>::type() ) == 0)          // URI prop
        valueStr = to_str(prop->value<URI>());
      else
        throw CastingFailed(FromHere(), "Unable to convert " + propType + " to string.");
    }

    props[ it->first.c_str() ] = valueStr.c_str();
  }

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CNode::actions(QList<ActionInfo> & actions) const
{
  actions = m_actionSigs;

  QStringList::const_iterator it = m_localSignals.begin();

  for( ; it != m_localSignals.end() ; it++)
  {

    if(m_signals.find(it->toStdString()) != m_signals.end())
    {
      ActionInfo ai;
      const Signal & sig = m_signals.find(it->toStdString())->second;

      ai.m_name = it->toStdString().c_str();
      ai.m_description = sig.description.c_str();
      ai.m_readableName = sig.readable_name.c_str();
      ai.m_signature = sig.signature;
      ai.m_isLocal = true;

      actions.append(ai);
    }
    else
      ClientRoot::log()->addError(*it + ": local signal not found");
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CNode::Ptr CNode::createFromXmlRec(XmlNode & node, QMap<NLink::Ptr, URI> & linkTargets)
{
  XmlAttr * typeAttr = node.first_attribute("atype");
  XmlAttr * nameAttr = node.first_attribute("name");
  XmlAttr * modeAttr = node.first_attribute("mode");

  cf_assert(typeAttr != nullptr);
  cf_assert(nameAttr != nullptr);

  char * typeName = typeAttr->value();
  char * nodeName = nameAttr->value();
  XmlNode * child = node.first_node();

  cf_assert(typeName != nullptr);
  cf_assert(nodeName != nullptr);
  cf_assert(std::strlen(typeName) > 0);
  cf_assert(std::strlen(nodeName) > 0);

  CNode::Ptr rootNode;

  if(std::strcmp(typeName, "CCore") == 0)
    return rootNode;

  if(std::strcmp(typeName, "CLink") == 0)
  {
    NLink::Ptr link = boost::shared_ptr<NLink>(new NLink(nodeName));
    rootNode = link;
    linkTargets[link] = node.value();
  }
  else if(std::strcmp(typeName, "CRoot") == 0)
    rootNode = boost::shared_ptr<NRoot>(new NRoot(nodeName));
  else
    rootNode = boost::shared_ptr<NGeneric>(new NGeneric(nodeName, typeName));

  if(modeAttr != nullptr && std::strcmp(modeAttr->value(), "basic") == 0)
    rootNode->mark_basic();

  while(child != nullptr)
  {
    try
    {
      if(std::strcmp(child->name(), XmlParams::tag_node_map()) == 0)
      {
        rootNode->setOptions(node);
        rootNode->setProperties(node);
        rootNode->setSignals(node);
      }
      else
      {
        CNode::Ptr node = createFromXmlRec(*child, linkTargets);

        if(node.get() != nullptr)
        {
          if(rootNode->checkType(ROOT_NODE))
            rootNode->castTo<NRoot>()->root()->add_component(node);
          else
            rootNode->add_component(node);
        }
      }
    }
    catch (Exception & e)
    {
      ClientRoot::log()->addException(e.msg().c_str());
    }

    child = child->next_sibling();
  }

  return rootNode;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Option::Ptr CNode::makeOption(const CF::Common::XmlNode & node)
{
  bool advanced;
  XmlAttr * keyAttr= node.first_attribute( XmlParams::tag_attr_key() );
  XmlAttr * descrAttr = node.first_attribute( XmlParams::tag_attr_descr() );
  XmlAttr * modeAttr = node.first_attribute( "mode" );
  Option::Ptr option;
  const char * descrVal = (descrAttr != nullptr) ? descrAttr->value() : "";

  advanced = modeAttr != nullptr && std::strcmp(modeAttr->value(), "adv") == 0;

  if ( keyAttr != nullptr )
  {
    const char * keyVal = keyAttr->value(); // option name

    if(std::strcmp(node.name(), XmlParams::tag_node_value())  == 0)
    {
      XmlNode * type_node = node.first_node();

      if( type_node != nullptr)
      {
        const char * typeVal = type_node->name(); // type name

        if(std::strcmp(typeVal, "bool") == 0)
          option = makeOptionT<bool>(keyVal, descrVal, *type_node);
        else if(std::strcmp(typeVal, "integer") == 0)
          option = makeOptionT<int>(keyVal, descrVal, *type_node);
        else if(std::strcmp(typeVal, "unsigned") == 0)
          option = makeOptionT<CF::Uint>(keyVal, descrVal, *type_node);
        else if(std::strcmp(typeVal, "real") == 0)
          option = makeOptionT<CF::Real>(keyVal, descrVal, *type_node);
        else if(std::strcmp(typeVal, "string") == 0)
          option = makeOptionT<std::string>(keyVal, descrVal, *type_node);
        else if(std::strcmp(typeVal, "uri") == 0)
        {
          URI value;
          OptionURI::Ptr optURI;
          XmlAttr * attr = node.first_attribute(XmlParams::tag_attr_protocol());

          to_value(*type_node, value);
          optURI = OptionURI::Ptr( new OptionURI(keyVal, descrVal, value) );

          for( ; attr != nullptr ; attr = attr->next_attribute(XmlParams::tag_attr_protocol()))
          {
            URIProtocol::Type protocol = URIProtocol::Convert::to_enum(attr-> value());

            if(protocol == URIProtocol::INVALID)
              throw ProtocolError(FromHere(), std::string(attr->value()) + ": unknown protocol.");

            optURI->supported_protocol(protocol);
          }

          option = optURI;
        }
        else
          throw ShouldNotBeHere(FromHere(), std::string(typeVal) + ": Unknown type");
      }
      else
        throw XmlError(FromHere(), "No type node found for option [" + std::string(keyVal) + "] .");
    }
    else if(std::strcmp(node.name(), "array")  == 0)
    {
      XmlAttr * typeAttr= node.first_attribute( XmlParams::tag_attr_type() );

      if( typeAttr != nullptr)
      {
        const char * typeVal = typeAttr->value(); // element type

        if(std::strcmp(typeVal, "bool") == 0)
          option = CNode::makeOptionArrayT<bool>(keyVal, descrVal, node);
        else if(std::strcmp(typeVal, "integer") == 0)
          option = CNode::makeOptionArrayT<int>(keyVal, descrVal, node);
        else if(std::strcmp(typeVal, "unsigned") == 0)
          option = CNode::makeOptionArrayT<CF::Uint>(keyVal, descrVal, node);
        else if(std::strcmp(typeVal, "real") == 0)
          option = CNode::makeOptionArrayT<CF::Real>(keyVal, descrVal, node);
        else if(std::strcmp(typeVal, "string") == 0)
          option = CNode::makeOptionArrayT<std::string>(keyVal, descrVal, node);
        else if(std::strcmp(typeVal, "uri") == 0)
          option = CNode::makeOptionArrayT<URI>(keyVal, descrVal, node);
        else
          throw ShouldNotBeHere(FromHere(), std::string(typeVal) + ": Unknown array type");
      }
      else
        throw XmlError(FromHere(), "No type found for option array [" + std::string(keyVal) + "] .");
    }
    else
      throw XmlError(FromHere(), "Node [" + std::string(node.name()) +"] could not be processed.");

    if(!advanced && option.get() != nullptr)
      option->mark_basic();
  }
  else
    throw XmlError(FromHere(), "No [" + std::string(XmlParams::tag_attr_key()) +"] attribute found.");

  return option;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Signal::return_t CNode::update_tree(XmlNode & node)
{
  ClientRoot::core()->updateTree();
}

#undef ADD_ARRAY_TO_XML
