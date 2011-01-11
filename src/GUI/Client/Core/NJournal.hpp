// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Client_Core_NJournal_hpp
#define CF_GUI_Client_Core_NJournal_hpp

/////////////////////////////////////////////////////////////////////////////

#include <QObject>

#include "GUI/Client/Core/CNode.hpp"

/////////////////////////////////////////////////////////////////////////////

class QString;

/////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace ClientCore {

/////////////////////////////////////////////////////////////////////////////

class NJournal : public QObject, public CNode
{
  Q_OBJECT

public: // typedefs

  typedef boost::shared_ptr<NJournal> Ptr;
  typedef boost::shared_ptr<NJournal const> ConstPtr;

public:

  NJournal(const QString & name);

  /// @name SIGNALS
  //@{

  void list_journal(Common::XmlNode & node);

  //@} END SIGNALS

  /// @name VIRTUAL FUNCTIONS
  //@{

  /// @brief Gives the tool tip text
  /// @return Returns The class name
  virtual QString toolTip() const;

  //@} END VIRTUAL FUNCTIONS

signals:

  void newJournal(/*NJournal * journal, */Common::XmlNode * node);

}; // NJournal

/////////////////////////////////////////////////////////////////////////////

class JournalNotifier : public QObject
{
  Q_OBJECT

public:

  static JournalNotifier & instance();

  void regist(const NJournal* journal);

signals:

  void newJournal(/*const NJournal* journal, */Common::XmlNode * node);

private:

  JournalNotifier();

}; // JournalNotifier

/////////////////////////////////////////////////////////////////////////////

} // ClientCore
} // GUI
} // CF

/////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_Core_NJournal_hpp

