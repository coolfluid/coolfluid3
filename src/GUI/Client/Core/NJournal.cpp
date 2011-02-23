// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "GUI/Network/ComponentNames.hpp"

#include "GUI/Client/Core/ClientRoot.hpp"

#include "GUI/Client/Core/NJournal.hpp"

/////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Common::XML;

/////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace ClientCore {

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

NJournal::NJournal(const QString & name)
  : CNode(name, "CJournal", JOURNAL_NODE)
{
  regist_signal("list_journal", "List journal", "List journal")
      ->connect(boost::bind(&NJournal::list_journal, this, _1));

  m_localSignals << "list_journal";

  JournalNotifier::instance().regist(this);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString NJournal::toolTip() const
{
  return getComponentType();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NJournal::list_journal(Signal::arg_t &)
{
  emit journalRequest(false);
}

/////////////////////////////////////////////////////////////////////////////

} // ClientCore
} // GUI
} // CF
