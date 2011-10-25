// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Signal.hpp"

#include "UI/Core/NJournal.hpp"

/////////////////////////////////////////////////////////////////////////////

using namespace cf3::common;
using namespace cf3::common::XML;

/////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace UI {
namespace Core {

/////////////////////////////////////////////////////////////////////////////

JournalNotifier::JournalNotifier()
{

}

/////////////////////////////////////////////////////////////////////////////

JournalNotifier & JournalNotifier::instance()
{
  static JournalNotifier jn;
  return jn;
}

/////////////////////////////////////////////////////////////////////////////

void JournalNotifier::regist(const NJournal * journal)
{
  cf3_assert(journal != nullptr);

  connect(journal, SIGNAL(journal_request(bool)), this, SIGNAL(journal_request(bool)));
}

/////////////////////////////////////////////////////////////////////////////

NJournal::NJournal(const std::string & name)
  : CNode(name, "Journal", CNode::STANDARD_NODE)
{
  regist_signal( "list_journal" )
    ->connect( boost::bind( &NJournal::list_journal, this, _1 ) )
    ->description("List journal")
    ->pretty_name("List journal");

  m_local_signals << "list_journal";

  JournalNotifier::instance().regist(this);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString NJournal::tool_tip() const
{
  return component_type();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NJournal::list_journal(SignalArgs &)
{
  emit journal_request(false);
}

/////////////////////////////////////////////////////////////////////////////

} // Core
} // UI
} // cf3
