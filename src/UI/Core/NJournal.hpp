// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_GUI_Core_NJournal_hpp
#define cf3_GUI_Core_NJournal_hpp

/////////////////////////////////////////////////////////////////////////////

#include <QObject>

#include "UI/Core/CNode.hpp"

/////////////////////////////////////////////////////////////////////////////

class QString;

/////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace UI {
namespace Core {

/////////////////////////////////////////////////////////////////////////////

class NJournal : public QObject, public CNode
{
  Q_OBJECT

public: // typedefs

  typedef boost::shared_ptr<NJournal> Ptr;
  typedef boost::shared_ptr<NJournal const> ConstPtr;

public:

  NJournal(const std::string & name);

  /// @name SIGNALS
  //@{

  void list_journal(common::SignalArgs & node);

  //@} END SIGNALS

  /// @name VIRTUAL FUNCTIONS
  //@{

  /// @brief Gives the tool tip text
  /// @return Returns The class name
  virtual QString toolTip() const;

  //@} END VIRTUAL FUNCTIONS

signals:

  void journalRequest(bool local);

protected:

  /// Disables the local signals that need to.
  /// @param localSignals Map of local signals. All values are set to true
  /// by default.
  virtual void disableLocalSignals(QMap<QString, bool> & localSignals) const {}

}; // NJournal

/////////////////////////////////////////////////////////////////////////////

class JournalNotifier : public QObject
{
  Q_OBJECT

public:

  static JournalNotifier & instance();

  void regist(const NJournal* journal);

signals:

  void journalRequest(bool local);

private:

  JournalNotifier();

}; // JournalNotifier

/////////////////////////////////////////////////////////////////////////////

} // Core
} // UI
} // cf3

/////////////////////////////////////////////////////////////////////////////

#endif // CF3_GUI_Core_NJournal_hpp

