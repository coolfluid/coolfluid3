// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
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
#include <QListView>
#include <QMessageBox>

#include <QDebug>

#include <cstdlib>      // for abs()

#include "UI/Core/NRemoteFSBrowser.hpp"
#include "UI/Core/ThreadManager.hpp"
#include "UI/Core/TreeThread.hpp"
#include "UI/Core/NLog.hpp"

#include "UI/Graphics/FileFilter.hpp"

#include "UI/Graphics/BrowserDialog.hpp"

using namespace CF::UI::Core;
using namespace CF::UI::UICommon;

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {
namespace Graphics {

////////////////////////////////////////////////////////////////////////////

BrowserDialog::BrowserDialog(QWidget *parent) :
  QDialog(parent),
  m_updatingCompleter(false)
{
  m_model = NRemoteFSBrowser::Ptr(new NRemoteFSBrowser("MyBrowser"));
  m_filterModel = new FileFilter(m_model.get(), this);
  m_view = new QListView( this );
  m_favoritesView = new QListView( this );
  m_labPath = new QLabel("Path:");
  m_labFilter = new QLabel("Filter:");
  m_editFilter = new QLineEdit();
  m_editPath = new QLineEdit("/Users/qt/workspace/coolfluid3/Builds/Dev/src/UI/");
  m_comboFilter = new QComboBox();
  m_btAddFav = new QPushButton("Add");
  m_btRemoveFav = new QPushButton("Remove");
  m_buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  m_completer = new QCompleter(m_model->completionModel(), this);

  m_pathLayout = new QHBoxLayout();
  m_favButtonsLayout = new QHBoxLayout();
  m_filterLayout = new QHBoxLayout();

  m_mainLayout = new QGridLayout(this);

  m_view->setModel( m_filterModel );
  m_editPath->setCompleter( m_completer );

  m_filterModel->setDynamicSortFilter(true);

  m_comboFilter->addItems( QStringList() << "Exact Match" << "Wildcards" << "Regular Expression");

  m_mainLayout->setSpacing(5);

//  m_view->setSortingEnabled(true);
//  m_view->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
//  m_view->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
  m_view->setAlternatingRowColors(true);
//  m_view->setSelectionBehavior(QAbstractItemView::SelectRows);
//  m_view->horizontalHeader()->setStretchLastSection(true);

  m_labPath->setBuddy(m_editPath);
  m_labFilter->setBuddy(m_editFilter);

  m_pathLayout->addWidget(m_labPath);
  m_pathLayout->addWidget(m_editPath);

  m_favButtonsLayout->addWidget(m_btAddFav);
  m_favButtonsLayout->addWidget(m_btRemoveFav);

  m_filterLayout->addWidget(m_labFilter);
  m_filterLayout->addWidget(m_editFilter);
  m_filterLayout->addWidget(m_comboFilter);

  m_mainLayout->addLayout(m_pathLayout,       0, 0, 1, -1); // span: 1 row, all cols
  m_mainLayout->addWidget(m_favoritesView,    1, 0);
  m_mainLayout->addWidget(m_view,             1, 1, 2, -1); // span: 2 rows, all cols
  m_mainLayout->addLayout(m_favButtonsLayout, 2, 0);
  m_mainLayout->addLayout(m_filterLayout,     3, 0, 1, 2);  // span: 1 row, 2 cols
  m_mainLayout->addWidget(m_buttons,          3, 3);

  m_mainLayout->setColumnStretch(0, 1);
  m_mainLayout->setColumnStretch(1, 2);
  m_mainLayout->setColumnStretch(2, 0);
  m_mainLayout->setColumnStretch(3, 2);

  ThreadManager::instance().tree().root()->addNode(m_model);

  connect(m_comboFilter, SIGNAL(currentIndexChanged(int)),
          this, SLOT(filterTypeChanged(int)));

  connect(m_view, SIGNAL(doubleClicked(QModelIndex)),
          this, SLOT(doubleClicked(QModelIndex)));

  connect(m_model.get(), SIGNAL(currentPathChanged(QString)),
          this, SLOT(currentPathChanged(QString)));

  connect(m_completer, SIGNAL(activated(QString)),
          this, SLOT(completerActivated(QString)));

  connect(m_editPath, SIGNAL(textEdited(QString)), this, SLOT(pathEdited(QString)) );

  connect(m_buttons, SIGNAL(accepted()), this, SLOT(accept()));

  connect(m_buttons, SIGNAL(rejected()), this, SLOT(reject()));

//  this->resize(QSize(this->width() /** 1.25*/, this->height() /** 1.25*/) );
  m_view->adjustSize();
  this->adjustSize();
  m_model->openDir("");

}

////////////////////////////////////////////////////////////////////////////

void BrowserDialog::filterTypeChanged(int index)
{

}

////////////////////////////////////////////////////////////////////////////

void BrowserDialog::doubleClicked(const QModelIndex &index)
{
  QModelIndex indexInModel = m_filterModel->mapToSource( index );

  if( m_model->isDirectory( indexInModel ) )
  {
    m_model->openDir( m_model->retrieveFullPath( indexInModel ) );
  }
}

////////////////////////////////////////////////////////////////////////////

void BrowserDialog::currentPathChanged( const QString & path )
{
  if( !m_updatingCompleter )
  {
    m_editPath->setText( path );

    if(m_editPath->hasFocus())
      m_completer->popup()->show();
  }
  else
    m_updatingCompleter = false;
}

////////////////////////////////////////////////////////////////////////////

void BrowserDialog::completerActivated( const QString & text )
{
  m_model->openDir( text );
}

////////////////////////////////////////////////////////////////////////////

void BrowserDialog::pathEdited( const QString & text )
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
  else if(m_oldPath.endsWith("/") || std::abs(m_oldPath.length() - text.length()) > 1)
  {
    m_updatingCompleter = true;
    send = true;
    path = QFileInfo(text).path();
  }

  if(send)
    m_model->openDir(path);

  m_oldPath = text;
}

////////////////////////////////////////////////////////////////////////////

void BrowserDialog::keyPressEvent(QKeyEvent * event)
{
  // key code for the pressed key
  int pressedKey = event->key();
  // modifiers keys pressed (ctrl, shift, alt, etc...)
  Qt::KeyboardModifiers modifiers = event->modifiers();

  QDialog::keyPressEvent(event);

  // if the path line edit has the focus
  if(m_editPath->hasFocus())
  {
    // Qt::Key_Enter : enter key located on the keypad
    // Qt::Key_Return : return key
    if(pressedKey == Qt::Key_Enter || pressedKey == Qt::Key_Return)
      m_editPath->setText(m_editPath->text());

    return;
  }

  // if user pressed Enter key, it is similar to clicking on a button
  // (if any has the focus). Note: if none has the focus the default one ("Ok") is
  // taken (this is managed by QDialogButtonBox class)
  // Qt::Key_Enter : enter key located on the keypad
  // Qt::Key_Return : return key
  if(pressedKey == Qt::Key_Enter || pressedKey == Qt::Key_Return)
  {
//    if(m_buttons->button(QDialogButtonBox::Ok)->hasFocus())
//      this->btOkClicked();

//    else if(m_buttons->button(QDialogButtonBox::Cancel)->hasFocus())
//      this->btCancelClicked();
  }

  else if(pressedKey == Qt::Key_Backspace)
    m_model->openDir(m_model->currentPath() + ".."); // back to the parent directory

  // if user pressed either no modifier key or Shift key *and* another key,
  // the filter line edit takes the focus
  else if(modifiers == Qt::NoModifier ||
          (modifiers == Qt::ShiftModifier && !event->text().isEmpty()))
  {
    m_view->clearFocus();

    m_editFilter->setText(m_editFilter->text() + event->text());
    pathEdited(m_editFilter->text());
    m_editFilter->setFocus(Qt::NoFocusReason);
  }
}

////////////////////////////////////////////////////////////////////////////

bool BrowserDialog::focusNextPrevChild(bool next)
{
  if(m_editPath->hasFocus() && m_completer->popup()->isVisible())
  {
    m_completer->setCurrentRow(0);
    m_editPath->setText(m_completer->currentCompletion());
    pathEdited(m_completer->currentCompletion());
    m_completer->popup()->setCurrentIndex(m_completer->currentIndex());
    return true;
  }

  else
    return QDialog::focusNextPrevChild(next);
}

//////////////////////////////////////////////////////////////////////////////

bool BrowserDialog::show( bool multiSelect, QVariant & selected )
{
  if( multiSelect )
    m_view->setSelectionMode(QAbstractItemView::ExtendedSelection); // allow multi-selection
  else
    m_view->setSelectionMode(QAbstractItemView::SingleSelection);   // mono-selection

  connect(NLog::globalLog().get(), SIGNAL(newMessage(QString, UICommon::LogMessage::Type)),
          this, SLOT(message(QString, UICommon::LogMessage::Type)));

  bool okClicked = exec() == Accepted;
  QString path = m_model->currentPath();

  // if user clicked on "OK"
  if(okClicked)
  {
    QModelIndexList selectedIndexes = m_view->selectionModel()->selectedIndexes();

    if( !multiSelect && selectedIndexes.count() == 1 )
    {
      QModelIndex indexInModel = m_filterModel->mapToSource( selectedIndexes.at(0) );
      selected = path + m_model->data( indexInModel, Qt::DisplayRole ).toString();
    }
    else if( multiSelect )
    {
      QModelIndexList::iterator it = selectedIndexes.begin();
      QStringList list;

      for( ; it != selectedIndexes.end() ; ++it )
      {
        QModelIndex indexInModel = m_filterModel->mapToSource( *it );

        list << path + m_model->data( indexInModel, Qt::DisplayRole ).toString();
      }

      selected = list;
    }
  }

  disconnect(NLog::globalLog().get());

  return okClicked;
}

//////////////////////////////////////////////////////////////////////////////

void BrowserDialog::message( const QString& message , LogMessage::Type type)
{
  if ( type == LogMessage::INFO )
    QMessageBox::information(this, "Info", message);
  else if ( type == LogMessage::WARNING )
    QMessageBox::warning(this, "Warning", message);
  else
    QMessageBox::critical(this, "Error", message);
}

//////////////////////////////////////////////////////////////////////////////

} // Graphics
} // UI
} // CF
