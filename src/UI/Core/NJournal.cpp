// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Signal.hpp"

#include "UI/Core/NJournal.hpp"

/////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Common::XML;

/////////////////////////////////////////////////////////////////////////////

namespace CF {
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
  cf_assert(journal != nullptr);

  connect(journal, SIGNAL(journalRequest(bool)), this, SIGNAL(journalRequest(bool)));
}

/////////////////////////////////////////////////////////////////////////////

NJournal::NJournal(const std::string & name)
  : CNode(name, "CJournal", CNode::STANDARD_NODE)
{
  regist_signal( "list_journal" )
    ->connect( boost::bind( &NJournal::list_journal, this, _1 ) )
    ->description("List journal")
    ->pretty_name("List journal");

  m_localSignals << "list_journal";

  JournalNotifier::instance().regist(this);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString NJournal::toolTip() const
{
  return componentType();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NJournal::list_journal(SignalArgs &)
{
  emit journalRequest(false);
}

/////////////////////////////////////////////////////////////////////////////

} // Core
} // UI
} // CF
