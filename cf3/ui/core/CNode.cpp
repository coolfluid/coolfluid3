
// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
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

#include "common/BoostAnyConversion.hpp"
#include "common/CF.hpp"
#include "common/Signal.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"
#include "common/OptionURI.hpp"
#include "common/StringConversion.hpp"
#include "common/FindComponents.hpp"
#include "common/UUCount.hpp"

#include "common/XML/CastingFunctions.hpp"
#include "common/XML/FileOperations.hpp"
#include "common/XML/Protocol.hpp"
#include "common/XML/SignalOptions.hpp"

#include "ui/uicommon/ComponentNames.hpp"

#include "ui/core/CNodeBuilders.hpp"
#include "ui/core/NetworkQueue.hpp"
#include "ui/core/NetworkThread.hpp"
#include "ui/core/NGeneric.hpp"
#include "ui/core/NJournal.hpp"
#include "ui/core/NLog.hpp"
#include "ui/core/NLink.hpp"
#include "ui/core/NRoot.hpp"
#include "ui/core/NTree.hpp"
#include "ui/core/ThreadManager.hpp"

#include "ui/core/CNode.hpp"

//////////////////////////////////////////////////////////////////////////////

using namespace cf3::common;
using namespace cf3::common::XML;

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace core {

/////////////////////////////////////////////////////////////////////////////

CNodeNotifier::CNodeNotifier( CNode * parent )
  : m_parent( parent )
{
  qRegisterMetaType<SignalArgs>("cf3::common::SignalArgs&");
}

////////////////////////////////////////////////////////////////////////////

void CNodeNotifier::notify_child_count_changed()
{
  emit child_count_changed();
}

////////////////////////////////////////////////////////////////////////////

void CNodeNotifier::notify_signal_signature( SignalArgs & node )
{
  emit signal_signature( node );
}

////////////////////////////////////////////////////////////////////////////

CNode::CNode( const std::string & name, const QString & component_type, Type type )
  : Component( name ),
    m_notifier( new CNodeNotifier( this ) ),
    m_component_type( component_type ),
    m_type( type ),
    m_listing_content( false ),
    m_is_root( false )
{
  m_content_listed = is_local_component();
  m_mutex = new QMutex();

  // unregister some base class signals

  unregist_signal( "signal_signature" );
  unregist_signal( "configure" );

  regist_signal( "configure" )
    .description("Update component options")
    .connect(boost::bind(&CNode::reply_configure, this, _1));

  regist_signal( "tree_updated" )
    .description("Event that notifies a path has changed")
    .connect(boost::bind(&CNode::reply_update_tree, this, _1));

  regist_signal( "list_content" )
    .description("Updates node contents")
    .connect(boost::bind(&CNode::reply_list_content, this, _1));

  regist_signal("signal_signature")
      .connect(boost::bind(&CNode::reply_signal_signature, this, _1))
      .hidden(true);

  properties().add_property( "original_component_type", m_component_type.toStdString() );
}

////////////////////////////////////////////////////////////////////////////

QString CNode::component_type() const
{
  return m_component_type;
}

////////////////////////////////////////////////////////////////////////////

void CNode::set_properties( const SignalArgs & options )
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
        boost::shared_ptr<Option> opt = SignalOptions::xml_to_option(curr_node);
        cf3_assert( opt.get() != nullptr );
        this->options().store[ opt->name() ] = opt;
      }
      else // it is a property
      {
        rapidxml::xml_attribute<> * key_attr = node->first_attribute( Protocol::Tags::attr_key() );

        if ( key_attr != nullptr )
        {
          const char * key_val = key_attr->value();
          std::string typ_val = Map::get_value_type(curr_node); // type name
          rapidxml::xml_node<>* first_node = curr_node.content->first_node();

          if( is_not_null(first_node) )
          {
            const char * value = first_node->value();

            if(properties().check(key_val))
            {
              if( typ_val == Protocol::Tags::type<bool>() )
                properties().configure_property(key_val, from_str<bool>(value));
              else if( typ_val == Protocol::Tags::type<int>() )
                properties().configure_property(key_val, from_str<int>(value));
              else if( typ_val == Protocol::Tags::type<cf3::Uint>() )
                properties().configure_property(key_val, from_str<cf3::Uint>(value));
              else if( typ_val == Protocol::Tags::type<cf3::Real>() )
                properties().configure_property(key_val, from_str<cf3::Real>(value));
              else if( typ_val == Protocol::Tags::type<std::string>() )
                properties().configure_property(key_val, std::string(value));
              else if( typ_val == Protocol::Tags::type<URI>() )
                properties().configure_property(key_val, from_str<URI>(value));
              else if( typ_val == Protocol::Tags::type<UUCount>() )
                properties().configure_property(key_val, from_str<UUCount>(value));
              else
                throw ShouldNotBeHere(FromHere(), typ_val + ": Unknown type.");
            }
            else
            {

              if( typ_val == Protocol::Tags::type<bool>() )
                properties().add_property(key_val, from_str<bool>(value));
              else if( typ_val == Protocol::Tags::type<int>() )
                properties().add_property(key_val, from_str<int>(value));
              else if( typ_val == Protocol::Tags::type<cf3::Uint>() )
                properties().add_property(key_val, from_str<cf3::Uint>(value));
              else if( typ_val == Protocol::Tags::type<cf3::Real>() )
                properties().add_property(key_val, from_str<cf3::Real>(value));
              else if( typ_val == Protocol::Tags::type<std::string>() )
                properties().add_property(key_val, std::string(value));
              else if( typ_val == Protocol::Tags::type<URI>() )
                properties().add_property(key_val, from_str<URI>(value));
              else if( typ_val == Protocol::Tags::type<UUCount>() )
                properties().configure_property(key_val, from_str<UUCount>(value));
              else
                throw ShouldNotBeHere(FromHere(), typ_val + ": Unknown type.");
            }
          } // end of "it( is_not_null(first_node) )"
        }
      } // end of "else"
    }
  } // end of "if( options.has_map() )"
}

////////////////////////////////////////////////////////////////////////////

void CNode::set_signals( const SignalArgs & args )
{
  QMutexLocker locker(m_mutex);

  if( args.has_map( Protocol::Tags::key_signals() ) )
  {
    Map sig_map = args.map( Protocol::Tags::key_signals() ).main_map;

    m_action_sigs.clear();

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
        cf3_always_assert( key_attr != nullptr );
        cf3_always_assert( key_attr->value()[0] != '\0');

        si.name = key_attr->value();
        si.readable_name = name_attr != nullptr ? name_attr->value() : si.name;
        si.description = desc_attr != nullptr ? desc_attr->value() : "";
        si.is_local = false;
        si.is_enabled = true;

        m_action_sigs.append(si);

      }

      sig_map.content.content = node->next_sibling();
    }

  }
}

////////////////////////////////////////////////////////////////////////////

template<typename TYPE>
void addValueToXml( const std::string& name,
                    const std::string& value,
                    const std::string& sep,
                    SignalOptions& options )
{
  // if it is an array, the separator is not empty
  if( !sep.empty() )
  {
    std::vector<TYPE> data;
    Map::split_string(value, sep, data);

    options.add_option(name, data);
  }
  else
  {
    try
    {
      options.add_option(name, from_str<TYPE>(value));
    }
    catch( const boost::bad_lexical_cast & e)
    {
      throw CastingFailed(FromHere(), "Unable to cast [" + value + "] to " +
                          Protocol::Tags::type<TYPE>() + ".");
    }
  }
}

////////////////////////////////////////////////////////////////////////////

void CNode::modify_options( const QMap<QString, QString> & opts )
{
  QMutexLocker locker(m_mutex);

  QMap<QString, QString>::const_iterator it = opts.begin();

  QMap<QString, QString>::const_iterator it2 = opts.begin();

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

    if( !this->options().check(name) )
      throw ValueNotFound(FromHere(), "Could not find an option with name ["+name+"]");

    Option& option = this->options().option(name);
    std::string type( option.type() );
    bool is_array = std::strcmp(option.tag(), "array") == 0;

    // if it is an array, we need to get the element type
    if(is_array)
    {
      type = option.element_type();
      sep = option.separator();
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
        options.add_option(name, from_str<URI>(value));
      else
      {
        std::vector<URI> data;
        Map::split_string(value, sep, data);
        options.add_option(name, data);
      }
    }
    else
      throw ValueNotFound(FromHere(), type + ": Unknown type for option [" + name + "]." );
  }

  // if there were options to modify
  if( !options.store.empty() )
  {
    SignalFrame frame = options.create_frame("configure", uri(), uri());

    if( is_local_component() )
      signal_configure( frame );
    else
      NetworkQueue::global()->send( frame );
  }
}

////////////////////////////////////////////////////////////////////////////

void CNode::local_signature( const QString & name, SignalArgs& node )
{
  std::string sname = name.toStdString();
  ( * signal( sname )->signature() ) ( node );
}

////////////////////////////////////////////////////////////////////////////

void CNode::finish_setup()
{
  SignalFrame frame;
  call_signal("setUpFinished", frame);
}

////////////////////////////////////////////////////////////////////////////

boost::shared_ptr< CNode > CNode::create_from_xml( XmlNode args )
{
  QMap<boost::shared_ptr< NLink >, URI> link_targets;
  QMap<boost::shared_ptr< NLink >, URI>::iterator it;
  boost::shared_ptr< CNode > root_node;

  root_node = create_from_xml_recursive(args, link_targets);

  it = link_targets.begin();

  for( ; it != link_targets.end() ; it++)
    it.key()->set_target_path(it.value());

  return root_node;
}

////////////////////////////////////////////////////////////////////////////

Handle< CNode > CNode::child(cf3::Uint index)
{
  QMutexLocker locker(m_mutex);

  ComponentIterator<CNode> it = component_begin<CNode>(*this);

  cf3_assert(index < count_children());

  for(Uint i = 0 ; i < index ; i++)
    it++;

  return it.get();
}

////////////////////////////////////////////////////////////////////////////

void CNode::connect_notifier( QObject * reciever,
                              const char * signal,
                              const char * slot )
{
  QObject::connect( m_notifier, signal, reciever, slot, Qt::DirectConnection );
}

////////////////////////////////////////////////////////////////////////////

CNodeNotifier * CNode::notifier() const
{
  return m_notifier;
}

////////////////////////////////////////////////////////////////////////////

void CNode::list_child_paths( QStringList & list,
                              bool recursive,
                              bool client_nodes ) const
{
  QMutexLocker locker(m_mutex);

  ComponentIterator<const CNode> it_begin = component_begin<const CNode>(*this);
  ComponentIterator<const CNode> it_end   = component_end<const CNode>(*this);

  // add the current path
  if(list.isEmpty())
    list << uri().path().c_str();

  for( ; it_begin != it_end ; it_begin++)
  {
    if(!it_begin->is_local_component() || client_nodes)
    {
      list << it_begin->uri().path().c_str();

      if(recursive)
        it_begin->list_child_paths(list, recursive, client_nodes);
    }

  }
}

////////////////////////////////////////////////////////////////////////////

void CNode::add_node(boost::shared_ptr< CNode > node)
{
  QMutexLocker locker(m_mutex);

  add_component(node);

  m_notifier->notify_child_count_changed();
}

////////////////////////////////////////////////////////////////////////////

void CNode::remove_node(const QString & nodeName)
{
  QMutexLocker locker(m_mutex);

  remove_component( nodeName.toStdString() );

  m_notifier->notify_child_count_changed();
}

////////////////////////////////////////////////////////////////////////////

void CNode::reply_configure(SignalArgs & args)
{
  URI path = uri();
  QString msg("Node \"%1\" options updated.");

  signal_configure(args);

  NTree::global()->options_changed( path );
  NLog::global()->add_message( msg.arg(path.string().c_str()) );
}

////////////////////////////////////////////////////////////////////////////

void CNode::reply_list_content( SignalArgs & node )
{
  set_properties(node);
  set_signals(node);

  m_content_listed = true;
  m_listing_content = false;

  NTree::global()->content_listed( handle<Component>() );
}

////////////////////////////////////////////////////////////////////////////

void CNode::reply_signal_signature( SignalArgs & node )
{
  m_notifier->notify_signal_signature( node );
}

////////////////////////////////////////////////////////////////////////////

void CNode::list_options(QList<boost::shared_ptr< Option > > & list)
{
  QMutexLocker locker( m_mutex );

 if( !m_content_listed )
   fetch_content();
 else
 {
   OptionList::const_iterator it = options().begin();

   for( ; it != options().end() ; ++it)
     list.append( it->second );
 }
}

////////////////////////////////////////////////////////////////////////////

void CNode::list_properties(QMap<QString, QString> & props)
{
  QMutexLocker locker( m_mutex );

  if( !m_content_listed )
    fetch_content();
  else
  {
    PropertyList::const_iterator it_prop = properties().begin();
    OptionList::const_iterator it_opt = options().begin();

    props.clear();

    // add the properties
    for( ; it_prop != properties().end() ; ++it_prop)
      props[ it_prop->first.c_str() ] =  any_to_str(it_prop->second).c_str();

    // add the options
    for( ; it_opt != options().end() ; ++it_opt)
      props[ it_opt->first.c_str() ] = it_opt->second->value_str().c_str();
  }

}

////////////////////////////////////////////////////////////////////////////

void CNode::list_signals(QList<ActionInfo> & actions)
{
  QMutexLocker locker( m_mutex );

  QStringList::const_iterator it = m_local_signals.begin();
  QMap<QString, bool> available_local_signals;

  for( ; it != m_local_signals.end() ; it++)
    available_local_signals[*it] = true;

  this->disable_local_signals(available_local_signals);

  it = m_local_signals.begin();

  // put the local signals
  for( ; it != m_local_signals.end() ; it++)
  {

    std::string sname = it->toStdString();

    if( signal_exists( sname ) )
    {
      ActionInfo ai;
      SignalPtr sig = signal( sname );

      ai.name = sname.c_str();
      ai.description = sig->description().c_str();
      ai.readable_name = sig->pretty_name().c_str();
      ai.is_local = true;
      ai.is_enabled = available_local_signals[*it];

      actions.append(ai);
    }
    else
      NLog::global()->add_error(*it + ": local signal not found");
  }

  // if content has not been listed yet, we fetch
  if(!m_content_listed)
    fetch_content();
  else
    actions.append(m_action_sigs); // copy the "remote signals"
}

////////////////////////////////////////////////////////////////////////////

boost::shared_ptr< CNode > CNode::create_from_xml_recursive( XmlNode & node,
                                             QMap<boost::shared_ptr< NLink >, URI> & link_targets )
{
  rapidxml::xml_attribute<>* typeAttr = node.content->first_attribute("atype");
  rapidxml::xml_attribute<>* nameAttr = node.content->first_attribute("name");
  rapidxml::xml_attribute<>* mode_attr = node.content->first_attribute("mode");

  UUCount uuid(node.attribute_value( "uuid" ));

  cf3_always_assert(typeAttr != nullptr);
  cf3_always_assert(nameAttr != nullptr);

  QString type_name = typeAttr->value();
  char * node_name = nameAttr->value();
  XmlNode child( node.content->first_node() );

  cf3_assert( !type_name.isEmpty() );
  cf3_assert( node_name != nullptr );
  cf3_assert( type_name.length() > 0 );
  cf3_assert( std::strlen(node_name) > 0 );

  boost::shared_ptr< CNode > root_node;

  if( type_name == "CCore" )
    return root_node;

  if( type_name == "Link" )
  {
    boost::shared_ptr< NLink > link = boost::shared_ptr<NLink>(new NLink(node_name));
    root_node = link;
    link_targets[link] = node.content->value();
  }
  else if( CNodeBuilders::instance().has_builder( type_name ) )
    root_node = CNodeBuilders::instance().build_cnode(type_name, node_name);
  else if( type_name == "Journal" )
    root_node = boost::shared_ptr<NJournal>(new NJournal(node_name));
  else if( type_name == "Root" )
    root_node = boost::shared_ptr<NRoot>(new NRoot(node_name));
  else
    root_node = boost::shared_ptr<NGeneric>(new NGeneric(node_name, type_name));

  if(mode_attr != nullptr && std::strcmp(mode_attr->value(), "basic") == 0)
    root_node->mark_basic();

  if( !uuid.is_nil() )
    root_node->properties().configure_property( "uuid", uuid );
  else
    NLog::global()->add_warning( "Found a Component without no UuiD." );

  while( child.is_valid() )
  {
    try
    {
      boost::shared_ptr< CNode > node = create_from_xml_recursive(child, link_targets);

      if(node.get() != nullptr)
      {
        root_node->add_node(node);
        node->setup_finished();
      }
    }
    catch (Exception & e)
    {
      NLog::global()->add_exception(e.msg().c_str());
    }

    child.content = child.content->next_sibling();
  }

  return root_node;
}

////////////////////////////////////////////////////////////////////////////

void CNode::request_signal_signature(const QString & name)
{
  URI path = uri();
  XmlNode * node;

  SignalFrame frame("signal_signature", path, path);

  frame.map( Protocol::Tags::key_options() ).set_option("name", name.toStdString());

  NetworkQueue::global()->send( frame, NetworkQueue::IMMEDIATE );
}

////////////////////////////////////////////////////////////////////////////

void CNode::reply_update_tree(SignalArgs & node)
{
  NTree::global()->update_tree();
}

////////////////////////////////////////////////////////////////////////////

void CNode::fetch_content()
{
  NetworkThread& network = ThreadManager::instance().network();
  if(!m_content_listed && !m_listing_content && network.is_connected())
  {
    URI path = uri();

    SignalFrame frame("list_content", path, path);

    network.send(frame);
    m_listing_content = true;
  }
}

////////////////////////////////////////////////////////////////////////////

} // Core
} // ui
} // cf3

//////////////////////////////////////////////////////////////////////////////
