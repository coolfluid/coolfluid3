// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QMutex>
#include <QMap>
#include <QStringList>
#include <QVariant>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "rapidxml/rapidxml.hpp"

#include "Common/BoostAnyConversion.hpp"
#include "Common/CF.hpp"
#include "Common/Signal.hpp"
#include "Common/OptionArray.hpp"
#include "Common/OptionT.hpp"
#include "Common/OptionURI.hpp"
#include "Common/StringConversion.hpp"

#include "Common/XML/CastingFunctions.hpp"
#include "Common/XML/Protocol.hpp"
#include "Common/XML/SignalOptions.hpp"

#include "UI/UICommon/ComponentNames.hpp"

#include "UI/Core/CNodeBuilders.hpp"
#include "UI/Core/NetworkThread.hpp"
#include "UI/Core/NGeneric.hpp"
#include "UI/Core/NJournal.hpp"
#include "UI/Core/NLog.hpp"
#include "UI/Core/NLink.hpp"
#include "UI/Core/NRoot.hpp"
#include "UI/Core/NTree.hpp"
#include "UI/Core/ThreadManager.hpp"

#include "UI/Core/CNode.hpp"

//////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Common::XML;

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {
namespace Core {

/////////////////////////////////////////////////////////////////////////////

CNodeNotifier::CNodeNotifier(CNode * parent)
  : m_parent(parent)
{

}

////////////////////////////////////////////////////////////////////////////

void CNodeNotifier::notifyChildCountChanged()
{
  emit childCountChanged();
}

////////////////////////////////////////////////////////////////////////////

void CNodeNotifier::notifySignalSignature(SignalArgs * node)
{
  emit signalSignature(node);
}

////////////////////////////////////////////////////////////////////////////

CNode::CNode(const std::string & name, const QString & componentType, Type type)
  : Component(name),
    m_notifier(new CNodeNotifier(this)),
    m_componentType(componentType),
    m_type(type),
    m_listingContent(false),
    m_isRoot(false)
{
  m_contentListed = isLocalComponent();
  m_mutex = new QMutex();

  // unregister some base class signals

  unregist_signal("signal_signature");
  unregist_signal("configure");

  regist_signal( "configure" )
    ->description("Update component options")
    ->connect(boost::bind(&CNode::configure_reply, this, _1));

  regist_signal( "tree_updated" )
    ->description("Event that notifies a path has changed")
    ->connect(boost::bind(&CNode::update_tree, this, _1));

  regist_signal( "list_content" )
    ->description("Updates node contents")
    ->connect(boost::bind(&CNode::list_content_reply, this, _1));

  regist_signal("signal_signature")
      ->hidden(true);

  m_properties.add_property("original_component_type", m_componentType.toStdString());
}

////////////////////////////////////////////////////////////////////////////

QString CNode::componentType() const
{
  return m_componentType;
}

////////////////////////////////////////////////////////////////////////////

Component::Ptr CNode::realComponent()
{
  if( m_isRoot )
    return castTo<NRoot>()->root();

  return as_ptr<Component>();
}

////////////////////////////////////////////////////////////////////////////

Component::ConstPtr CNode::realComponent() const
{
  if( m_isRoot )
    return castTo<const NRoot>()->root();

  return as_ptr<const Component>();
}

////////////////////////////////////////////////////////////////////////////

void CNode::setProperties(const SignalArgs & options)
{
  QMutexLocker locker(m_mutex);

  if( options.has_map( Protocol::Tags::key_properties() ) )
  {
    Map prop_map = options.map( Protocol::Tags::key_properties() ).main_map;
    XmlNode curr_node( prop_map.content.content->first_node() );

    for( ; curr_node.is_valid() ; curr_node.content = curr_node.content->next_sibling())
    {
      rapidxml::xml_node<>* node = curr_node.content;
      rapidxml::xml_attribute<>* attr = node->first_attribute("is_option");

      if( is_not_null(attr) && from_str<bool>(attr->value()) ) // if it is an option
      {
        Option::Ptr opt = SignalOptions::xml_to_option(curr_node);
        cf_assert( opt.get() != nullptr );
        m_options.store[ opt->name() ] = opt;
      }
      else // it is a property
      {
        rapidxml::xml_attribute<> * keyAttr = node->first_attribute( Protocol::Tags::attr_key() );

        if ( keyAttr != nullptr )
        {
          const char * keyVal = keyAttr->value();
          std::string typeVal = Map::get_value_type(curr_node); // type name
          rapidxml::xml_node<>* firstNode = curr_node.content->first_node();

          if( is_not_null(firstNode) )
          {
            const char * value = firstNode->value();

            if(m_properties.check(keyVal))
            {
              if( typeVal == Protocol::Tags::type<bool>() )
                configure_property(keyVal, from_str<bool>(value));
              else if( typeVal == Protocol::Tags::type<int>() )
                configure_property(keyVal, from_str<int>(value));
              else if( typeVal == Protocol::Tags::type<CF::Uint>() )
                configure_property(keyVal, from_str<CF::Uint>(value));
              else if( typeVal == Protocol::Tags::type<CF::Real>() )
                configure_property(keyVal, from_str<CF::Real>(value));
              else if( typeVal == Protocol::Tags::type<std::string>() )
                configure_property(keyVal, std::string(value));
              else if( typeVal == Protocol::Tags::type<URI>() )
                configure_property(keyVal, from_str<URI>(value));
              else
                throw ShouldNotBeHere(FromHere(), typeVal + ": Unknown type.");
            }
            else
            {

              if( typeVal == Protocol::Tags::type<bool>() )
                m_properties.add_property(keyVal, from_str<bool>(value));
              else if( typeVal == Protocol::Tags::type<int>() )
                m_properties.add_property(keyVal, from_str<int>(value));
              else if( typeVal == Protocol::Tags::type<CF::Uint>() )
                m_properties.add_property(keyVal, from_str<CF::Uint>(value));
              else if( typeVal == Protocol::Tags::type<CF::Real>() )
                m_properties.add_property(keyVal, from_str<CF::Real>(value));
              else if( typeVal == Protocol::Tags::type<std::string>() )
                m_properties.add_property(keyVal, std::string(value));
              else if( typeVal == Protocol::Tags::type<URI>() )
                m_properties.add_property(keyVal, from_str<URI>(value));
              else
                throw ShouldNotBeHere(FromHere(), typeVal + ": Unknown type.");
            }
          } // end of "it( is_not_null(firstNode) )"
        }
      } // end of "else"
    }
  } // end of "if( options.has_map() )"
}

////////////////////////////////////////////////////////////////////////////

void CNode::setSignals(const SignalArgs & args)
{
  QMutexLocker locker(m_mutex);

  if( args.has_map( Protocol::Tags::key_signals() ) )
  {
    Map sig_map = args.map( Protocol::Tags::key_signals() ).main_map;

    m_actionSigs.clear();

    while( sig_map.content.is_valid() )
    {
      ActionInfo si;
      rapidxml::xml_node<>* node = sig_map.content.content;
      rapidxml::xml_attribute<> * key_attr = node->first_attribute( Protocol::Tags::attr_key() );
      rapidxml::xml_attribute<> * desc_attr = node->first_attribute( Protocol::Tags::attr_descr() );
      rapidxml::xml_attribute<> * name_attr = node->first_attribute( "name" );
      rapidxml::xml_attribute<> * hidden_attr = node->first_attribute( "hidden" );

      if(hidden_attr == nullptr || !from_str<bool>(hidden_attr->value()) )
      {
        cf_assert( key_attr != nullptr );
        cf_assert( key_attr->value()[0] != '\0');

        si.name = key_attr->value();
        si.readableName = name_attr != nullptr ? name_attr->value() : si.name;
        si.description = desc_attr != nullptr ? desc_attr->value() : "";
        si.isLocal = false;
        si.isEnabled = true;

        m_actionSigs.append(si);

      }

      sig_map.content.content = node->next_sibling();
    }

  }
}

////////////////////////////////////////////////////////////////////////////

template<typename TYPE>
void addValueToXml(const std::string& name, const std::string& value,
                   const std::string& sep, SignalOptions& options)
{
  // if it is an array, the separator is not empty
  if( !sep.empty() )
  {
    std::vector<TYPE> data;
    Map::split_string(value, sep, data);

    options.add_option< OptionArrayT<TYPE> >(name, data);
  }
  else
  {
    try
    {
      options.add_option< OptionT<TYPE> >(name, from_str<TYPE>(value));
    }
    catch( const boost::bad_lexical_cast & e)
    {
      throw CastingFailed(FromHere(), "Unable to cast [" + value + "] to " +
                          Protocol::Tags::type<TYPE>() + ".");
    }
  }
}

////////////////////////////////////////////////////////////////////////////

void CNode::modifyOptions(const QMap<QString, QString> & opts)
{
  QMutexLocker locker(m_mutex);

  QMap<QString, QString>::const_iterator it = opts.begin();

  // for better readability of the code, a SignalFrame is built even when
  // modifying local options. It's not the most efficient way to do, but local
  // options are not meant to be modified 10 times each second so this should
  // not affect the application performances.
  //  SignalFrame frame("configure", uri(), uri());
  //  SignalOptions options(frame);
  SignalOptions options;

  for( ; it != opts.end() ; it++)
  {
    std::string sep;
    std::string name( it.key().toStdString() );
    std::string value( it.value().toStdString() );

    if( !m_options.check(name) )
      throw ValueNotFound(FromHere(), "Could not find an option with name ["+name+"]");

    Option& option = m_options[name];
    std::string type( option.type() );
    bool is_array = std::strcmp(option.tag(), "array") == 0;

    // if it is an array, we need to get the element type
    if(is_array)
    {
      OptionArray * optArray;

      optArray = static_cast<OptionArray*>(&option);

      cf_assert(optArray != nullptr);

      type = optArray->elem_type();
      sep = optArray->separator();
    }

    if(type == Protocol::Tags::type<bool>())              // bool
      addValueToXml<bool>(name, value, sep, options);
    else if(type == Protocol::Tags::type<int>())          // int
      addValueToXml<int>(name, value, sep, options);
    else if(type == Protocol::Tags::type<Uint>())         // Uint
      addValueToXml<Uint>(name, value, sep, options);
    else if(type == Protocol::Tags::type<Real>())         // Real
      addValueToXml<Real>(name, value, sep, options);
    else if(type == Protocol::Tags::type<std::string>())  // string
      addValueToXml<std::string>(name, value, sep, options);
    else if(type == Protocol::Tags::type<URI>())          // URI
    {
      // since OptionT<URI> does not exist, using addValueToXml for
      // this type would lead to an undefined reference linking error
      if( sep.empty() )
        options.add_option< OptionURI >(name, from_str<URI>(value));
      else
      {
        std::vector<URI> data;
        Map::split_string(value, sep, data);
        options.add_option< OptionArrayT<URI> >(name, data);
      }
    }
    else
      throw ValueNotFound(FromHere(), type + ": Unknown type for option [" + name + "]." );
  }

  // if there were options to modify
  if( !options.store.empty() )
  {
    SignalFrame frame = options.create_frame("configure", uri(), uri());

    if( isLocalComponent() )
      signal_configure( frame );
    else
      ThreadManager::instance().network().send(frame);
  }
}

////////////////////////////////////////////////////////////////////////////

void CNode::localSignature(const QString & name, SignalArgs& node )
{
  std::string sname = name.toStdString();
  ( * signal( sname )->signature() ) ( node );
}

////////////////////////////////////////////////////////////////////////////

void CNode::finishSetUp()
{
  SignalFrame frame;
  call_signal("setUpFinished", frame);
}

////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////

CNode::Ptr CNode::child(CF::Uint index)
{
  QMutexLocker locker(m_mutex);

  Component::Ptr compo = realComponent();
  CF::Uint i;

  ComponentIterator<CNode> it = compo->begin<CNode>();

  cf_assert(index < compo->count_children());

  for(i = 0 ; i < index ; i++)
    it++;

  return it.get();
}

////////////////////////////////////////////////////////////////////////////

void CNode::connectNotifier(QObject * reciever, const char * signal, const char * slot)
{
  QObject::connect(m_notifier, signal, reciever, slot, Qt::DirectConnection);
}

////////////////////////////////////////////////////////////////////////////

CNodeNotifier * CNode::notifier() const
{
  return m_notifier;
}

////////////////////////////////////////////////////////////////////////////

void CNode::listChildPaths(QStringList & list, bool recursive, bool clientNodes) const
{
  QMutexLocker locker(m_mutex);
  Component::ConstPtr comp = realComponent();

  ComponentIterator<const CNode> itBegin = comp->begin<const CNode>();
  ComponentIterator<const CNode> itEnd = comp->end<const CNode>();

  // add the current path
  if(list.isEmpty())
    list << comp->uri().path().c_str();

  for( ; itBegin != itEnd ; itBegin++)
  {
    if(!itBegin->isLocalComponent() || clientNodes)
    {
      list << itBegin->uri().path().c_str();

      if(recursive)
        itBegin->listChildPaths(list, recursive, clientNodes);
    }

  }
}

////////////////////////////////////////////////////////////////////////////

void CNode::addNode(CNode::Ptr node)
{
  QMutexLocker locker(m_mutex);

  realComponent()->add_component(node);

  m_notifier->notifyChildCountChanged();
}

////////////////////////////////////////////////////////////////////////////

void CNode::removeNode(const QString & nodeName)
{
  QMutexLocker locker(m_mutex);

  realComponent()->remove_component( nodeName.toStdString() );

  m_notifier->notifyChildCountChanged();
}

////////////////////////////////////////////////////////////////////////////

void CNode::configure_reply(SignalArgs & args)
{
  URI path = uri();
  QString msg("Node \"%1\" options updated.");

  signal_configure(args);

  NTree::globalTree()->optionsChanged(path);
  NLog::globalLog()->addMessage(msg.arg(path.string().c_str()));
}

////////////////////////////////////////////////////////////////////////////

void CNode::list_content_reply( SignalArgs & node )
{
  setProperties(node);
  setSignals(node);

  m_contentListed = true;
  m_listingContent = false;

  NTree::globalTree()->contentListed( boost::dynamic_pointer_cast<CNode>(self()) );
}

////////////////////////////////////////////////////////////////////////////

void CNode::signal_signature_reply( SignalArgs & node )
{
  m_notifier->notifySignalSignature(&node);
}

////////////////////////////////////////////////////////////////////////////

void CNode::listOptions(QList<Option::ConstPtr> & list)
{
  QMutexLocker locker( m_mutex );

 if( !m_contentListed )
   fetchContent();
 else
 {
   OptionList::const_iterator it = m_options.begin();

   for( ; it != m_options.end() ; ++it)
     list.append( it->second );
 }
}

////////////////////////////////////////////////////////////////////////////

void CNode::listProperties(QMap<QString, QString> & props)
{
  QMutexLocker locker( m_mutex );

  if( !m_contentListed )
    fetchContent();
  else
  {
    Component::Ptr comp = realComponent();
    PropertyList::const_iterator it_prop = comp->properties().begin();
    OptionList::const_iterator it_opt = comp->options().begin();

    props.clear();

    // add the properties
    for( ; it_prop != comp->properties().end() ; ++it_prop)
      props[ it_prop->first.c_str() ] =  any_to_str(it_prop->second).c_str();

    // add the options
    for( ; it_opt != comp->options().end() ; ++it_opt)
      props[ it_opt->first.c_str() ] = it_opt->second->value_str().c_str();
  }

}

////////////////////////////////////////////////////////////////////////////

void CNode::listSignals(QList<ActionInfo> & actions)
{
  QMutexLocker locker( m_mutex );

  QStringList::const_iterator it = m_localSignals.begin();
  QMap<QString, bool> availableLocalSignals;

  for( ; it != m_localSignals.end() ; it++)
    availableLocalSignals[*it] = true;

  this->disableLocalSignals(availableLocalSignals);

  it = m_localSignals.begin();

  // put the local signals
  for( ; it != m_localSignals.end() ; it++)
  {

    std::string sname = it->toStdString();

    if( signal_exists( sname ) )
    {
      ActionInfo ai;
      SignalPtr sig = signal( sname );

      ai.name = sname.c_str();
      ai.description = sig->description().c_str();
      ai.readableName = sig->pretty_name().c_str();
      ai.isLocal = true;
      ai.isEnabled = availableLocalSignals[*it];

      actions.append(ai);
    }
    else
      NLog::globalLog()->addError(*it + ": local signal not found");
  }

  // if content has not been listed yet, we fetch
  if(!m_contentListed)
    fetchContent();
  else
    actions.append(m_actionSigs); // copy the "remote signals"
}

////////////////////////////////////////////////////////////////////////////

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
  else if( CNodeBuilders::instance().hasBuilder( typeName ) )
    rootNode = CNodeBuilders::instance().buildCNode(typeName, nodeName);
  else if( typeName == "CJournal" )
    rootNode = boost::shared_ptr<NJournal>(new NJournal(nodeName));
  else if( typeName == "CRoot" )
    rootNode = boost::shared_ptr<NRoot>(new NRoot(nodeName));
  else
    rootNode = boost::shared_ptr<NGeneric>(new NGeneric(nodeName, typeName));

  if(modeAttr != nullptr && std::strcmp(modeAttr->value(), "basic") == 0)
    rootNode->mark_basic();

  while( child.is_valid() )
  {
    try
    {
      CNode::Ptr node = createFromXmlRec(child, linkTargets);

      if(node.get() != nullptr)
      {
        rootNode->addNode(node);
        node->setUpFinished();
      }
    }
    catch (Exception & e)
    {
      NLog::globalLog()->addException(e.msg().c_str());
    }

    child.content = child.content->next_sibling();
  }

  return rootNode;
}

////////////////////////////////////////////////////////////////////////////

void CNode::requestSignalSignature(const QString & name)
{
  URI path = realComponent()->uri();
  XmlNode * node;

  SignalFrame frame("signal_signature", path, path);

  frame.map( Protocol::Tags::key_options() ).set_option("name", name.toStdString());

  ThreadManager::instance().network().send(frame);
}

////////////////////////////////////////////////////////////////////////////

void CNode::update_tree(SignalArgs & node)
{
  NTree::globalTree()->updateTree();
}

////////////////////////////////////////////////////////////////////////////

void CNode::fetchContent()
{
  NetworkThread& network = ThreadManager::instance().network();
  if(!m_contentListed && !m_listingContent && network.isConnected())
  {
    URI path = realComponent()->uri();

    SignalFrame frame("list_content", path, path);

    network.send(frame);
    m_listingContent = true;
  }
}

////////////////////////////////////////////////////////////////////////////

} // Core
} // UI
} // CF

//////////////////////////////////////////////////////////////////////////////
