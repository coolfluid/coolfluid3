// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_GUI_Graphics_JournalBrowserDialog_hpp
#define cf3_GUI_Graphics_JournalBrowserDialog_hpp

////////////////////////////////////////////////////////////////////////////

#include <QDialog>

#include "UI/Core/NJournalBrowser.hpp"

class QAbstractButton;
class QDialogButtonBox;
class QModelIndex;
class QTableView;
class QVBoxLayout;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {

namespace common {
  namespace XML { class XmlNode; }
}

namespace UI {
namespace Graphics {

////////////////////////////////////////////////////////////////////////////

class JournalBrowserDialog;

class JournalBrowserBuilder : public QObject
{
  Q_OBJECT

public:

  static JournalBrowserBuilder & instance();

private slots:

  void journal_request(bool local);

private:

  JournalBrowserDialog * m_dialog;

  JournalBrowserBuilder();

}; // JournalBrowserBuilder

////////////////////////////////////////////////////////////////////////////

class JournalBrowserDialog : public QDialog
{
  Q_OBJECT

public:

  JournalBrowserDialog(QWidget * parent = 0);

  ~JournalBrowserDialog();

  void show(const common::XML::XmlNode * rootNode);

private slots:

  void double_clicked(const QModelIndex & index);

  void bt_clicked(QAbstractButton * button);

private:

  QTableView * m_view;

  QVBoxLayout * m_main_layout;

  QDialogButtonBox * m_buttons;

  QPushButton * m_bt_execute;

  Core::NJournalBrowser::Ptr m_model;

}; // JournalBrowserDialog

////////////////////////////////////////////////////////////////////////////

} // Graphics
} // UI
} // cf3

////////////////////////////////////////////////////////////////////////////

#endif // cf3_GUI_Graphics_JournalBrowserDialog_hpp
