// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/date_time/posix_time/posix_time.hpp>

#include "rapidxml/rapidxml.hpp"

#include "common/BoostAnyConversion.hpp"
#include "common/Builder.hpp"
#include "common/Signal.hpp"
#include "common/Core.hpp"
#include "common/NetworkInfo.hpp"
#include "common/Log.hpp"
#include "common/LibCommon.hpp"
#include "common/OptionT.hpp"
#include "common/XML/Protocol.hpp"
#include "common/XML/XmlDoc.hpp"
#include "common/XML/FileOperations.hpp"

#include "common/Journal.hpp"

using namespace cf3::common::XML;

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {

///////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < Journal, Component, LibCommon > Journal_Builder;

////////////////////////////////////////////////////////////////////////////////

Journal::Journal (const std::string & name)
  : Component(name),
    m_xmldoc(Protocol::create_doc())
{
  regist_signal( "list_journal" )
    .description("Lists all journal entries")
    .pretty_name("List journal").connect( boost::bind( &Journal::list_journal, this, _1) );

  regist_signal( "load_journal" )
    .description("Loads the journal entries from file")
    .pretty_name("Load journal").connect( boost::bind( &Journal::load_journal, this, _1) );

  regist_signal( "save_journal" )
    .description("Saves all journal entries")
    .pretty_name("Save journal").connect( boost::bind( &Journal::save_journal, this, _1) );

  signal("list_journal")->hidden(true);

  options().add("RecordReplies", false)
      .description("If true, both signal and reply frames are recorded. If "
                        "false, only signal frames are recorded.\nRecording replies "
                        "will significantly increase the journal size and the memory used.");

  options()["RecordReplies"].mark_basic();

  XmlNode doc_node = Protocol::goto_doc_node(*m_xmldoc.get());
  const char * tag_map = Protocol::Tags::node_map();

  m_info_node = Map(doc_node.add_node( tag_map ));
  m_signals_map = Map(doc_node.add_node( tag_map ));

  m_signals_map.content.set_attribute( "key", "signals" );

  m_info_node.content.set_attribute( Protocol::Tags::attr_key(), "journalInfo");

  m_info_node.set_value( "hostname", class_name<std::string>(), Core::instance().network_info().hostname() );
  m_info_node.set_value( "port", common::class_name<std::string>(), to_str(static_cast<Uint>(Core::instance().network_info().port())));
}

////////////////////////////////////////////////////////////////////////////////

Journal::~Journal()
{

}

////////////////////////////////////////////////////////////////////////////////

boost::shared_ptr<Journal> Journal::create_from_file ( const std::string & name,
                                         const URI& file_path )
{
  boost::shared_ptr<Journal> journal( allocate_component<Journal>(name) );

  journal->load_journal_file(file_path);

  return journal;
}

////////////////////////////////////////////////////////////////////////////////

void Journal::load_journal_file ( const URI& file_path )
{
  /// @todo handle m_info_node and m_signals_map

  m_signals.clear();
  m_xmldoc = XML::parse_file(file_path);
}

////////////////////////////////////////////////////////////////////////////////

void Journal::dump_journal_to ( const URI& file_path ) const
{
  XML::to_file( *m_xmldoc, file_path );

  CFinfo << "Journal dumped to '" << file_path.string() << "'" << CFendl;
}

////////////////////////////////////////////////////////////////////////////////

void Journal::add_signal ( const SignalArgs & signal_node )
{
  rapidxml::xml_attribute<> * type_attr = signal_node.node.content->first_attribute("type");

  if( options()["RecordReplies"].value<bool>() ||
     (type_attr != nullptr && std::strcmp(type_attr->value(), "signal") == 0) )
  {
    XmlNode copy = copy_node(signal_node.node, m_signals_map.content);

    boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
    copy.set_attribute("time", boost::posix_time::to_simple_string(now));
  }
}

////////////////////////////////////////////////////////////////////////////////

void Journal::execute_signals (const URI& filename)
{
  boost::shared_ptr<XmlDoc> xmldoc = XML::parse_file(filename);
  XmlNode doc_node = Protocol::goto_doc_node(*xmldoc.get());
//  rapidxml::xml_node * signals_map = doc_node.content->first_node();
//  bool found_map = false;
  rapidxml::xml_node<>* node = nullptr;
//  rapidxml::xml_attribute<>* key_attr = nullptr;
  Component& root = Core::instance().root();
  const char * frame_tag = Protocol::Tags::node_frame();

  XmlNode signal_map = Map(doc_node).find_value( Protocol::Tags::key_signals() );

//  for( ; signals_map != nullptr ; signals_map = signals_map->next_sibling())
//  {
//    key_attr = signals_map->first_attribute("key");
//    found_map = key_attr != nullptr && std::strcmp(key_attr->value(), "signals") == 0;

//    if(found_map)
//      break;
//  }

  if( !signal_map.is_valid() )
    throw XmlError(FromHere(), "Could not find \'signals\' map.");

  node = signal_map.content->first_node( frame_tag );

  for( ; node != nullptr ; node = node->next_sibling(frame_tag) )
  {
    rapidxml::xml_attribute<>* type_attr = node->first_attribute("type");

    if(type_attr != nullptr && std::strcmp(type_attr->value(), "signal") == 0)
    {
      rapidxml::xml_attribute<>* target_attr = node->first_attribute("target");
      rapidxml::xml_attribute<>* receiver_attr = node->first_attribute("receiver");

      std::string target = target_attr != nullptr ? target_attr->value() : "";
      std::string receiver = receiver_attr != nullptr ? receiver_attr->value() : "";

      if(target.empty())
        CFwarn << "Warning: missing or empty target. Skipping this signal." << CFendl;

      if(receiver.empty())
        CFwarn << "Warning: missing or empty receiver. Skipping this signal." << CFendl;

      if(receiver == "//Core") // server specific component
        continue;

      try
      {
        SignalFrame sf(node);
        root.access_component(receiver)->call_signal(target, sf);
      }
      catch(Exception & e)
      {
        CFerror << e.what() << CFendl;
      }

    }
  }

}

////////////////////////////////////////////////////////////////////////////////

void Journal::list_journal ( SignalArgs & args )
{
  SignalFrame reply = args.create_reply( uri() );

  copy_node(m_signals_map.content, reply.main_map.content);
}

////////////////////////////////////////////////////////////////////////////////

void Journal::load_journal ( SignalArgs & args )
{
  throw NotImplemented(FromHere(), "Journal::load_journal()");
}

////////////////////////////////////////////////////////////////////////////////

void Journal::save_journal ( SignalArgs & args )
{
  URI file_path("./server-journal.xml", URI::Scheme::FILE);
  XML::to_file( *m_xmldoc, file_path );

  CFinfo << "Journal dumped to '" << file_path.string() << "'" << CFendl;
}

////////////////////////////////////////////////////////////////////////////////

XmlNode Journal::copy_node(const XmlNode & in, XmlNode & out) const
{
  rapidxml::xml_node<>* content = in.content;

  XmlNode copy = out.add_node(content->name(), content->value());
  rapidxml::xml_attribute<>* attr = content->first_attribute();
  XmlNode node( content->first_node() );

  while( attr != nullptr )
  {
    copy.set_attribute(attr->name(), attr->value());
    attr = attr->next_attribute();
  }


  while( node.is_valid() )
  {
    copy_node(node, copy);

    node = node.content->next_sibling();
  }

  return copy;
}


////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

