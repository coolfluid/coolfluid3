// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Client_UI_JournalBrowserDialog_hpp
#define CF_GUI_Client_UI_JournalBrowserDialog_hpp

////////////////////////////////////////////////////////////////////////////

#include <QDialog>

#include "Common/XML.hpp"

#include "GUI/Client/Core/NJournal.hpp"

class QAbstractButton;
class QDialogButtonBox;
class QModelIndex;
class QTableView;
class QVBoxLayout;

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace ClientUI {

////////////////////////////////////////////////////////////////////////////

class JournalBrowserBuilder : public QObject
{
  Q_OBJECT

public:

  static JournalBrowserBuilder & instance();

private slots:

  void newJournal(/*CF::GUI::ClientCore::NJournal * journal, */Common::XmlNode * node);

private:

  JournalBrowserBuilder();

}; // JournalBrowserBuilder

////////////////////////////////////////////////////////////////////////////

class JournalBrowserDialog : public QDialog
{
  Q_OBJECT

public:

  JournalBrowserDialog(QWidget * parent = 0);

  ~JournalBrowserDialog();

  void show(const Common::XmlNode * rootNode);

private slots:

  void doubleClicked(const QModelIndex & index);

  void btClicked(QAbstractButton * button);

private:

  QTableView * m_view;

  QVBoxLayout * m_mainLayout;

  QDialogButtonBox * m_buttons;

  QPushButton * m_btExecute;

}; // JournalBrowserDialog

////////////////////////////////////////////////////////////////////////////

} // ClientUI
} // GUI
} // CF

////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_UI_JournalBrowserDialog_hpp
