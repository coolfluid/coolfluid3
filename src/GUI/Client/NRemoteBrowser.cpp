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

#include "Common/XmlHelpers.hpp"

#include "GUI/Client/NBrowser.hpp"
#include "GUI/Client/NCore.hpp"
#include "GUI/Client/NLog.hpp"
#include "GUI/Client/ClientNetworkComm.hpp"
#include "GUI/Client/FilesListItem.hpp"
#include "GUI/Client/ClientRoot.hpp"

#include "GUI/Network/ComponentNames.hpp"

#include "GUI/Client/NRemoteBrowser.hpp"

using namespace CF::Common;
using namespace CF::GUI::Client;
using namespace CF::GUI::Network;

NRemoteBrowser::NRemoteBrowser(const QString & componentType, QMainWindow * parent)
  : QDialog(parent),
    CNode("a", componentType, CNode::BROWSER_NODE)

{
  this->rename(ClientRoot::getBrowser()->generateName().toStdString());

  regist_signal("read_dir", "Directory content")->connect(boost::bind(&NRemoteBrowser::read_dir, this, _1));

  this->setWindowTitle("Open file");

  m_clientCore = ClientRoot::getCore();

  // create the components
  m_labFilter = new QLabel("Filter (wildcards allowed) :", this);
  m_labFilesList = new QLabel("Files in", this);
  m_viewModel = new QStandardItemModel();
  m_listView = new QListView(this);
  m_editFilter = new QLineEdit(this);
  m_editPath = new QLineEdit(this);
  m_labStatus = new QLabel(this);
  m_filterModel = new QSortFilterProxyModel();
  m_completerModel = new QStandardItemModel();
  m_pathCompleter = new QCompleter(m_completerModel, this);

  m_parentWindow = parent;

  m_layout = new QVBoxLayout(this);
  m_pathLayout = new QHBoxLayout();
  m_bottomLayout = new QHBoxLayout();

  // create 2 buttons : "Ok" and "Cancel"
  m_buttons = new QDialogButtonBox(QDialogButtonBox::Ok
                                   | QDialogButtonBox::Cancel);

  m_okClicked = false;
  m_multipleSelectAllowed = false;
  m_updatingCompleter = false;

  m_pathSep = "/";

  this->setModal(true);

  m_filterModel->setDynamicSortFilter(true);

  m_filterModel->setSourceModel(m_viewModel);
  m_listView->setModel(m_filterModel);

  m_listView->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_listView->setAlternatingRowColors(true);

  m_editPath->setCompleter(m_pathCompleter);

  m_labFilter->setBuddy(m_editFilter);
  m_labFilesList->setBuddy(m_editPath);

  // add the components to the layouts
  m_pathLayout->addWidget(m_labFilesList);
  m_pathLayout->addWidget(m_editPath);

  m_bottomLayout->addWidget(m_labFilter);
  m_bottomLayout->addWidget(m_editFilter);
  m_bottomLayout->addWidget(m_buttons);

  m_layout->addLayout(m_pathLayout);
  m_layout->addWidget(m_listView);
  m_layout->addWidget(m_labStatus);
  m_layout->addLayout(m_bottomLayout);

  // set "Ok" button as default when user presses enter
  m_buttons->button(QDialogButtonBox::Ok)->setDefault(true);
  m_buttons->button(QDialogButtonBox::Cancel)->setAutoDefault(false);

  // connect useful signals to slots
  connect(m_buttons, SIGNAL(accepted()), this, SLOT(btOkClicked()));
  connect(m_buttons, SIGNAL(rejected()), this, SLOT(btCancelClicked()));

  connect(m_editFilter, SIGNAL(textEdited(const QString &)),
          this, SLOT(filterUpdated(const QString &)));

  connect(m_editPath, SIGNAL(textEdited(const QString &)),
          this, SLOT(pathUpdated(const QString &)));

  connect(m_listView, SIGNAL(doubleClicked(const QModelIndex &)),
          this, SLOT(doubleClick(const QModelIndex &)));

  connect(m_pathCompleter, SIGNAL(activated(const QString &)),
          this, SLOT(completerActivated(const QString &)));

  m_includeFiles = true;
  m_includeNoExtension = true;
  this->allowModifyBools = true;
  this->allowSingleSelect = true;
  this->allowMultipleSelect = true;

  this->resize(this->height() * 2, this->height());
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

NRemoteBrowser::~NRemoteBrowser()
{
  delete m_buttons;
  delete m_editFilter;
  delete m_filterModel;
  delete m_labFilter;
  delete m_labFilesList;
  delete m_labStatus;
  delete m_pathLayout;
  delete m_bottomLayout;
  delete m_layout;
  delete m_listView;
  delete m_editPath;
  delete m_pathCompleter;
  delete m_viewModel;
  delete m_completerModel;

  // disconnecting the signals connected by the constructor
  // (normally, this is automatically done when the object is destroyed,
  // but the documentation is not clear on this point)
  disconnect(this);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString NRemoteBrowser::show(const QString & startingDir)
{
  if(!this->allowSingleSelect)
  {
    this->showError("This dialog can not be used to select a single file");
    return QString();
  }

  if(startingDir.isEmpty())
    this->openDir(m_currentPath);

  else
    this->openDir(startingDir);

  m_multipleSelectAllowed = false;
  m_currentFile = "";
  m_currentFilesList.clear();

  m_listView->setSelectionMode(QAbstractItemView::SingleSelection);
  m_listView->clearSelection();

  connect(ClientRoot::getLog().get(), SIGNAL(newMessage(const QString &, bool)),
          this, SLOT(error(const QString &, bool)));

  this->reinitValues();

  this->exec();

  this->disconnect(ClientRoot::getLog().get());

  if(m_okClicked)
    return this->getSelectedFile();

  // restore mouse cursor
  QApplication::restoreOverrideCursor();

  return QString();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QStringList NRemoteBrowser::showMultipleSelect(const QString & startingDir)
{
  QStringList list;

  if(!this->allowMultipleSelect)
  {
    this->showError("This dialog can not be used to select multiple files");
    return QStringList();
  }

  if(startingDir.isEmpty())
    this->openDir(m_currentPath);

  else
    this->openDir(startingDir);

  m_multipleSelectAllowed = true;
  m_currentFile = "";
  m_currentFilesList.clear();

  m_listView->setSelectionMode(QAbstractItemView::ExtendedSelection);
  m_listView->clearSelection();

  connect(ClientRoot::getLog().get(), SIGNAL(newMessage(const QString &, bool)),
          this, SLOT(error(const QString &, bool)));

  this->reinitValues();

  this->exec();

  this->disconnect(ClientRoot::getLog().get());

  if(m_okClicked)
    getSelectedFileList(list);

  // restore mouse cursor
  QApplication::restoreOverrideCursor();

  return list;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NRemoteBrowser::setIncludeFiles(bool includeFiles)
{
  if(this->allowModifyBools)
    m_includeFiles = includeFiles;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NRemoteBrowser::setIncludeNoExtension(bool includeNoExtension)
{
  if(this->allowModifyBools)
    m_includeNoExtension = includeNoExtension;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QStringList NRemoteBrowser::getExtensions() const
{
  return m_extensions;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool NRemoteBrowser::getIncludeFiles() const
{
  return m_includeFiles;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool NRemoteBrowser::getIncludeNoExtension() const
{
  return m_includeNoExtension;
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool NRemoteBrowser::itemExists(const QString & name) const
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

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool NRemoteBrowser::isDirectory(const QString & name) const
{
  QList<FilesListItem *>::const_iterator it = m_items.begin();
  bool found = false;

  while(it != m_items.end() && !found)
  {
    FilesListItem * item = *it;
    QString path = m_currentPath;
    this->assemblePath(path, item->text());

    found = item->getType() == DIRECTORY && name == path;
    it++;
  }

  return found;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool NRemoteBrowser::isFile(const QString & name) const
{
  QList<FilesListItem *>::const_iterator it = m_items.begin();
  bool found = false;

  while(it != m_items.end() && !found)
  {
    FilesListItem * item = *it;
    found = item->getType() == FILE && item->text() == name;
    it++;
  }

  return found;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

ValidationPolicy NRemoteBrowser::isAcceptable(const QString & name, bool isDir)
{
  return POLICY_VALID;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

ValidationPolicy NRemoteBrowser::isAcceptableList(const QStringList & names)
{
  return POLICY_VALID;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NRemoteBrowser::showError(const QString & message)
{
  QMessageBox::critical(this, "Error", message);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NRemoteBrowser::reinitValues()
{
  // do nothing (see doc)
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NRemoteBrowser::assemblePath(QString & part1, const QString & part2) const
{
  // if part1 ends with pathSep XOR part2 starts with pathSep,
  // we can append part2 to part1
  if(part1.endsWith(m_pathSep) ^ part2.startsWith(m_pathSep))
    part1.append(part2);

  // if part1 ends with pathSep AND part2 starts with pathSep,
  // we can append part2 to part1 from which the tailing pathSep has been removed
  else if(part1.endsWith(m_pathSep) && part2.startsWith(m_pathSep))
    part1.remove(0, part1.length() - m_pathSep.length() - 1).append(part2);

  else
    part1.append(m_pathSep).append(part2);

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NRemoteBrowser::getSelectedFileList(QStringList & fileList) const
{
  QModelIndexList selectedItems = m_listView->selectionModel()->selectedIndexes();
  QModelIndexList::iterator it = selectedItems.begin();

  fileList.clear();

  while(it != selectedItems.end())
  {
    QModelIndex index = *it;
    QModelIndex indexInModel = m_filterModel->mapToSource(index);

    FilesListItem * item;
    item = static_cast<FilesListItem *>(m_viewModel->itemFromIndex(indexInModel));

    if(item != CFNULL)
      fileList << m_currentPath + item->text();

    it++;
  }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NRemoteBrowser::setStatus(const QString & text)
{
  m_labStatus->setText(text);
}


// SLOTS


void NRemoteBrowser::btOkClicked()
{
  QModelIndexList indexes = m_listView->selectionModel()->selectedIndexes();
  QString name = m_currentPath;
  ValidationPolicy validation;

  if(!m_multipleSelectAllowed) // if show() was called
  {
    bool isDir = true;

    if(!indexes.isEmpty())
    {
      QModelIndex indexInModel = m_filterModel->mapToSource(indexes.at(0));
      FilesListItem * item;

      item = static_cast<FilesListItem *>(m_viewModel->itemFromIndex(indexInModel));

      if(item != CFNULL) // if an item is selected
      {
        this->assemblePath(name, item->text());
        isDir = item->getType() == DIRECTORY;
      }
    }

    validation = this->isAcceptable(name, isDir);

    if(validation == POLICY_VALID)
    {
      m_okClicked = true;
      this->setVisible(false);
    }
    else if(validation == POLICY_ENTER_DIRECTORY && isDir)
      this->openDir(name);

  } // for "if(!this->multipleSelectAllowed)"
  else // if showMultipleSelect() was called
  {
    QStringList list;

    NRemoteBrowser::getSelectedFileList(list);

    validation = this->isAcceptableList(list);

    if(validation == POLICY_VALID)
    {
      disconnect(ClientRoot::getLog().get());

      m_okClicked = true;
      this->setVisible(false);
    }

  }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NRemoteBrowser::btCancelClicked()
{
  disconnect(ClientRoot::getLog().get());

  m_okClicked = false;
  this->setVisible(false);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NRemoteBrowser::filterUpdated(const QString & text)
{
  QRegExp regex(text, Qt::CaseInsensitive, QRegExp::Wildcard);
  m_filterModel->setFilterRegExp(regex);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NRemoteBrowser::pathUpdated(const QString & text)
{
  static QString oldText;
  QString path;
  bool send = false;

  // if user just typed a '/', the path to explore is the current path in the field
  if(text.endsWith(m_pathSep))
  {
    send = true;
    path = text;
  }

  // if user just deleted a '/' or lengths of texts differ of more than one
  // character (this may happen if user pasted a path or delete more than
  // one character at a time), the path to explore is the parent directory of
  // the path in the field
  else if(oldText.endsWith(m_pathSep) || std::abs(oldText.length() - text.length()) > 1)
  {
    m_updatingCompleter = true;
    send = true;
    path = QFileInfo(text).path();
  }

  if(send)
    this->openDir(path);

  oldText = text;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NRemoteBrowser::error(const QString & error, bool fromServer)
{
  // restore mouse cursor
  QApplication::restoreOverrideCursor();

  QMessageBox::critical(this, "Error", error);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NRemoteBrowser::doubleClick(const QModelIndex & index)
{
  QModelIndex indexInModel = m_filterModel->mapToSource(index);
  FilesListItem * item;
  item = static_cast<FilesListItem *>(m_viewModel->itemFromIndex(indexInModel));

  if(item == CFNULL)
    return;

  if(item->getType() == DIRECTORY)
    this->openDir(m_currentPath + item->text());

  else
    this->btOkClicked();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NRemoteBrowser::completerActivated(const QString & text)
{
  this->openDir(text);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NRemoteBrowser::keyPressEvent(QKeyEvent * event)
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

  // if user pressed Enter key, it similar to clicking on a button
  // (if any has focus). Note: if none has the focus the default one ("Ok") is
  // taken (this is managed by QDialogButtonBox class)
  // Qt::Key_Enter : enter key located on the keypad
  // Qt::Key_Return : return key
  if(pressedKey == Qt::Key_Enter || pressedKey == Qt::Key_Return)
  {
    if(m_buttons->button(QDialogButtonBox::Ok)->hasFocus())
      this->btOkClicked();

    else if(m_buttons->button(QDialogButtonBox::Cancel)->hasFocus())
      this->btCancelClicked();
  }

  else if(pressedKey == Qt::Key_Backspace)
    this->openDir(m_currentPath + ".."); // back to the parent directory

  // if user pressed either no modifier key or Shift key *and* another key,
  // the filter line edit takes the focus
  else if(modifiers == Qt::NoModifier ||
          (modifiers == Qt::ShiftModifier && !event->text().isEmpty()))
  {
    m_listView->clearFocus();

    m_editFilter->setText(m_editFilter->text() + event->text());
    this->filterUpdated(m_editFilter->text());
    m_editFilter->setFocus(Qt::NoFocusReason);
  }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool NRemoteBrowser::focusNextPrevChild(bool next)
{
  if(m_editPath->hasFocus() && m_pathCompleter->popup()->isVisible())
  {
    m_pathCompleter->setCurrentRow(0);
    m_editPath->setText(m_pathCompleter->currentCompletion());
    this->pathUpdated(m_pathCompleter->currentCompletion());
    m_pathCompleter->popup()->setCurrentIndex(m_pathCompleter->currentIndex());
    return true;
  }

  else
    return QDialog::focusNextPrevChild(next);
}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QPushButton * NRemoteBrowser::addButton(const QString & text,
                                         QDialogButtonBox::ButtonRole role)
{
  if(!text.isEmpty())
    return m_buttons->addButton(text, role);

  return CFNULL;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NRemoteBrowser::setExtensions(const QStringList & newExtensions)
{
  m_extensions = newExtensions;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString NRemoteBrowser::getCurrentPath() const
{
  return m_currentPath;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NRemoteBrowser::updateModel(QStandardItemModel * model,
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
      name.prepend(path + (path.endsWith(m_pathSep) ? "" : m_pathSep));

    item = new FilesListItem(dirIcon, name + m_pathSep, DIRECTORY);
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

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NRemoteBrowser::openDir(const QString & path)
{
  boost::shared_ptr<XmlDoc> docnode = XmlOps::create_doc();
  XmlNode * rootNode = XmlOps::goto_doc_node(*docnode.get());
  XmlNode * signalNode = XmlOps::add_signal_frame(*rootNode, "read_dir", full_path(), SERVER_CORE_PATH, true);
  XmlParams p(*signalNode);
  std::vector<std::string> vect;
  QStringList::iterator it = m_extensions.begin();

  while(it != m_extensions.end())
  {
    vect.push_back(it->toStdString());
    it++;
  }

  p.add_param("dirPath", path.toStdString());
  p.add_param("includeFiles", m_includeFiles);
  p.add_param("includeNoExtensions", m_includeNoExtension);
  p.add_array("extensions", vect);

  m_clientCore->sendSignal(*docnode.get());
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Signal::return_t NRemoteBrowser::read_dir(Signal::arg_t & node)
{
  XmlParams p(node);

  std::vector<std::string> dirs;
  std::vector<std::string> files;

  m_currentPath = p.get_param<std::string>("dirPath").c_str();

  // add an ending '/' if the string does not have any
  if(!m_currentPath.endsWith(m_pathSep))
    m_currentPath += m_pathSep;

  // clear the filter
  m_editFilter->setText("");
  this->filterUpdated("");

  if(!m_updatingCompleter)
    m_editPath->setText(m_currentPath);
  else
    m_updatingCompleter = false;

  dirs = p.get_array<std::string>("dirs");
  files = p.get_array<std::string>("files");

  this->updateModel(m_viewModel, "", dirs, files, m_items);
  this->updateModel(m_completerModel, m_currentPath, dirs,
                    std::vector<std::string>(), m_itemsCompleter);

  // restore mouse cursor
  QApplication::restoreOverrideCursor();

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString NRemoteBrowser::getSelectedFile() const
{
  QModelIndex index = m_listView->currentIndex();
  QModelIndex indexInModel = m_filterModel->mapToSource(index);
  FilesListItem * item;

  item = static_cast<FilesListItem *>(m_viewModel->itemFromIndex(indexInModel));

  if(item != CFNULL && item->getType() == FILE)
    return m_currentPath + item->text();

  return QString();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//void NRemoteBrowser::ack(NetworkFrameType type)
//{
//  switch(type)
//  {
//    case NETWORK_CREATE_DIR:
//      this->openDir(m_currentPath);
//      break;

//    default:
//      ClientRoot::getLog()->addMessage("Unexpected ACK recieved");
//      break;
//  }
//}
