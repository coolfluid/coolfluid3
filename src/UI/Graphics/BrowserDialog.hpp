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
namespace UI {

namespace Core { class NRemoteFSBrowser; }

namespace Graphics {

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

  void filterTypeChanged( int index );

  void doubleClicked( const QModelIndex & index );

  void currentPathChanged( const QString & path );

  void completerActivated ( const QString & text );

  void pathEdited( const QString & text );

  void message( const QString& message , UICommon::LogMessage::Type);

private:

  boost::shared_ptr<Core::NRemoteFSBrowser> m_model;

  FileFilter * m_filterModel;

  QListView * m_view;

  QDialogButtonBox * m_buttons;

  QGridLayout * m_mainLayout;

  QListView * m_favoritesView;

  QHBoxLayout * m_pathLayout;

  QLabel * m_labPath;

  QLineEdit * m_editPath;

  QHBoxLayout * m_favButtonsLayout;

  QPushButton * m_btAddFav;

  QPushButton * m_btRemoveFav;

  QHBoxLayout * m_filterLayout;

  QLabel * m_labFilter;

  QLineEdit * m_editFilter;

  QComboBox * m_comboFilter;

  QCompleter * m_completer;

  QString m_oldPath;

  bool m_updatingCompleter;

}; // BrowserDialog

////////////////////////////////////////////////////////////////////////////

} // Graphics
} // UI
} // cf3

////////////////////////////////////////////////////////////////////////////

#endif // CF3_UI_Graphics_BrowserDialog_hpp
