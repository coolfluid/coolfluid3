// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QMenu>
#include <QMutex>
#include <QPoint>
#include <QStringList>
#include <QVariant>
#include <QDebug>

#include <boost/filesystem/path.hpp>

#include <cstring>

#include "rapidxml/rapidxml.hpp"

#include "Common/CF.hpp"
#include "Common/Log.hpp"
#include "Common/OptionURI.hpp"
#include "Common/StringConversion.hpp"

#include "Common/XML/CastingFunctions.hpp"

#include "GUI/Client/Core/ClientRoot.hpp"
#include "GUI/Client/Core/NCore.hpp"
#include "GUI/Client/Core/NGeneric.hpp"
#include "GUI/Client/Core/NHistory.hpp"
#include "GUI/Client/Core/NJournal.hpp"
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

//////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Common::XML;

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace ClientCore {

/////////////////////////////////////////////////////////////////////////

CNodeNotifier::CNodeNotifier(CNode * parent)
  : m_parent(parent)
{
  //qRegisterMetaType<XmlNode>("CF::Common::XmlNode");
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CNodeNotifier::notifyChildCountChanged()
{
  emit childCountChanged();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CNodeNotifier::notifySignalSignature(Signal::arg_t * node)
{
  emit signalSignature(node);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CNode::CNode(const QString & name, const QString & componentType, CNode::Type type)
  : Component(name.toStdString()),
    m_type(type),
    m_notifier(new CNodeNotifier(this)),
    m_componentType(componentType),
    m_contentListed( isClientComponent() ),
    m_listingContent(false)
{
  m_mutex = new QMutex();

  regist_signal("configure", "Update component options")->connect(boost::bind(&CNode::configure_reply, this, _1));
  regist_signal("tree_updated", "Event that notifies a path has changed")->connect(boost::bind(&CNode::update_tree, this, _1));
  regist_signal("list_content", "Updates node contents")->connect(boost::bind(&CNode::list_content_reply, this, _1));

  m_signals.erase("signal_signature"); // unregister base class signal

  regist_signal("signal_signature", "")->connect(boost::bind(&CNode::signal_signature_reply, this, _1));

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

//void CNode::setOptions(Signal::arg_t & options)
//{
//  QMutexLocker locker(m_mutex);

//  if( options.has_map( Protocol::Tags::key_options() ) )
//  {
//    Map map = options.map(Protocol::Tags::key_options()).main_map;
//    XmlNode node( map.content.content->first_node( ) );

//    // iterate through options
//    for ( ; node.is_valid() ; node.content = node.content->next_sibling() )
//    {
//      Option::Ptr opt = makeOption(node);
//      m_properties.store[ opt->name() ] = opt;
//    }
//  }
//}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CNode::setProperties(const Signal::arg_t & options)
{
  QMutexLocker locker(m_mutex);

  throw NotImplemented(FromHere(), "Check what is returned by the server");

//  if( options.has_map( Protocol::Tags::key_properties() ) )
//  {
//    // iterate through properties
//    XmlNode* node = p.property_map->first_node();
//    for ( ; node != nullptr ; node = node->next_sibling(  ) )
//    {
//      rapidxml::xml_node<> * keyAttr = node->first_attribute( Protocol::Tags::attr_key() );

//      if ( keyAttr != nullptr )
//      {
//        const char * keyVal = keyAttr->value(); // option name

//        if(std::strcmp(node->name(), XmlParams::tag_node_value())  == 0)
//        {
//          XmlNode * type_node = node->first_node();

//          if( type_node != nullptr)
//          {
//            const char * typeVal = type_node->name(); // type name

//            if(m_properties.check(keyVal))
//            {
//              if(std::strcmp(typeVal, "bool") == 0)
//                configure_property(keyVal, from_str<bool>(type_node->value()));
//              else if(std::strcmp(typeVal, "integer") == 0)
//                configure_property(keyVal, from_str<int>(type_node->value()));
//              else if(std::strcmp(typeVal, "unsigned") == 0)
//                configure_property(keyVal, from_str<CF::Uint>(type_node->value()));
//              else if(std::strcmp(typeVal, "real") == 0)
//                configure_property(keyVal, from_str<CF::Real>(type_node->value()));
//              else if(std::strcmp(typeVal, "string") == 0)
//                configure_property(keyVal, std::string(type_node->value()));
//              else if(std::strcmp(typeVal, "uri") == 0)
//                configure_property(keyVal, from_str<URI>(type_node->value()));
//              else
//                throw ShouldNotBeHere(FromHere(), std::string(typeVal) + ": Unknown type parent is " + node->name());
//            }
//            else
//            {
//              const char * value = type_node->value();
//              if(std::strcmp(typeVal, "bool") == 0)
//                m_properties.add_property(keyVal, from_str<bool>(value));
//              else if(std::strcmp(typeVal, "integer") == 0)
//                m_properties.add_property(keyVal, from_str<int>(value));
//              else if(std::strcmp(typeVal, "unsigned") == 0)
//                m_properties.add_property(keyVal, from_str<CF::Uint>(value));
//              else if(std::strcmp(typeVal, "real") == 0)
//                m_properties.add_property(keyVal, from_str<CF::Real>(value));
//              else if(std::strcmp(typeVal, "string") == 0)
//                m_properties.add_property(keyVal, std::string(value));
//              else if(std::strcmp(typeVal, "uri") == 0)
//                m_properties.add_property(keyVal, from_str<URI>(value));
//              else
//                throw ShouldNotBeHere(FromHere(), std::string(typeVal) + ": Unknown type parent is " + node->name());

//            }
//          }
//        }
//      }
//    }
//  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CNode::setSignals(const Signal::arg_t & node)
{
  QMutexLocker locker(m_mutex);

  throw NotImplemented(FromHere(), "Check what is returned by the server");

//  if( node.has_map( Protocol::Tags::key_signals() ) )
//  {
//    Map map = node.map(Protocol::Tags::key_signals());

//    m_actionSigs.clear();

//    while(map != nullptr)
//    {
//      ActionInfo si;
//      XmlAttr * key_attr = map->first_attribute( XmlParams::tag_attr_key() );
//      XmlAttr * desc_attr = map->first_attribute( XmlParams::tag_attr_descr() );
//      XmlAttr * name_attr = map->first_attribute( "name" );
//      XmlAttr * hidden_attr = map->first_attribute( "hidden" );

//      if(hidden_attr == nullptr || std::strcmp(hidden_attr->value(), "false") == 0)
//      {
//        cf_assert( key_attr != nullptr );
//        cf_assert( key_attr->value_size() > 0 );

//        si.name = key_attr->value();
//        si.readableName = name_attr != nullptr ? name_attr->value() : "";
//        si.description = desc_attr != nullptr ? desc_attr->value() : "";
////        si.m_signature = XmlSignature(*map);
//        si.isLocal = false;

//        m_actionSigs.append(si);

//      }

//      map = map->next_sibling();
//    }

//  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CNode::modifyOptions(const QMap<QString, QString> & options)
{
  QMutexLocker locker(m_mutex);

//  QMap<QString, QString>::const_iterator it = options.begin();

//  if(isClientComponent())
//  {
//    for ( ; it != options.end() ; it++)
//    {
////			if(m_options.contains(it.key()))
////				m_options[it.key()].m_paramValue = it.value();
//    }
//  }
//  else
//  {
//    boost::shared_ptr<XmlDoc> docnode = XmlOps::create_doc();
//    XmlNode * rootNode = XmlOps::goto_doc_node(*docnode.get());
//    XmlNode * signalNode = XmlOps::add_signal_frame(*rootNode, "configure", full_path(), full_path(), true);
//    XmlParams p(*signalNode);
//    bool valid = true;

//    for ( ; it != options.end() ; it++)
//    {
//      Property * prop = &m_properties[it.key().toStdString()].as_option();

//      if( prop != nullptr && strcmp( prop->tag() , "array" ) )
//      {
//        std::string name = it.key().toStdString();
//        QString value = it.value();

//        if(prop->type() == "bool")
//          p.add_option(name, QVariant(value).toBool());
//        else if(prop->type() == "integer")
//          p.add_option(name, value.toInt());
//        else if(prop->type() == "unsigned")
//          p.add_option(name, value.toUInt());
//        else if(prop->type() == "real")
//          p.add_option(name, value.toDouble());
//        else if(prop->type() == "string")
//          p.add_option(name, value.toStdString());
//        else if(prop->type() == "uri")
//          p.add_option(name, URI(value.toStdString()));
//        else
//          throw ValueNotFound(FromHere(), prop->type() + ": Unknown type for option " + name );
//      }
//      else if( prop != nullptr && !strcmp ( prop->tag() , "array" ))
//      {
//        OptionArray * optArray;
//        QStringList list = it.value().split("@@");
//        QStringList::iterator itList = list.begin();

//        optArray = static_cast<OptionArray*>(prop);

//        cf_assert(optArray != nullptr);

//        const char * elemType = optArray->elem_type();

//        if(std::strcmp(elemType, "bool") == 0)
//          ADD_ARRAY_TO_XML(bool)
//        else if(std::strcmp(elemType, "integer") == 0)
//          ADD_ARRAY_TO_XML(int)
//        else if(std::strcmp(elemType, "unsigned") == 0)
//          ADD_ARRAY_TO_XML(CF::Uint)
//        else if(std::strcmp(elemType, "real") == 0)
//          ADD_ARRAY_TO_XML(CF::Real)
//        else if(std::strcmp(elemType, "string") == 0)
//        {
//          std::vector<std::string> data;
//          boost::shared_ptr<OptionArrayT<std::string> > array;

//          array = boost::dynamic_pointer_cast<OptionArrayT<std::string> >(array);

//          for( ; itList != list.end() ; itList++)
//            data.push_back( itList->toStdString() );

//          p.add_array(it.key().toStdString(), data);
//        }
//        else if(std::strcmp(elemType, "uri") == 0)
//        {
//          std::vector<URI> data;
//          boost::shared_ptr<OptionArrayT<URI> > array;

//          array = boost::dynamic_pointer_cast<OptionArrayT<URI> >(array);

//          for( ; itList != list.end() ; itList++)
//          {
//            data.push_back( URI(itList->toStdString()) );
//          }

//          p.add_array(it.key().toStdString(), data);
//        }
//        else
//          throw ValueNotFound(FromHere(), std::string(elemType) + ": Unknown type for option array " + optArray->name());
//      }
//    }

//    if(valid)
//      ClientRoot::instance().core()->sendSignal(*docnode.get());
//  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CNode::localSignature(const QString & name, Signal::arg_t& node )
{
  ( *signal( name.toStdString() ).signature )(node);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CNode::Ptr CNode::createFromXml(XmlNode args)
{
  QMap<NLink::Ptr, URI> linkTargets;
  QMap<NLink::Ptr, URI>::iterator it;
  CNode::Ptr rootNode;

  rootNode = createFromXmlRec(args, linkTargets);

  it = linkTargets.begin();

  for( ; it != linkTargets.end() ; it++)
    it.key()->setTargetPath(it.value());

  return rootNode;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CNode::Ptr CNode::child(CF::Uint index)
{
  QMutexLocker locker(m_mutex);

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
  QMutexLocker locker(m_mutex);

  ComponentIterator<const CNode> it = this->begin<const CNode>();
  ComponentIterator<const CNode> itEnd = this->end<const CNode>();

  if(this->checkType(ROOT_NODE))
  {
    CRoot::ConstPtr root = this->castTo<const NRoot>()->root();
    it = root->begin<const CNode>();
    itEnd = root->end<const CNode>();

    if(it->get_child_count() > 0)
      list << QString(root->full_path().path().c_str()) /*+ '/'*/;
    else
      list << root->full_path().path().c_str();
  }

  for( ; it != itEnd ; it++)
  {
    if(!it->isClientComponent() || clientNodes)
    {
      if(it->get_child_count() > 0)
        list << QString(it->full_path().path().c_str()) /*+ '/'*/;
      else
        list << it->full_path().path().c_str();
    }

    if(recursive)
      it->listChildPaths(list, recursive, clientNodes);
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CNode::addNode(CNode::Ptr node)
{
  QMutexLocker locker(m_mutex);

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
  QMutexLocker locker(m_mutex);

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

void CNode::configure_reply(Signal::arg_t & args)
{
  ClientRoot::instance().tree()->optionsChanged(this->full_path());
  ClientRoot::instance().log()->addMessage(QString("Node \"%1\" options updated.").arg(full_path().path().c_str()));
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CNode::list_content_reply( Signal::arg_t & node )
{
  setProperties(node);
  setSignals(node);

  m_contentListed = true;
  m_listingContent = false;

  ClientRoot::instance().tree()->contentListed( boost::dynamic_pointer_cast<CNode>(self()) );
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CNode::signal_signature_reply( Signal::arg_t & node )
{
  m_notifier->notifySignalSignature(&node);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CNode::options(QList<Option::ConstPtr> & list)
{
  QMutexLocker locker(m_mutex);

  if(!m_contentListed)
    fetchContent();
  else
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
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CNode::properties(QMap<QString, QString> & props)
{
  QMutexLocker locker(m_mutex);

  if(!m_contentListed)
    fetchContent();
  else
  {
    PropertyList::PropertyStorage_t::const_iterator it = m_properties.store.begin();

    props.clear();

    for( ; it != m_properties.store.end() ; it++)
    {
      Property::Ptr prop = it->second;
      std::string valueStr;

      if(prop->is_option())
        valueStr = prop->value_str().c_str();
      else
      {
//        std::string propType = prop->type();

        valueStr = prop->value_str();

//        if(propType.compare( XmlTag<bool>::type() ) == 0)              // bool prop
//          valueStr = to_str(prop->value<bool>());
//        else if(propType.compare( XmlTag<int>::type() ) == 0)          // int prop
//          valueStr = to_str(prop->value<int>());
//        else if(propType.compare( XmlTag<CF::Uint>::type() ) == 0)     // Uint prop
//          valueStr = to_str(prop->value<CF::Uint>());
//        else if(propType.compare( XmlTag<CF::Real>::type() ) == 0)     // Real prop
//          valueStr = to_str(prop->value<CF::Real>());
//        else if(propType.compare( XmlTag<std::string>::type() ) == 0)  // string prop
//          valueStr = prop->value_str();
//        else if(propType.compare( XmlTag<URI>::type() ) == 0)          // URI prop
//          valueStr = to_str(prop->value<URI>());
//        else
//          throw CastingFailed(FromHere(), "Unable to convert " + propType + " to string.");
      }

      props[ it->first.c_str() ] = valueStr.c_str();
    }
  }

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CNode::actions(QList<ActionInfo> & actions)
{
  QMutexLocker locker(m_mutex);

  if(!m_contentListed)
    fetchContent();
  else
  {
    actions = m_actionSigs;

    QStringList::const_iterator it = m_localSignals.begin();

    for( ; it != m_localSignals.end() ; it++)
    {

      if(m_signals.find(it->toStdString()) != m_signals.end())
      {
        ActionInfo ai;
        const Signal & sig = m_signals.find(it->toStdString())->second;

        ai.name = it->toStdString().c_str();
        ai.description = sig.description.c_str();
        ai.readableName = sig.readable_name.c_str();
//        ai.m_signature = sig.signature;
        ai.isLocal = true;

        actions.append(ai);
      }
      else
        ClientRoot::instance().log()->addError(*it + ": local signal not found");
    }
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CNode::Ptr CNode::createFromXmlRec(XmlNode & node, QMap<NLink::Ptr, URI> & linkTargets)
{
  rapidxml::xml_attribute<>* typeAttr = node.content->first_attribute("atype");
  rapidxml::xml_attribute<>* nameAttr = node.content->first_attribute("name");
  rapidxml::xml_attribute<>* modeAttr = node.content->first_attribute("mode");

  cf_assert(typeAttr != nullptr);
  cf_assert(nameAttr != nullptr);

  QString typeName = typeAttr->value();
  char * nodeName = nameAttr->value();
  XmlNode child( node.content->first_node() );

  cf_assert( !typeName.isEmpty() );
  cf_assert(nodeName != nullptr);
  cf_assert( typeName.length() > 0);
  cf_assert(std::strlen(nodeName) > 0);

  CNode::Ptr rootNode;

  if( typeName == "CCore" )
    return rootNode;

  if( typeName == "CLink" )
  {
    NLink::Ptr link = boost::shared_ptr<NLink>(new NLink(nodeName));
    rootNode = link;
    linkTargets[link] = node.content->value();
  }
  else if( typeName == "CJournal" )
    rootNode = boost::shared_ptr<NJournal>(new NJournal(nodeName));
  else if( typeName == "CRoot" )
    rootNode = boost::shared_ptr<NRoot>(new NRoot(nodeName));
  else if(std::strcmp(typeName, "CHistory") == 0)
    rootNode = boost::shared_ptr<NHistory>(new NHistory(nodeName));
  else
    rootNode = boost::shared_ptr<NGeneric>(new NGeneric(nodeName, typeName));

  if(modeAttr != nullptr && std::strcmp(modeAttr->value(), "basic") == 0)
    rootNode->mark_basic();

  while( child.is_valid() )
  {
    try
    {
      if(std::strcmp(child.content->name(), Protocol::Tags::node_map()) == 0)
      {
//        rootNode->setOptions(node);
//        rootNode->setProperties(node);
//        rootNode->setSignals(node);
      }
      else
      {
        CNode::Ptr node = createFromXmlRec(child, linkTargets);

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
      ClientRoot::instance().log()->addException(e.msg().c_str());
    }

    child.content = child.content->next_sibling();
  }

  return rootNode;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Option::Ptr CNode::makeOption(const XmlNode & node)
{
  cf_assert( node.is_valid() );

  bool advanced;
  Option::Ptr option;
  std::string type( Map::get_value_type(node) );
  rapidxml::xml_attribute<>* keyAttr = node.content->first_attribute( Protocol::Tags::attr_key() );
  rapidxml::xml_attribute<>* descrAttr = node.content->first_attribute( Protocol::Tags::attr_descr() );

  rapidxml::xml_attribute<>* modeAttr = node.content->first_attribute( "mode" );
  advanced = modeAttr != nullptr && std::strcmp(modeAttr->value(), "adv") == 0;

  std::string descr_str( (descrAttr != nullptr) ? descrAttr->value() : "");

  if( is_not_null(keyAttr) )
  {
    std::string key_str( keyAttr->value() ); // option name

    if( Map::is_single_value(node) )
    {
      XmlNode type_node( node.content->first_node(type.c_str()) );

      if(type == Protocol::Tags::type<bool>() )
        option = makeOptionT<bool>(key_str, descr_str, type_node);
      else if(type == Protocol::Tags::type<int>() )
        option = makeOptionT<int>(key_str, descr_str, type_node);
      else if(type == Protocol::Tags::type<Uint>() )
        option = makeOptionT<Uint>(key_str, descr_str, type_node);
      else if(type == Protocol::Tags::type<Real>() )
        option = makeOptionT<Real>(key_str, descr_str, type_node);
      else if(type == Protocol::Tags::type<std::string>() )
        option = makeOptionT<std::string>(key_str, descr_str, type_node);
      else if(type == Protocol::Tags::type<URI>() )
      {
        URI value;
        to_value(type_node, value);
        XmlNode restr_node = Map(type_node).seek_value(Protocol::Tags::key_restricted_values(), Protocol::Tags::node_array());

        std::vector<URI> restr_list;

        option = Option::Ptr (new Common::OptionURI(key_str, descr_str, value));

        if(restr_node.is_valid())

        {
          restr_list = Map().array_to_vector<URI>( restr_node );

          std::vector<URI>::iterator it;

          for( it = restr_list.begin() ; it != restr_list.end() ; ++it)
            option->restricted_list().push_back( *it );
        }

        /// @todo manage protocols

      }
      else
        throw ShouldNotBeHere(FromHere(), type + ": Unknown type");

    }
    else if( Map::is_array_value(node) )
    {
      if(type == Protocol::Tags::type<bool>() )
        option = makeOptionArrayT<bool>(key_str, descr_str, node);
      else if(type == Protocol::Tags::type<int>() )
        option = makeOptionArrayT<int>(key_str, descr_str, node);
      else if(type == Protocol::Tags::type<Uint>() )
        option = makeOptionArrayT<Uint>(key_str, descr_str, node);
      else if(type == Protocol::Tags::type<Real>() )
        option = makeOptionArrayT<Real>(key_str, descr_str, node);
      else if(type == Protocol::Tags::type<std::string>() )
        option = makeOptionArrayT<std::string>(key_str, descr_str, node);
      else if(type == Protocol::Tags::type<URI>() )
        option = makeOptionArrayT<URI>(key_str, descr_str, node);
      else
        throw ShouldNotBeHere(FromHere(), type + ": Unknown type");
    }
    else
      throw XmlError(FromHere(), "Node [" + std::string(node.content->name()) +"] could not be processed.");

    if(!advanced && option.get() != nullptr)
      option->mark_basic();
  }

  return option;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CNode::requestSignalSignature(const QString & name)
{
  URI path;
  XmlNode * node;

  if( m_type == ROOT_NODE )
    path = URI(CLIENT_ROOT_PATH, URI::Scheme::CPATH);
  else
    path = full_path();

  SignalFrame frame("signal_signature", path, path);

  frame.map( Protocol::Tags::key_options() ).set_option("name", name.toStdString());

  ClientRoot::instance().core()->sendSignal(frame);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Signal::return_t CNode::update_tree(Signal::arg_t & node)
{
  ClientRoot::instance().core()->updateTree();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CNode::fetchContent()
{
  if(!m_contentListed && !m_listingContent && ClientRoot::instance().core()->isConnected())
  {
    URI path;

    if( m_type == ROOT_NODE )
      path = URI(CLIENT_ROOT_PATH, URI::Scheme::CPATH);
    else
      path = full_path();

    SignalFrame frame("list_content", path, path);

    ClientRoot::instance().core()->sendSignal(frame);
    m_listingContent = true;
  }
}

////////////////////////////////////////////////////////////////////////////

template<typename TYPE>
Option::Ptr CNode::makeOptionT(const std::string & name, const std::string & descr, XmlNode & node)
{
  TYPE value;
  to_value(node, value);
  XmlNode restr_node = Map(node).seek_value(Protocol::Tags::key_restricted_values(), Protocol::Tags::node_array());

  std::vector<TYPE> restr_list;

  Option::Ptr option(new Common::OptionT<TYPE>(name, descr, value));

  if(restr_node.is_valid())
  {
    restr_list = Map().array_to_vector<TYPE>( restr_node );

    typename std::vector<TYPE>::iterator it;

    for( it = restr_list.begin() ; it != restr_list.end() ; ++it)
      option->restricted_list().push_back( *it );
  }

  return option;
}

////////////////////////////////////////////////////////////////////////////

template<typename TYPE>
Option::Ptr CNode::makeOptionArrayT(const std::string & name, const std::string & descr,
                             const XmlNode & node)
{
  std::vector<TYPE> value = Map().array_to_vector<TYPE>(node);

  Option::Ptr option(new Common::OptionArrayT<TYPE>(name, descr, value));

  XmlNode restr_node = Map(node).seek_value(Protocol::Tags::key_restricted_values(), Protocol::Tags::node_array());

  std::vector<TYPE> restr_list;

  if(restr_node.is_valid())
  {
    restr_list = Map().array_to_vector<TYPE>( restr_node );

    typename std::vector<TYPE>::iterator it;

    for( it = restr_list.begin() ; it != restr_list.end() ; ++it)
      option->restricted_list().push_back( *it );
  }

  return option;
}

////////////////////////////////////////////////////////////////////////////

#define TEMPLATE_EXPLICIT_INSTANTIATON(T) \
Common_TEMPLATE template Option::Ptr CNode::makeOptionArrayT<T>(const std::string&, const std::string&, const XmlNode&);\
Common_TEMPLATE template Option::Ptr CNode::makeOptionT<T>(const std::string &, const std::string &, XmlNode &)

TEMPLATE_EXPLICIT_INSTANTIATON( bool );
TEMPLATE_EXPLICIT_INSTANTIATON( int );
TEMPLATE_EXPLICIT_INSTANTIATON( Uint );
TEMPLATE_EXPLICIT_INSTANTIATON( Real );
TEMPLATE_EXPLICIT_INSTANTIATON( std::string );
Common_TEMPLATE template Option::Ptr CNode::makeOptionArrayT<URI>(
    const std::string&, const std::string&, const XmlNode&);

#undef TEMPLATE_EXPLICIT_INSTANTIATON

//////////////////////////////////////////////////////////////////////////////

} // ClientCore
} // GUI
} // CF

//////////////////////////////////////////////////////////////////////////////

#undef ADD_ARRAY_TO_XML
