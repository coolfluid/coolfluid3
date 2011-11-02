// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UI_Graphics_BrowserDialog_hpp
#define cf3_UI_Graphics_BrowserDialog_hpp

#include <QDialog>

#include "UI/UICommon/LogMessage.hpp"

#include "UI/Graphics/LibGraphics.hpp"

class QComboBox;
class QCompleter;
class QDialogButtonBox;
class QGridLayout;
class QHBoxLayout;
class QLabel;
class QLineEdit;
class QListView;
class QModelIndex;
class QPushButton;
class QSortFilterProxyModel;
class QListView;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {

namespace core { class NRemoteFSBrowser; }

namespace graphics {

class FileFilter;

////////////////////////////////////////////////////////////////////////////

class Graphics_API BrowserDialog : public QDialog
{
  Q_OBJECT

public:

  BrowserDialog ( QWidget *parent = 0 );

  bool show( bool multiSelect, QVariant & selected );

protected:

  virtual void keyPressEvent( QKeyEvent * event );

  virtual bool focusNextPrevChild( bool next );

private slots:

  void filter_type_changed( int index );

  void double_clicked( const QModelIndex & index );

  void current_path_changed( const QString & path );

  void completer_activated ( const QString & text );

  void path_edited( const QString & text );

  void message( const QString& message , UICommon::LogMessage::Type);

private:

  boost::shared_ptr<core::NRemoteFSBrowser> m_model;

  FileFilter * m_filter_model;

  QListView * m_view;

  QDialogButtonBox * m_buttons;

  QGridLayout * m_main_layout;

  QListView * m_favorites_view;

  QHBoxLayout * m_path_layout;

  QLabel * m_lab_path;

  QLineEdit * m_edit_path;

  QHBoxLayout * m_fav_buttons_layout;

  QPushButton * m_bt_add_fav;

  QPushButton * m_bt_remove_fav;

  QHBoxLayout * m_filter_layout;

  QLabel * m_lab_filter;

  QLineEdit * m_edit_filter;

  QComboBox * m_combo_filter;

  QCompleter * m_completer;

  QString m_old_path;

  bool m_updating_completer;

}; // BrowserDialog

////////////////////////////////////////////////////////////////////////////

} // Graphics
} // UI
} // cf3

////////////////////////////////////////////////////////////////////////////

#endif // cf3_UI_Graphics_BrowserDialog_hpp
