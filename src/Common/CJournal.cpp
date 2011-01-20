// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/date_time/posix_time/posix_time.hpp>
#include "Common/CBuilder.hpp"
#include "Common/Core.hpp"
#include "Common/Log.hpp"
#include "Common/LibCommon.hpp"
#include "Common/OptionT.hpp"

#include "Common/CJournal.hpp"


////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

///////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < CJournal, Component, LibCommon > CJournal_Builder;

////////////////////////////////////////////////////////////////////////////////

CJournal::CJournal (const std::string & name)
  : Component(name),
    m_xmldoc(XmlOps::create_doc())
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

  /// @todo change this when the XML layer arrives
  XmlNode & doc_node = *XmlOps::goto_doc_node(*m_xmldoc.get());

  m_info_node = XmlOps::add_node_to(doc_node, XmlParams::tag_node_map());
  m_signals_map = XmlOps::add_node_to(doc_node, XmlParams::tag_node_map());

  XmlOps::add_attribute_to(*m_signals_map, "key", "signals");

  XmlOps::add_attribute_to(*m_info_node, XmlParams::tag_attr_key(), "journalInfo");

  XmlParams::add_value_to(*m_info_node, "hostname", Core::instance().network_info().hostname());
  XmlParams::add_value_to(*m_info_node, "port", (Uint)Core::instance().network_info().port());
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
  m_xmldoc = XmlOps::parse(file_path);
}

////////////////////////////////////////////////////////////////////////////////

void CJournal::dump_journal_to ( const boost::filesystem::path & file_path ) const
{
  XmlOps::write_xml_node(*m_xmldoc.get(), file_path);

  CFinfo << "Journal dumped to '" << file_path.string() << "'" << CFendl;
}

////////////////////////////////////////////////////////////////////////////////

void CJournal::add_signal ( const XmlNode & signal_node )
{
  XmlAttr * type_attr = signal_node.first_attribute("type");

  if(m_properties["RecordReplies"].value<bool>() ||
     (type_attr != nullptr && std::strcmp(type_attr->value(), "signal") == 0) )
  {
    XmlNode * copy = copy_node(signal_node, *m_signals_map);

    boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
    XmlOps::add_attribute_to(*copy, "time", boost::posix_time::to_simple_string(now));
  }
}

////////////////////////////////////////////////////////////////////////////////

void CJournal::execute_signals (const boost::filesystem::path & filename)
{


  if (m_root.expired())
    throw IllegalCall(FromHere(), "Component \'" + name() + "\' has no root");

  boost::shared_ptr<XmlNode> xmldoc = XmlOps::parse(filename);
  XmlNode & doc_node = *XmlOps::goto_doc_node(*xmldoc.get());
  XmlNode * signals_map = doc_node.first_node();
  bool found_map = false;
  XmlNode * node = nullptr;
  XmlAttr * key_attr = nullptr;
  CRoot::Ptr root = Core::instance().root();

  for( ; signals_map != nullptr ; signals_map = signals_map->next_sibling())
  {
    key_attr = signals_map->first_attribute("key");
    found_map = key_attr != nullptr && std::strcmp(key_attr->value(), "signals") == 0;

    if(found_map)
      break;
  }

  if(!found_map)
    throw XmlError(FromHere(), "Could not find \'signals\' map.");

  node = signals_map->first_node("frame");

  for( ; node != nullptr ; node = node->next_sibling("frame"))
  {
    XmlAttr * type_attr = node->first_attribute("type");

    if(type_attr != nullptr && std::strcmp(type_attr->value(), "signal") == 0)
    {
      XmlAttr * target_attr = node->first_attribute("target");
      XmlAttr * receiver_attr = node->first_attribute("receiver");

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
        CFinfo << "Executing: '" << target << "'\t on '" << receiver << "'." << CFendl;
        root->access_component(receiver)->call_signal(target, *node);
      }
      catch(Exception & e)
      {
        CFerror << e.what();
      }

    }
  }

}

////////////////////////////////////////////////////////////////////////////////

void CJournal::list_journal ( XmlNode & node )
{
  XmlNode & reply_node = *XmlOps::add_reply_frame(node);

  XmlOps::add_attribute_to( reply_node, "sender", full_path().string_without_scheme() );

  copy_node(*m_signals_map, reply_node);
}

////////////////////////////////////////////////////////////////////////////////

void CJournal::load_journal ( XmlNode & node )
{
  throw NotImplemented(FromHere(), "CJournal::load_journal()");
}

////////////////////////////////////////////////////////////////////////////////

void CJournal::save_journal ( XmlNode & node )
{
  URI file_path("./server-journal.xml", URI::Scheme::FILE);
  boost::filesystem::path path(file_path.string_without_scheme());

  XmlOps::write_xml_node(*m_xmldoc.get(), path);

  CFinfo << "Journal dumped to '" << path.canonize().string() << "'" << CFendl;
}

////////////////////////////////////////////////////////////////////////////////

XmlNode * CJournal::copy_node(const XmlNode & in, XmlNode & out) const
{
  XmlNode * copy = XmlOps::add_node_to(out, in.name(), in.value());
  XmlAttr * attr = in.first_attribute();
  XmlNode * node = in.first_node();

  while( attr != nullptr )
  {
    XmlOps::add_attribute_to(*copy, attr->name(), attr->value());
    attr = attr->next_attribute();
  }


  while( node != nullptr )
  {
    copy_node(*node, *copy);

    node = node->next_sibling();
  }

  return copy;
}


////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

