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

#include "Common/CF.hpp"
#include "Common/Signal.hpp"
#include "Common/OptionArray.hpp"
#include "Common/OptionT.hpp"
#include "Common/OptionURI.hpp"
#include "Common/StringConversion.hpp"

#include "Common/XML/CastingFunctions.hpp"
#include "Common/XML/Protocol.hpp"

#include "GUI/Network/ComponentNames.hpp"

#include "GUI/Client/Core/NetworkThread.hpp"
#include "GUI/Client/Core/NGeneric.hpp"
#include "GUI/Client/Core/NJournal.hpp"
#include "GUI/Client/Core/NLog.hpp"
#include "GUI/Client/Core/NLink.hpp"
#include "GUI/Client/Core/NPlotXY.hpp"
#include "GUI/Client/Core/NRoot.hpp"
#include "GUI/Client/Core/NTree.hpp"
#include "GUI/Client/Core/ThreadManager.hpp"

#include "GUI/Client/Core/CNode.hpp"

#define ADD_ARRAY_TO_XML(type) { \
std::vector<type> data;\
boost::shared_ptr<OptionArrayT<type> > array;\
array = boost::dynamic_pointer_cast<OptionArrayT<type> >(array);\
        \
for( ; itList != list.end() ; itList++)\
 data.push_back( from_str<type>(itList->toStdString()) );\
\
 p.set_array(it.key().toStdString(), data, " ; ");\
}

//////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Common::XML;

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace ClientCore {

/////////////////////////////////////////////////////////////////////////////

CNodeNotifier::CNodeNotifier(CNode * parent)
  : m_parent(parent)
{
  //qRegisterMetaType<XmlNode>("CF::Common::XmlNode");
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

CNode::CNode(const QString & name, const QString & componentType, CNode::Type type)
  : Component(name.toStdString()),
    m_type(type),
    m_notifier(new CNodeNotifier(this)),
    m_componentType(componentType),
    m_contentListed( isLocalComponent() ),
    m_listingContent(false)
{
  m_mutex = new QMutex();

  // unregister some base class signals
  m_signals.erase("signal_signature");
  m_signals.erase("configure");

  regist_signal("configure", "Update component options")->signal->connect(boost::bind(&CNode::configure_reply, this, _1));
  regist_signal("tree_updated", "Event that notifies a path has changed")->signal->connect(boost::bind(&CNode::update_tree, this, _1));
  regist_signal("list_content", "Updates node contents")->signal->connect(boost::bind(&CNode::list_content_reply, this, _1));

  regist_signal("signal_signature", "");

  signal("signal_signature")->is_hidden = true;

  m_properties.add_property("originalComponentType", m_componentType.toStdString());
}

////////////////////////////////////////////////////////////////////////////

QString CNode::getComponentType() const
{
  return m_componentType;
}

////////////////////////////////////////////////////////////////////////////

CNode::Type CNode::type() const
{
  return m_type;
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
        Option::Ptr opt = makeOption(node);
        cf_assert( opt.get() != nullptr );
        m_properties.store[ opt->name() ] = opt;
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

        m_actionSigs.append(si);

      }

      sig_map.content.content = node->next_sibling();
    }

  }
}

////////////////////////////////////////////////////////////////////////////

template<typename TYPE>
void addValueToXml(const std::string& name, const std::string& value, bool is_array,
                   SignalFrame& options)
{
  if( is_array )
  {
    std::vector<TYPE> data;
    Map::split_string(value, "@@", data);

    options.set_array(name, data, " ; ");
  }
  else
  {
    try
    {
      options.set_option(name, from_str<TYPE>(value));
    }
    catch( const boost::bad_lexical_cast & e)
    {
      throw CastingFailed(FromHere(), "Unable to cast [" + value + "] to " +
                          Protocol::Tags::type<TYPE>() + ".");
    }
  }
}

//////// ////////////////////////////////////////////////////////////////////

void CNode::modifyOptions(const QMap<QString, QString> & opts)
{
  QMutexLocker locker(m_mutex);

  QMap<QString, QString>::const_iterator it = opts.begin();

  // for better readability of the code, a SignalFrame is built even when
  // modifying local options. It's not the most efficient way to do, but local
  // options are not meant to be modified 10 times each second so this should
  // not affect the application performances.
  SignalFrame frame("configure", full_path(), full_path());
  SignalFrame& options = frame.map( Protocol::Tags::key_options() );

  for( ; it != opts.end() ; it++)
  {
    Option& option = m_properties.option( it.key().toStdString() );
    std::string type( option.type() );
    std::string name( it.key().toStdString() );
    std::string value( it.value().toStdString() );
    bool is_array = std::strcmp(option.tag(), "array") == 0;

    // if it is an array, we need to get the element type
    if(is_array)
    {
      OptionArray * optArray;

      optArray = static_cast<OptionArray*>(&option);

      cf_assert(optArray != nullptr);

      type = optArray->elem_type();
    }

    if(type == Protocol::Tags::type<bool>())              // bool
      addValueToXml<bool>(name, value, is_array, options);
    else if(type == Protocol::Tags::type<int>())          // int
      addValueToXml<int>(name, value, is_array, options);
    else if(type == Protocol::Tags::type<Uint>())         // Uint
      addValueToXml<Uint>(name, value, is_array, options);
    else if(type == Protocol::Tags::type<Real>())         // Real
      addValueToXml<Real>(name, value, is_array, options);
    else if(type == Protocol::Tags::type<std::string>())  // string
      addValueToXml<std::string>(name, value, is_array, options);
    else if(type == Protocol::Tags::type<URI>())          // URI
      addValueToXml<URI>(name, value, is_array, options);
    else
      throw ValueNotFound(FromHere(), type + ": Unknown type for option [" + name + "]." );
  }

  // if there were options to modify
  if(!opts.isEmpty())
  {
    if( isLocalComponent() )
      signal_configure( frame );
    else
      ThreadManager::instance().network().send(frame);
  }
}

////////////////////////////////////////////////////////////////////////////

void CNode::localSignature(const QString & name, SignalArgs& node )
{
  ( *signal( name.toStdString() )->signature )(node);
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

  Component::Ptr compo = shared_from_this();
  CF::Uint i;

  if(checkType(CNode::ROOT_NODE))
    compo = this->castTo<NRoot>()->root();

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

  ComponentIterator<const CNode> itBegin = this->begin<const CNode>();
  ComponentIterator<const CNode> itEnd = this->end<const CNode>();

  // add the current path
  if(this->checkType(ROOT_NODE))
  {
    CRoot::ConstPtr root = this->castTo<const NRoot>()->root();
    itBegin = root->begin<const CNode>();
    itEnd = root->end<const CNode>();

    list << root->full_path().path().c_str();
  }
  else if(list.isEmpty())
    list << this->full_path().path().c_str();

  for( ; itBegin != itEnd ; itBegin++)
  {
    if(!itBegin->isLocalComponent() || clientNodes)
    {
      list << itBegin->full_path().path().c_str();

      if(recursive)
        itBegin->listChildPaths(list, recursive, clientNodes);
    }

  }
}

////////////////////////////////////////////////////////////////////////////

void CNode::addNode(CNode::Ptr node)
{
  QMutexLocker locker(m_mutex);

  if(checkType(ROOT_NODE))
    ((NRoot *)this)->root()->add_component(node);
  else
    this->add_component(node);

  m_notifier->notifyChildCountChanged();
}

////////////////////////////////////////////////////////////////////////////

void CNode::removeNode(const QString & nodeName)
{
  QMutexLocker locker(m_mutex);

  if(checkType(ROOT_NODE))
    ((NRoot *)this)->root()->remove_component(nodeName.toStdString());
  else
    this->remove_component(nodeName.toStdString());

  m_notifier->notifyChildCountChanged();
}

////////////////////////////////////////////////////////////////////////////

void CNode::configure_reply(SignalArgs & args)
{
  signal_configure(args);
  NTree::globalTree()->optionsChanged(this->full_path());
  NLog::globalLog()->addMessage(QString("Node \"%1\" options updated.").arg(full_path().path().c_str()));
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

////////////////////////////////////////////////////////////////////////////

void CNode::listProperties(QMap<QString, QString> & props)
{
  QMutexLocker locker(m_mutex);

  if(!m_contentListed)
    fetchContent();
  else
  {
    PropertyList::PropertyStorage_t::const_iterator it = m_properties.store.begin();

    props.clear();

    for( ; it != m_properties.store.end() ; it++)
      props[ it->first.c_str() ] = it->second->value_str().c_str();
  }

}

////////////////////////////////////////////////////////////////////////////

void CNode::listSignals(QList<ActionInfo> & actions)
{
  QMutexLocker locker(m_mutex);

  if(!m_contentListed)
    fetchContent();
  else
  {
    QStringList::const_iterator it = m_localSignals.begin();


    // put the local signals
    for( ; it != m_localSignals.end() ; it++)
    {

      if(m_signals.find(it->toStdString()) != m_signals.end())
      {
        ActionInfo ai;
        SignalPtr sig = m_signals.find(it->toStdString())->second;

        ai.name = it->toStdString().c_str();
        ai.description = sig->description.c_str();
        ai.readableName = sig->readable_name.c_str();
        ai.isLocal = true;

        actions.append(ai);
      }
      else
        NLog::globalLog()->addError(*it + ": local signal not found");

    }

    actions.append(m_actionSigs); // copy the "remote signals"
  }
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
  else if( typeName == "CJournal" )
    rootNode = boost::shared_ptr<NJournal>(new NJournal(nodeName));
  else if( typeName == "CRoot" )
    rootNode = boost::shared_ptr<NRoot>(new NRoot(nodeName));
  else if( typeName == "CPlotXY" )
    rootNode = boost::shared_ptr<NPlotXY>(new NPlotXY(nodeName));
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
      NLog::globalLog()->addException(e.msg().c_str());
    }

    child.content = child.content->next_sibling();
  }

  return rootNode;
}

////////////////////////////////////////////////////////////////////////////

/// Creates an @c #OptionT option with a value of type TYPE.
/// @param name Option name
/// @param descr Option description
/// @param node The value node. If it has a sibling node, this node is taken
/// the restricted values list.
/// @return Returns the created option.

template<typename TYPE>
Option::Ptr makeOptionT(const std::string & name, const std::string & descr, XmlNode & node)
{
  TYPE value;
  to_value(node, value);
  XmlNode restr_node = Map(node.content->parent()).seek_value(Protocol::Tags::key_restricted_values(), Protocol::Tags::node_array());

  std::vector<TYPE> restr_list;

  Option::Ptr option(new Common::OptionT<TYPE>(name, descr, value));


  if(restr_node.is_valid())
  {
    restr_list = Map().array_to_vector<TYPE>( restr_node );

    typename std::vector<TYPE>::iterator it;

//    option->restricted_list().push_back( value );

    for( it = restr_list.begin() ; it != restr_list.end() ; ++it)
      option->restricted_list().push_back( *it );
  }

  return option;
}

////////////////////////////////////////////////////////////////////////////

/// Creates an @c #OptionArrayT option with values of type TYPE.
/// @param name Option name
/// @param descr Option description
/// @param node The value node. If it has a sibling node, this node is taken
/// the restricted values list.
/// @return Returns the created option.

template<typename TYPE>
typename OptionArrayT<TYPE>::Ptr makeOptionArrayT(const std::string & name,
                                                         const std::string & descr,
                                                         const XmlNode & node)
{
  std::vector<TYPE> value = Map().array_to_vector<TYPE>(node);

  typename OptionArrayT<TYPE>::Ptr option(new OptionArrayT<TYPE>(name, descr, value));

  XmlNode restr_node = Map(node.content->parent()).seek_value(Protocol::Tags::key_restricted_values(), Protocol::Tags::node_array());

  std::vector<TYPE> restr_list;

  if(restr_node.is_valid())
  {
    restr_list = Map().array_to_vector<TYPE>( restr_node );

    typename std::vector<TYPE>::iterator it;

    option->restricted_list().push_back( value );

    for( it = restr_list.begin() ; it != restr_list.end() ; ++it)
      option->restricted_list().push_back( *it );
  }


  return option;
}

////////////////////////////////////////////////////////////////////////////

Option::Ptr CNode::makeOption(const XmlNode & node)
{
  cf_assert( node.is_valid() );

  bool advanced;
  Option::Ptr option;
  std::string type( Map::get_value_type(node) );
  rapidxml::xml_attribute<>* keyAttr = node.content->first_attribute( Protocol::Tags::attr_key() );
  rapidxml::xml_attribute<>* descrAttr = node.content->first_attribute( Protocol::Tags::attr_descr() );

  rapidxml::xml_attribute<>* modeAttr = node.content->first_attribute( "mode" );
  advanced = modeAttr == nullptr || std::strcmp(modeAttr->value(), "adv") == 0;

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
        rapidxml::xml_attribute<>* protsAttr = nullptr;
        std::vector<std::string> prots;
        std::vector<std::string>::iterator it;
        OptionURI::Ptr optURI;

        to_value(type_node, value);
        XmlNode restr_node = Map(node).seek_value(Protocol::Tags::key_restricted_values(), Protocol::Tags::node_array());

        std::vector<URI> restr_list;

        optURI = OptionURI::Ptr (new Common::OptionURI(key_str, descr_str, value));

        if(restr_node.is_valid())

        {
          restr_list = Map().array_to_vector<URI>( restr_node );

          std::vector<URI>::iterator it;

          option->restricted_list().push_back( value );

          for( it = restr_list.begin() ; it != restr_list.end() ; ++it)
            option->restricted_list().push_back( *it );
        }

        protsAttr = node.content->first_attribute( Protocol::Tags::attr_uri_protocols());

        if( is_not_null(protsAttr) && protsAttr->value_size() != 0 )
        {
          std::string prots_str(protsAttr->value());

          boost::algorithm::split(prots, prots_str, boost::algorithm::is_any_of(","));

          for(it = prots.begin() ; it != prots.end() ; it++)
          {
            URI::Scheme::Type scheme = URI::Scheme::Convert::instance().to_enum(*it);

            if( scheme == URI::Scheme::INVALID )
              throw CastingFailed(FromHere(), "[" + *it + "] is not a supported scheme.");

            optURI->supported_protocol(scheme);
          }
        }

        option = optURI;

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

////////////////////////////////////////////////////////////////////////////

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
    URI path;

    if( m_type == ROOT_NODE )
      path = URI(CLIENT_ROOT_PATH, URI::Scheme::CPATH);
    else
      path = full_path();

    SignalFrame frame("list_content", path, path);

    network.send(frame);
    m_listingContent = true;
  }
}

////////////////////////////////////////////////////////////////////////////

} // ClientCore
} // GUI
} // CF

//////////////////////////////////////////////////////////////////////////////

#undef ADD_ARRAY_TO_XML
