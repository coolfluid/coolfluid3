// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/date_time/posix_time/posix_time.hpp>

#include "rapidxml/rapidxml.hpp"

#include "Common/CBuilder.hpp"
#include "Common/Core.hpp"
#include "Common/NetworkInfo.hpp"
#include "Common/Log.hpp"
#include "Common/LibCommon.hpp"
#include "Common/OptionT.hpp"
#include "Common/XML/Protocol.hpp"
#include "Common/XML/XmlDoc.hpp"
#include "Common/XML/FileOperations.hpp"

#include "Common/CJournal.hpp"

using namespace CF::Common::XML;

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

///////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < CJournal, Component, LibCommon > CJournal_Builder;

////////////////////////////////////////////////////////////////////////////////

CJournal::CJournal (const std::string & name)
  : Component(name),
    m_xmldoc(Protocol::create_doc())
{
  regist_signal("list_journal", "Lists all journal entries.", "List journal")->
      connect( boost::bind( &CJournal::list_journal, this, _1) );
  regist_signal("load_journal", "Loads the journal entries from file.", "Load journal")->
      connect( boost::bind( &CJournal::load_journal, this, _1) );
  regist_signal("save_journal", "Saves all journal entries.", "Save journal")->
      connect( boost::bind( &CJournal::save_journal, this, _1) );

  signal("list_journal").is_hidden = true;

  m_properties.add_option< OptionT<bool> >("RecordReplies", "If true, both signal and reply frames are recorded. If false, only signal frames are recorded.\nRecording replies will significantly increase the journal size and the memory used.", false);

  m_properties["RecordReplies"].as_option().mark_basic();

  XmlNode doc_node = Protocol::goto_doc_node(*m_xmldoc.get());
  const char * tag_map = Protocol::Tags::node_map();

  m_info_node = Map(doc_node.add_node( tag_map ));
  m_signals_map = Map(doc_node.add_node( tag_map ));

  m_signals_map.content.set_attribute( "key", "signals" );

  m_info_node.content.set_attribute( Protocol::Tags::attr_key(), "journalInfo");

  m_info_node.set_value( "hostname", Core::instance().network_info()->hostname() );
  m_info_node.set_value( "port", (Uint)Core::instance().network_info()->port());
}

////////////////////////////////////////////////////////////////////////////////

CJournal::~CJournal()
{

}

////////////////////////////////////////////////////////////////////////////////

CJournal::Ptr CJournal::create_from_file ( const std::string & name,
                                           const boost::filesystem::path & file_path )
{
  CJournal::Ptr journal( new CJournal(name) );

  journal->load_journal_file(file_path);

  return journal;
}

////////////////////////////////////////////////////////////////////////////////

void CJournal::load_journal_file ( const boost::filesystem::path & file_path )
{
  /// @todo handle m_info_node and m_signals_map

  m_signals.clear();
  m_xmldoc = XML::parse_file(file_path);
}

////////////////////////////////////////////////////////////////////////////////

void CJournal::dump_journal_to ( const boost::filesystem::path & file_path ) const
{
  XML::to_file( *m_xmldoc, file_path );

  CFinfo << "Journal dumped to '" << file_path.string() << "'" << CFendl;
}

////////////////////////////////////////////////////////////////////////////////

void CJournal::add_signal ( const Signal::arg_t & signal_node )
{
  rapidxml::xml_attribute<> * type_attr = signal_node.node.content->first_attribute("type");

  if(m_properties["RecordReplies"].value<bool>() ||
     (type_attr != nullptr && std::strcmp(type_attr->value(), "signal") == 0) )
  {
    XmlNode copy = copy_node(signal_node.node, m_signals_map.content);

    boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
    copy.set_attribute("time", boost::posix_time::to_simple_string(now));
  }
}

////////////////////////////////////////////////////////////////////////////////

void CJournal::execute_signals (const boost::filesystem::path & filename)
{


  if (m_root.expired())
    throw IllegalCall(FromHere(), "Component \'" + name() + "\' has no root");

  boost::shared_ptr<XmlDoc> xmldoc = XML::parse_file(filename);
  XmlNode doc_node = Protocol::goto_doc_node(*xmldoc.get());
//  rapidxml::xml_node * signals_map = doc_node.content->first_node();
  bool found_map = false;
  rapidxml::xml_node<>* node = nullptr;
//  rapidxml::xml_attribute<>* key_attr = nullptr;
  CRoot::Ptr root = Core::instance().root();
  const char * frame_tag = Protocol::Tags::node_frame();

  XmlNode signal_map = Map(doc_node).seek_value( Protocol::Tags::key_signals() );

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

      if(receiver == "//Root/Core") // server specific component
        continue;

      try
      {
        SignalFrame sf(node);
        root->retrieve_component(receiver)->call_signal(target, sf);
      }
      catch(Exception & e)
      {
        CFerror << e.what() << CFendl;
      }

    }
  }

}

////////////////////////////////////////////////////////////////////////////////

void CJournal::list_journal ( Signal::arg_t & args )
{
  SignalFrame reply = args.create_reply( full_path() );

  copy_node(m_signals_map.content, reply.main_map.content);
}

////////////////////////////////////////////////////////////////////////////////

void CJournal::load_journal ( Signal::arg_t & args )
{
  throw NotImplemented(FromHere(), "CJournal::load_journal()");
}

////////////////////////////////////////////////////////////////////////////////

void CJournal::save_journal ( Signal::arg_t & args )
{
  URI file_path("./server-journal.xml", URI::Scheme::FILE);
  boost::filesystem::path path(file_path.path());

  XML::to_file( *m_xmldoc, file_path.path() );

  CFinfo << "Journal dumped to '" << path.canonize().string() << "'" << CFendl;
}

////////////////////////////////////////////////////////////////////////////////

XmlNode CJournal::copy_node(const XmlNode & in, XmlNode & out) const
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

} // Common
} // CF

