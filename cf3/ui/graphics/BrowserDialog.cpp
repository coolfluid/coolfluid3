// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QComboBox>
#include <QCompleter>
#include <QDialogButtonBox>
#include <QFileInfo>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QPushButton>
#include <QMessageBox>

#include "ui/core/NBrowser.hpp"
#include "ui/core/NRemoteFSBrowser.hpp"
#include "ui/core/ThreadManager.hpp"
#include "ui/core/TreeThread.hpp"
#include "ui/core/NLog.hpp"

#include "ui/graphics/FavoritesModel.hpp"
#include "ui/graphics/FileFilter.hpp"

#include "ui/graphics/BrowserDialog.hpp"

using namespace cf3::ui::core;
using namespace cf3::ui::uiCommon;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace graphics {

////////////////////////////////////////////////////////////////////////////

BrowserDialog::BrowserDialog(QWidget *parent) :
  QDialog(parent),
  m_updating_completer(false)
{
  m_model = NRemoteFSBrowser::Ptr(new NRemoteFSBrowser( NBrowser::global()->generate_name() ));
  m_favorites_model = new FavoritesModel();

  init_gui();

  connect( m_model.get(), SIGNAL(favorites_changed(QStringList)),
           m_favorites_model, SLOT(set_string_list(QStringList)) );

  m_model->update_favorite_list();
  m_model->open_dir("");
}

////////////////////////////////////////////////////////////////////////////

void BrowserDialog::filter_edited( const QString & new_text )
{
  QRegExp regex(new_text, Qt::CaseInsensitive, QRegExp::Wildcard);
   m_filter_model->setFilterRegExp(regex);
}

////////////////////////////////////////////////////////////////////////////

void BrowserDialog::double_clicked(const QModelIndex &index)
{
  QModelIndex index_in_model = m_filter_model->mapToSource( index );

  if( m_model->is_directory( index_in_model ) )
  {
    m_model->open_dir( m_model->retrieve_full_path( index_in_model ) );
  }
}

////////////////////////////////////////////////////////////////////////////

void BrowserDialog::current_path_changed( const QString & path )
{
  if( !m_updating_completer )
    m_edit_path->setText( path );
  else
    m_updating_completer = false;
}

////////////////////////////////////////////////////////////////////////////

void BrowserDialog::completer_activated( const QString & text )
{
  m_model->open_dir( text );
}

////////////////////////////////////////////////////////////////////////////

void BrowserDialog::path_edited( const QString & text )
{
  QString path;
  bool send = false;

  // if user just typed a '/', the path to explore is the current path in the field
  if(text.endsWith("/"))
  {
    send = true;
    path = text;
  }

  // if user just deleted a '/' or lengths of texts differ of more than one
  // character (this may happen if user pasted a path or delete more than
  // one character at a time), the path to explore is the parent directory of
  // the path in the field
  else if(m_old_path.endsWith("/") || qAbs(m_old_path.length() - text.length()) > 1)
  {
    m_updating_completer = true;
    send = true;
    path = QFileInfo(text).path();
  }

  if(send)
    m_model->open_dir(path);

  m_old_path = text;
}

////////////////////////////////////////////////////////////////////////////

void BrowserDialog::keyPressEvent(QKeyEvent * event)
{
  // key code for the pressed key
  int pressed_key = event->key();
  // modifiers keys pressed (ctrl, shift, alt, etc...)
  Qt::KeyboardModifiers modifiers = event->modifiers();

  QDialog::keyPressEvent(event);

  // if the path line edit has the focus
  if(m_edit_path->hasFocus())
  {
    // Qt::Key_Enter : enter key located on the keypad
    // Qt::Key_Return : return key
    if(pressed_key == Qt::Key_Enter || pressed_key == Qt::Key_Return)
      m_edit_path->setText(m_edit_path->text());

    return;
  }

  // if user pressed Enter key, it is similar to clicking on a button
  // (if any has the focus). Note: if none has the focus the default one ("Ok") is
  // taken (this is managed by QDialogButtonBox class)
  // Qt::Key_Enter : enter key located on the keypad
  // Qt::Key_Return : return key
  if(pressed_key == Qt::Key_Enter || pressed_key == Qt::Key_Return)
  {
    if(m_buttons->button(QDialogButtonBox::Ok)->hasFocus())
      accept();
    else if(m_buttons->button(QDialogButtonBox::Cancel)->hasFocus())
      reject();
  }

  else if(pressed_key == Qt::Key_Backspace)
    m_model->open_dir(m_model->current_path() + ".."); // back to the parent directory

  // if user pressed either no modifier key or Shift key *and* another key,
  // the filter line edit takes the focus
  else if(modifiers == Qt::NoModifier ||
          (modifiers == Qt::ShiftModifier && !event->text().isEmpty()))
  {
    m_view->clearFocus();

    m_edit_filter->setText(m_edit_filter->text() + event->text());
    path_edited(m_edit_filter->text());
    m_edit_filter->setFocus(Qt::NoFocusReason);
  }
}

////////////////////////////////////////////////////////////////////////////

bool BrowserDialog::focusNextPrevChild(bool next)
{
  if(m_edit_path->hasFocus() && m_completer->popup()->isVisible())
  {
    m_completer->setCurrentRow(0);
    m_edit_path->setText(m_completer->currentCompletion());
    path_edited(m_completer->currentCompletion());
    m_completer->popup()->setCurrentIndex(m_completer->currentIndex());
    return true;
  }

  else
    return QDialog::focusNextPrevChild(next);
}

//////////////////////////////////////////////////////////////////////////////

bool BrowserDialog::show( bool multi_select, QVariant & selected )
{
  if( multi_select )
    m_view->setSelectionMode(QAbstractItemView::ExtendedSelection); // allow multi-selection
  else
    m_view->setSelectionMode(QAbstractItemView::SingleSelection);   // mono-selection

  connect(NLog::global().get(), SIGNAL(new_message(QString, uiCommon::LogMessage::Type)),
          this, SLOT(message(QString, uiCommon::LogMessage::Type)));

  bool ok_clicked = exec() == Accepted;
  QString path = m_model->current_path();

  // if user clicked on "OK"
  if(ok_clicked)
  {
    QModelIndexList selected_indexes = m_view->selectionModel()->selectedIndexes();

    if( !multi_select && selected_indexes.count() == 1 )
    {
      QModelIndex index_in_model = m_filter_model->mapToSource( selected_indexes.at(0) );
      selected = path + m_model->data( index_in_model, Qt::DisplayRole ).toString();
    }
    else if( multi_select )
    {
      QModelIndexList::iterator it = selected_indexes.begin();
      QStringList list;

      for( ; it != selected_indexes.end() ; ++it )
      {
        QModelIndex index_in_model = m_filter_model->mapToSource( *it );

        list << path + m_model->data( index_in_model, Qt::DisplayRole ).toString();
      }

      selected = list;
    }
  }

  disconnect(NLog::global().get());

  return ok_clicked;
}

//////////////////////////////////////////////////////////////////////////////

void BrowserDialog::message( const QString& message , LogMessage::Type type )
{
  if ( type == LogMessage::INFO )
    QMessageBox::information(this, "Info", message);
  else if ( type == LogMessage::WARNING )
    QMessageBox::warning(this, "Warning", message);
  else
    QMessageBox::critical(this, "Error", message);
}

//////////////////////////////////////////////////////////////////////////////

void BrowserDialog::add_favorite()
{
  QString path = m_model->current_path();


  path.remove(path.length() - 1, 1);

  if( !m_favorites_model->add_string(path) )
    QMessageBox::critical(this, "Error", "This favorite place already exists!");
  else
    m_model->send_favorites(m_favorites_model->string_list());
}

//////////////////////////////////////////////////////////////////////////////

void BrowserDialog::remove_favorite()
{
  QModelIndexList list = m_favorites_view->selectionModel()->selectedIndexes();
  if( !list.isEmpty() )
  {
    if( !m_favorites_model->remove_string(list.at(0).row()) )
      QMessageBox::critical(this, "Error", "You cannot remove this favorite place!");
    else
      m_model->send_favorites(m_favorites_model->string_list());
  }
}

//////////////////////////////////////////////////////////////////////////////

void BrowserDialog::favorite_selected( const QModelIndex & index)
{
  if( index.isValid() )
  {
    QString path = m_favorites_model->full_path(index);

    if( !m_favorites_model->is_special_dir(index) )
      m_model->open_dir(path);
    else
      m_model->open_special_dir(path);
  }
}

//////////////////////////////////////////////////////////////////////////////

void BrowserDialog::init_gui()
{
  // initiate graphical components
  m_filter_model = new FileFilter(m_model.get(), this);
  m_view = new QListView( this );
  m_favorites_view = new QListView( this );
  m_lab_path = new QLabel("Path:");
  m_lab_filter = new QLabel("Filter (wildcards allowed):");
  m_edit_filter = new QLineEdit();
  m_edit_path = new QLineEdit();
  m_bt_add_fav = new QPushButton("Add");
  m_bt_remove_fav = new QPushButton("Remove");
  m_buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  m_completer = new QCompleter(m_model->completion_model(), this);

  // initiate layouts
  m_path_layout = new QHBoxLayout();
  m_fav_buttons_layout = new QHBoxLayout();
  m_filter_layout = new QHBoxLayout();

  m_main_layout = new QGridLayout(this);

  // configure the graphical components
  m_view->setModel( m_filter_model );
  m_favorites_view->setModel( m_favorites_model );
  m_edit_path->setCompleter( m_completer );
  m_completer->setMaxVisibleItems(7);

  m_filter_model->setDynamicSortFilter(true);

  m_main_layout->setSpacing(5);

  m_view->setAlternatingRowColors(true);

  m_lab_path->setBuddy(m_edit_path);
  m_lab_filter->setBuddy(m_edit_filter);

  // add components to their layout
  m_path_layout->addWidget(m_lab_path);
  m_path_layout->addWidget(m_edit_path);

  m_fav_buttons_layout->addWidget(m_bt_add_fav);
  m_fav_buttons_layout->addWidget(m_bt_remove_fav);

  m_filter_layout->addWidget(m_lab_filter);
  m_filter_layout->addWidget(m_edit_filter);

  m_main_layout->addLayout(m_path_layout,        0, 0, 1, -1); // span: 1 row, all cols
  m_main_layout->addWidget(m_favorites_view,     1, 0);
  m_main_layout->addWidget(m_view,               1, 1, 2, -1); // span: 2 rows, all cols
  m_main_layout->addLayout(m_fav_buttons_layout, 2, 0);
  m_main_layout->addLayout(m_filter_layout,      3, 0, 1, 2);  // span: 1 row, 2 cols
  m_main_layout->addWidget(m_buttons,            3, 3);

  m_main_layout->setColumnStretch(0, 1);
  m_main_layout->setColumnStretch(1, 2);
  m_main_layout->setColumnStretch(2, 0);
  m_main_layout->setColumnStretch(3, 2);

  ThreadManager::instance().tree().root()->add_node(m_model);

  // connect useful signals
  connect(m_view, SIGNAL(doubleClicked(QModelIndex)),
          this, SLOT(double_clicked(QModelIndex)));

  connect(m_model.get(), SIGNAL(current_path_changed(QString)),
          this, SLOT(current_path_changed(QString)));

  connect(m_completer, SIGNAL(activated(QString)),
          this, SLOT(completer_activated(QString)));

  connect(m_edit_filter, SIGNAL(textEdited(QString)),
          this, SLOT(filter_edited(QString)));

  connect(m_edit_path, SIGNAL(textEdited(QString)), this, SLOT(path_edited(QString)) );

  connect(m_buttons, SIGNAL(accepted()), this, SLOT(accept()));

  connect(m_buttons, SIGNAL(rejected()), this, SLOT(reject()));

  connect(m_bt_add_fav, SIGNAL(clicked()), this, SLOT(add_favorite()));

  connect(m_bt_remove_fav, SIGNAL(clicked()), this, SLOT(remove_favorite()));

  connect(m_favorites_view, SIGNAL(pressed(QModelIndex)),
          this, SLOT(favorite_selected(QModelIndex)));

  m_view->adjustSize();
  this->adjustSize();
}

//////////////////////////////////////////////////////////////////////////////

} // Graphics
} // ui
} // cf3
