// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#ifndef CF_UI_Graphics_BrowserDialog_hpp
#define CF_UI_Graphics_BrowserDialog_hpp

#include <QDialog>

#include "UI/Graphics/LibGraphics.hpp"

class QComboBox;
class QDialogButtonBox;
class QGridLayout;
class QHBoxLayout;
class QLabel;
class QLineEdit;
class QListView;
class QModelIndex;
class QPushButton;
class QSortFilterProxyModel;
class QTableView;

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {

namespace Core { class NRemoteFSBrowser; }

namespace Graphics {

////////////////////////////////////////////////////////////////////////////

class Graphics_API BrowserDialog : public QDialog
{
  Q_OBJECT

public:

  BrowserDialog ( QWidget *parent = 0 );

private slots:

  void filterTypeChanged( int index );

  void doubleClicked(const QModelIndex & index );

private:

  boost::shared_ptr<Core::NRemoteFSBrowser> m_model;

  QTableView * m_view;

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

  QSortFilterProxyModel * m_filteringModel;

}; // BrowserDialog

////////////////////////////////////////////////////////////////////////////

} // Graphics
} // UI
} // CF

////////////////////////////////////////////////////////////////////////////

#endif // CF_UI_Graphics_BrowserDialog_hpp
