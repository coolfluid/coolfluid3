// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QApplication>
#include <QCompleter>
#include <QDialogButtonBox>
#include <QFileIconProvider>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QMainWindow>
#include <QMessageBox>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QVBoxLayout>

#include <stdexcept>
#include <cstdlib>      // for abs()

#include "common/OptionT.hpp"
#include "common/OptionArray.hpp"
#include "common/Signal.hpp"

#include "common/XML/Protocol.hpp"
#include "common/XML/SignalOptions.hpp"

#include "ui/core/NBrowser.hpp"
#include "ui/core/NetworkQueue.hpp"
#include "ui/core/NLog.hpp"
#include "ui/graphics/FilesListItem.hpp"

#include "ui/uicommon/ComponentNames.hpp"

#include "ui/graphics/NRemoteBrowser.hpp"

using namespace cf3::common;
using namespace cf3::common::XML;
using namespace cf3::ui::core;
using namespace cf3::ui::uiCommon;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace graphics {

////////////////////////////////////////////////////////////////////////////

NRemoteBrowser::NRemoteBrowser(const QString & componentType, QMainWindow * parent)
  : QDialog(parent),
    CNode(NBrowser::global()->generate_name().toStdString(), componentType, CNode::DEBUG_NODE)
{

  regist_signal( "read_dir" )
    .description("Directory content")
    .pretty_name("").connect(boost::bind(&NRemoteBrowser::read_dir, this, _1));

  this->setWindowTitle("Open file");

  // create the components
  m_lab_filter = new QLabel("Filter (wildcards allowed) :", this);
  m_lab_files_list = new QLabel("Files in", this);
  m_view_model = new QStandardItemModel();
  m_list_view = new QListView(this);
  m_edit_filter = new QLineEdit(this);
  m_edit_path = new QLineEdit(this);
  m_lab_status = new QLabel(this);
  m_filter_model = new QSortFilterProxyModel();
  m_completer_model = new QStandardItemModel();
  m_path_completer = new QCompleter(m_completer_model, this);

  m_parent_window = parent;

  m_layout = new QVBoxLayout(this);
  m_path_layout = new QHBoxLayout();
  m_bottom_layout = new QHBoxLayout();

  // create 2 buttons : "Ok" and "Cancel"
  m_buttons = new QDialogButtonBox(QDialogButtonBox::Ok
                                   | QDialogButtonBox::Cancel);

  m_ok_clicked = false;
  m_multiple_select_allowed = false;
  m_updating_completer = false;

  m_path_sep = "/";

  this->setModal(true);

  m_filter_model->setDynamicSortFilter(true);

  m_filter_model->setSourceModel(m_view_model);
  m_list_view->setModel(m_filter_model);

  m_list_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_list_view->setAlternatingRowColors(true);

  m_edit_path->setCompleter(m_path_completer);

  m_lab_filter->setBuddy(m_edit_filter);
  m_lab_files_list->setBuddy(m_edit_path);

  // add the components to the layouts
  m_path_layout->addWidget(m_lab_files_list);
  m_path_layout->addWidget(m_edit_path);

  m_bottom_layout->addWidget(m_lab_filter);
  m_bottom_layout->addWidget(m_edit_filter);
  m_bottom_layout->addWidget(m_buttons);

  m_layout->addLayout(m_path_layout);
  m_layout->addWidget(m_list_view);
  m_layout->addWidget(m_lab_status);
  m_layout->addLayout(m_bottom_layout);

  // set "Ok" button as default when user presses enter
  m_buttons->button(QDialogButtonBox::Ok)->setDefault(true);
  m_buttons->button(QDialogButtonBox::Cancel)->setAutoDefault(false);

  // connect useful signals to slots
  connect(m_buttons, SIGNAL(accepted()), this, SLOT(bt_ok_clicked()));
  connect(m_buttons, SIGNAL(rejected()), this, SLOT(bt_cancel_clicked()));

  connect(m_edit_filter, SIGNAL(textEdited(QString)),
          this, SLOT(filter_updated(QString)));

  connect(m_edit_path, SIGNAL(textEdited(QString)),
          this, SLOT(path_updated(QString)));

  connect(m_list_view, SIGNAL(doubleClicked(QModelIndex)),
          this, SLOT(double_click(QModelIndex)));

  connect(m_path_completer, SIGNAL(activated(QString)),
          this, SLOT(completer_activated(QString)));

  m_includeFiles = true;
  m_include_no_extension = true;
  this->allow_modify_bools = true;
  this->allow_single_select = true;
  this->allow_multiple_select = true;

  this->resize(this->height() * 2, this->height());
}

////////////////////////////////////////////////////////////////////////////

NRemoteBrowser::~NRemoteBrowser()
{
  delete m_buttons;
  delete m_edit_filter;
  delete m_filter_model;
  delete m_lab_filter;
  delete m_lab_files_list;
  delete m_lab_status;
  delete m_path_layout;
  delete m_bottom_layout;
  delete m_layout;
  delete m_list_view;
  delete m_edit_path;
  delete m_path_completer;
  delete m_view_model;
  delete m_completer_model;

  // disconnecting the signals connected by the constructor
  // (normally, this is automatically done when the object is destroyed,
  // but the documentation is not clear on this point)
  disconnect(this);
}

////////////////////////////////////////////////////////////////////////////

QString NRemoteBrowser::show(const QString & startingDir, bool * canceled)
{
  if(!this->allow_single_select)
  {
    this->show_error("This dialog can not be used to select a single file");
    return QString();
  }

  if(startingDir.isEmpty())
    this->open_dir(m_current_path);

  else
    this->open_dir(startingDir);

  m_multiple_select_allowed = false;
  m_current_file = "";
  m_current_files_list.clear();

  m_list_view->setSelectionMode(QAbstractItemView::SingleSelection);
  m_list_view->clearSelection();

  connect(NLog::global().get(), SIGNAL(new_message(QString, uiCommon::LogMessage::Type)),
          this, SLOT(message(QString, uiCommon::LogMessage::Type)));

  this->reinit_values();

  this->exec();

  this->disconnect(NLog::global().get());

  if(canceled != nullptr)
    *canceled = !m_ok_clicked;

  if(m_ok_clicked)
    return this->selected_file();

  // restore mouse cursor
  QApplication::restoreOverrideCursor();

  return QString();
}

////////////////////////////////////////////////////////////////////////////

QStringList NRemoteBrowser::show_multiple_select(const QString & startingDir)
{
  QStringList list;

  if(!this->allow_multiple_select)
  {
    this->show_error("This dialog can not be used to select multiple files");
    return QStringList();
  }

  if(startingDir.isEmpty())
    this->open_dir(m_current_path);

  else
    this->open_dir(startingDir);

  m_multiple_select_allowed = true;
  m_current_file = "";
  m_current_files_list.clear();

  m_list_view->setSelectionMode(QAbstractItemView::ExtendedSelection);
  m_list_view->clearSelection();

  connect(NLog::global().get(), SIGNAL(new_message(QString, uiCommon::LogMessage::Type)),
          this, SLOT(message(QString, uiCommon::LogMessage::Type)));

  this->reinit_values();

  this->exec();

  this->disconnect(NLog::global().get());

  if(m_ok_clicked)
    selected_file_list(list);

  // restore mouse cursor
  QApplication::restoreOverrideCursor();

  return list;
}

////////////////////////////////////////////////////////////////////////////

void NRemoteBrowser::set_include_files(bool includeFiles)
{
  if(this->allow_modify_bools)
    m_includeFiles = includeFiles;
}

////////////////////////////////////////////////////////////////////////////

void NRemoteBrowser::set_include_no_extension(bool includeNoExtension)
{
  if(this->allow_modify_bools)
    m_include_no_extension = includeNoExtension;
}

////////////////////////////////////////////////////////////////////////////

QStringList NRemoteBrowser::extensions() const
{
  return m_extensions;
}

////////////////////////////////////////////////////////////////////////////

bool NRemoteBrowser::include_files() const
{
  return m_includeFiles;
}

////////////////////////////////////////////////////////////////////////////

bool NRemoteBrowser::include_no_extension() const
{
  return m_include_no_extension;
}
////////////////////////////////////////////////////////////////////////////

bool NRemoteBrowser::item_exists(const QString & name) const
{
  QList<FilesListItem *>::const_iterator it = m_items.begin();
  bool found = false;

  while(it != m_items.end() && !found)
  {
    found = (*it)->text() == name;
    it++;
  }

  return found;
}

////////////////////////////////////////////////////////////////////////////

bool NRemoteBrowser::is_directory(const QString & name) const
{
  QList<FilesListItem *>::const_iterator it = m_items.begin();
  bool found = false;

  while(it != m_items.end() && !found)
  {
    FilesListItem * item = *it;
    QString path = m_current_path;
    this->assemble_path(path, item->text());

    found = item->get_type() == DIRECTORY && name == path;
    it++;
  }

  return found;
}

////////////////////////////////////////////////////////////////////////////

bool NRemoteBrowser::is_file(const QString & name) const
{
  QList<FilesListItem *>::const_iterator it = m_items.begin();
  bool found = false;

  while(it != m_items.end() && !found)
  {
    FilesListItem * item = *it;
    found = item->get_type() == FILE && item->text() == name;
    it++;
  }

  return found;
}

////////////////////////////////////////////////////////////////////////////

ValidationPolicy NRemoteBrowser::is_acceptable(const QString & name, bool isDir)
{
  return POLICY_VALID;
}

////////////////////////////////////////////////////////////////////////////

ValidationPolicy NRemoteBrowser::is_acceptable_list(const QStringList & names)
{
  return POLICY_VALID;
}

////////////////////////////////////////////////////////////////////////////

void NRemoteBrowser::show_error(const QString & message)
{
  QMessageBox::critical(this, "Error", message);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NRemoteBrowser::reinit_values()
{
  // do nothing (see doc)
}

////////////////////////////////////////////////////////////////////////////

void NRemoteBrowser::assemble_path(QString & part1, const QString & part2) const
{
  // if part1 ends with pathSep XOR part2 starts with pathSep,
  // we can append part2 to part1
  if(part1.endsWith(m_path_sep) ^ part2.startsWith(m_path_sep))
    part1.append(part2);

  // if part1 ends with pathSep AND part2 starts with pathSep,
  // we can append part2 to part1 from which the tailing pathSep has been removed
  else if(part1.endsWith(m_path_sep) && part2.startsWith(m_path_sep))
    part1.remove(0, part1.length() - m_path_sep.length() - 1).append(part2);

  else
    part1.append(m_path_sep).append(part2);

}

////////////////////////////////////////////////////////////////////////////

void NRemoteBrowser::selected_file_list(QStringList & fileList) const
{
  QModelIndexList selectedItems = m_list_view->selectionModel()->selectedIndexes();
  QModelIndexList::iterator it = selectedItems.begin();

  fileList.clear();

  while(it != selectedItems.end())
  {
    QModelIndex index = *it;
    QModelIndex indexInModel = m_filter_model->mapToSource(index);

    FilesListItem * item;
    item = static_cast<FilesListItem *>(m_view_model->itemFromIndex(indexInModel));

    if(item != nullptr)
      fileList << m_current_path + item->text();

    it++;
  }
}

////////////////////////////////////////////////////////////////////////////

void NRemoteBrowser::set_status(const QString & text)
{
  m_lab_status->setText(text);
}


// SLOTS


void NRemoteBrowser::bt_ok_clicked()
{
  QModelIndexList indexes = m_list_view->selectionModel()->selectedIndexes();
  QString name = m_current_path;
  ValidationPolicy validation;

  if(!m_multiple_select_allowed) // if show() was called
  {
    bool isDir = true;

    if(!indexes.isEmpty())
    {
      QModelIndex indexInModel = m_filter_model->mapToSource(indexes.at(0));
      FilesListItem * item;

      item = static_cast<FilesListItem *>(m_view_model->itemFromIndex(indexInModel));

      if(item != nullptr) // if an item is selected
      {
        this->assemble_path(name, item->text());
        isDir = item->get_type() == DIRECTORY;
      }
    }

    validation = this->is_acceptable(name, isDir);

    if(validation == POLICY_VALID)
    {
      m_ok_clicked = true;
      this->setVisible(false);
    }
    else if(validation == POLICY_ENTER_DIRECTORY && isDir)
      this->open_dir(name);

  } // for "if(!this->multipleSelectAllowed)"
  else // if showMultipleSelect() was called
  {
    QStringList list;

    NRemoteBrowser::selected_file_list(list);

    validation = this->is_acceptable_list(list);

    if(validation == POLICY_VALID)
    {
      disconnect(NLog::global().get());

      m_ok_clicked = true;
      this->setVisible(false);
    }

  }
}

////////////////////////////////////////////////////////////////////////////

void NRemoteBrowser::bt_cancel_clicked()
{
  disconnect(NLog::global().get());

  m_ok_clicked = false;
  this->setVisible(false);
}

////////////////////////////////////////////////////////////////////////////

void NRemoteBrowser::filter_updated(const QString & text)
{
  QRegExp regex(text, Qt::CaseInsensitive, QRegExp::Wildcard);
  m_filter_model->setFilterRegExp(regex);
}

////////////////////////////////////////////////////////////////////////////

void NRemoteBrowser::path_updated(const QString & text)
{
  static QString oldText;
  QString path;
  bool send = false;

  // if user just typed a '/', the path to explore is the current path in the field
  if(text.endsWith(m_path_sep))
  {
    send = true;
    path = text;
  }

  // if user just deleted a '/' or lengths of texts differ of more than one
  // character (this may happen if user pasted a path or delete more than
  // one character at a time), the path to explore is the parent directory of
  // the path in the field
  else if(oldText.endsWith(m_path_sep) || std::abs(oldText.length() - text.length()) > 1)
  {
    m_updating_completer = true;
    send = true;
    path = QFileInfo(text).path();
  }

  if(send)
    this->open_dir(path);

  oldText = text;
}

////////////////////////////////////////////////////////////////////////////

void NRemoteBrowser::message(const QString &error, LogMessage::Type type)
{
  if(type == LogMessage::ERROR || type == LogMessage::EXCEPTION)
  {
    // restore mouse cursor
    QApplication::restoreOverrideCursor();

    QMessageBox::critical(this, "Error", error);
  }
}

////////////////////////////////////////////////////////////////////////////

void NRemoteBrowser::double_click(const QModelIndex & index)
{
  QModelIndex indexInModel = m_filter_model->mapToSource(index);
  FilesListItem * item;
  item = static_cast<FilesListItem *>(m_view_model->itemFromIndex(indexInModel));

  if(item == nullptr)
    return;

  if(item->get_type() == DIRECTORY)
    this->open_dir(m_current_path + item->text());

  else
    this->bt_ok_clicked();
}

////////////////////////////////////////////////////////////////////////////

void NRemoteBrowser::completer_activated(const QString & text)
{
  this->open_dir(text);
}

////////////////////////////////////////////////////////////////////////////

void NRemoteBrowser::keyPressEvent(QKeyEvent * event)
{
  // key code for the pressed key
  int pressedKey = event->key();
  // modifiers keys pressed (ctrl, shift, alt, etc...)
  Qt::KeyboardModifiers modifiers = event->modifiers();

  QDialog::keyPressEvent(event);

  // if the path line edit has the focus
  if(m_edit_path->hasFocus())
  {
    // Qt::Key_Enter : enter key located on the keypad
    // Qt::Key_Return : return key
    if(pressedKey == Qt::Key_Enter || pressedKey == Qt::Key_Return)
      m_edit_path->setText(m_edit_path->text());

    return;
  }

  // if user pressed Enter key, it is similar to clicking on a button
  // (if any has the focus). Note: if none has the focus the default one ("Ok") is
  // taken (this is managed by QDialogButtonBox class)
  // Qt::Key_Enter : enter key located on the keypad
  // Qt::Key_Return : return key
  if(pressedKey == Qt::Key_Enter || pressedKey == Qt::Key_Return)
  {
    if(m_buttons->button(QDialogButtonBox::Ok)->hasFocus())
      this->bt_ok_clicked();

    else if(m_buttons->button(QDialogButtonBox::Cancel)->hasFocus())
      this->bt_cancel_clicked();
  }

  else if(pressedKey == Qt::Key_Backspace)
    this->open_dir(m_current_path + ".."); // back to the parent directory

  // if user pressed either no modifier key or Shift key *and* another key,
  // the filter line edit takes the focus
  else if(modifiers == Qt::NoModifier ||
          (modifiers == Qt::ShiftModifier && !event->text().isEmpty()))
  {
    m_list_view->clearFocus();

    m_edit_filter->setText(m_edit_filter->text() + event->text());
    this->filter_updated(m_edit_filter->text());
    m_edit_filter->setFocus(Qt::NoFocusReason);
  }
}

////////////////////////////////////////////////////////////////////////////

bool NRemoteBrowser::focusNextPrevChild(bool next)
{
  if(m_edit_path->hasFocus() && m_path_completer->popup()->isVisible())
  {
    m_path_completer->setCurrentRow(0);
    m_edit_path->setText(m_path_completer->currentCompletion());
    this->path_updated(m_path_completer->currentCompletion());
    m_path_completer->popup()->setCurrentIndex(m_path_completer->currentIndex());
    return true;
  }

  else
    return QDialog::focusNextPrevChild(next);
}


////////////////////////////////////////////////////////////////////////////

QPushButton * NRemoteBrowser::add_button(const QString & text,
                                         QDialogButtonBox::ButtonRole role)
{
  if(!text.isEmpty())
    return m_buttons->addButton(text, role);

  return nullptr;
}

////////////////////////////////////////////////////////////////////////////

void NRemoteBrowser::set_extensions(const QStringList & newExtensions)
{
  m_extensions = newExtensions;
}

////////////////////////////////////////////////////////////////////////////

QString NRemoteBrowser::current_path() const
{
  return m_current_path;
}

////////////////////////////////////////////////////////////////////////////

void NRemoteBrowser::update_model(QStandardItemModel * model,
                                  const QString & path,
                                  const std::vector<std::string> & dirs,
                                  const std::vector<std::string> & files,
                                  QList<FilesListItem *> & modelItems)
{
  QIcon dirIcon = QFileIconProvider().icon(QFileIconProvider::Folder);
  QIcon fileIcon = QFileIconProvider().icon(QFileIconProvider::File);

  std::vector<std::string>::const_iterator itDirs = dirs.begin();
  std::vector<std::string>::const_iterator itFiles = files.begin();
  QList<FilesListItem *>::iterator itItems;
  FilesListItem * item;

  // delete m_items
  itItems = modelItems.begin();

  while(itItems != modelItems.end())
  {
    delete *itItems;
    itItems++;
  }

  // clear the list and model
  modelItems.clear();
  model->clear();

  // add directories to the list
  while(itDirs != dirs.end())
  {
    QString name = itDirs->c_str();

    if(!path.isEmpty() && name != "..")
      name.prepend(path + (path.endsWith(m_path_sep) ? "" : m_path_sep));

    item = new FilesListItem(dirIcon, name + m_path_sep, DIRECTORY);
    modelItems.append(item);
    model->appendRow(item);
    itDirs++;
  }

  // add files to the list
  while(itFiles != files.end())
  {
    item = new FilesListItem(fileIcon, itFiles->c_str(), FILE);
    modelItems.append(item);
    model->appendRow(item);
    itFiles++;
  }
}

////////////////////////////////////////////////////////////////////////////

void NRemoteBrowser::open_dir(const QString & path)
{
  SignalFrame frame("read_dir", uri(), SERVER_core_PATH);
  SignalOptions options( frame );

  std::vector<std::string> vect;
  QStringList::iterator it = m_extensions.begin();

  while(it != m_extensions.end())
  {
    vect.push_back(it->toStdString());
    it++;
  }

  options.add_option("dirPath", path.toStdString());
  options.add_option("includeFiles", m_includeFiles);
  options.add_option("includeNoExtensions", m_include_no_extension);
  options.add_option("extensions", vect);

  options.flush();

  NetworkQueue::global()->send( frame, NetworkQueue::IMMEDIATE );
}

////////////////////////////////////////////////////////////////////////////

void NRemoteBrowser::read_dir(SignalArgs & args)
{
  SignalOptions options( args );

  std::vector<std::string> dirs;
  std::vector<std::string> files;

  m_current_path = options.value<std::string>("dirPath").c_str();

  // add an ending '/' if the string does not have any
  if(!m_current_path.endsWith(m_path_sep))
    m_current_path += m_path_sep;

  // clear the filter
  m_edit_filter->setText("");
  this->filter_updated("");

  if(!m_updating_completer)
    m_edit_path->setText(m_current_path);
  else
    m_updating_completer = false;

  dirs = options.array<std::string>("dirs");
  files = options.array<std::string>("files");

  this->update_model(m_view_model, "", dirs, files, m_items);
  this->update_model(m_completer_model, m_current_path, dirs,
                    std::vector<std::string>(), m_items_completer);

  // restore mouse cursor
  QApplication::restoreOverrideCursor();
}

////////////////////////////////////////////////////////////////////////////

QString NRemoteBrowser::selected_file() const
{
  QModelIndex index = m_list_view->currentIndex();
  QModelIndex indexInModel = m_filter_model->mapToSource(index);
  FilesListItem * item;

  item = static_cast<FilesListItem *>(m_view_model->itemFromIndex(indexInModel));

  if(item != nullptr && item->get_type() == FILE)
    return m_current_path + item->text();

  return QString();
}

//////////////////////////////////////////////////////////////////////////

} // Graphics
} // ui
} // cf3
