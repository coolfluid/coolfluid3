// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QApplication>
#include <QCloseEvent>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QHeaderView>
#include <QListView>
#include <QMessageBox>
#include <QMenuBar>
#include <QPushButton>
#include <QScrollArea>
#include <QSplitter>
#include <QTableView>
#include <QTabWidget>
#include <QVBoxLayout>

#include <QDebug>

#include "Common/Exception.hpp"
#include "Common/ConfigArgs.hpp"

#include "GUI/Client/UI/AboutCFDialog.hpp"
#include "GUI/Client/Core/ClientRoot.hpp"
#include "GUI/Client/UI/ConnectionDialog.hpp"
#include "GUI/Client/UI/LoggingList.hpp"
#include "GUI/Client/UI/MenuActionInfo.hpp"
#include "GUI/Client/UI/CentralPanel.hpp"
#include "GUI/Client/UI/NRemoteSave.hpp"
#include "GUI/Client/UI/NRemoteOpen.hpp"
#include "GUI/Client/UI/SelectFileDialog.hpp"
#include "GUI/Client/Core/StatusModel.hpp"
#include "GUI/Client/UI/StatusPanel.hpp"
#include "GUI/Client/UI/TreeBrowser.hpp"
#include "GUI/Client/UI/TreeView.hpp"
#include "GUI/Client/Core/PropertyModel.hpp"

#include "GUI/Network/ComponentNames.hpp"

#include "GUI/Client/UI/MainWindow.hpp"

#define connectSig(comm,slotSig) connect(comm, SIGNAL(slotSig), this, SLOT(slotSig));
#define connectKernel(slotSig) connect(m_treeView, SIGNAL(slotSig), \
&ClientCore::instance(), SLOT(slotSig));
#define WORKSPACE_FILE QDir::homePath() + "/CF_workspace.xml"

using namespace CF::GUI::ClientCore;
using namespace CF::GUI::ClientUI;
using namespace CF::GUI::Network;

using namespace CF::Common;

MainWindow::MainWindow()
  : m_logFile(new QFile(QString("coolfluid_client-") +
            QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + ".log"))
{
  this->setWindowTitle("COOLFluiD client");

  this->statusBar(); // has to be called to create the status bar

  m_logFile.device()->open(QIODevice::WriteOnly);

  // create the components
  m_centralPanel = new CentralPanel(this);
  m_treeView = new TreeView(m_centralPanel, this);
  m_statusPanel = nullptr;//new StatusPanel(m_statusModel, this);
  m_splitter = new QSplitter(Qt::Horizontal, this);
  m_centralSplitter = new QSplitter(Qt::Vertical, this);
  m_centralWidgetLayout = new QVBoxLayout(m_centralSplitter);
  m_tabWindow = new QTabWidget(m_centralPanel);
  m_logList = new LoggingList(m_tabWindow);
  m_propertyModel = new PropertyModel();
  m_propertyView = new QTableView(m_tabWindow);
  m_labDescription = new QLabel(m_tabWindow);
  m_treeBrowser = new TreeBrowser(m_treeView, this);
  m_scrollDescription = new QScrollArea(this);

  m_aboutCFDialog = new AboutCFDialog(this);

  // configure components

  m_propertyView->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
  m_propertyView->horizontalHeader()->setStretchLastSection(true);

  m_propertyView->setModel(m_propertyModel);

  m_labDescription->setTextFormat(Qt::RichText);
  m_labDescription->setAlignment(Qt::AlignTop);
  m_labDescription->setWordWrap(true);

  m_scrollDescription->setWidgetResizable(true);
  m_scrollDescription->setWidget(m_labDescription);

  m_tabWindow->addTab(m_logList, "Log");
  m_tabWindow->addTab(m_propertyView, "Properties");
  m_tabWindow->addTab(m_scrollDescription, "Description");

  m_centralWidgetLayout->addWidget(m_centralPanel);
  m_centralWidgetLayout->addWidget(m_tabWindow);

  m_centralWidgetLayout->setContentsMargins(0, 0, 0, 0);

  m_centralSplitter->setStretchFactor(0, 10);

  // add the components to the splitter
  m_splitter->addWidget(m_treeBrowser);

  m_splitter->addWidget(m_centralSplitter);
//  m_splitter->addWidget(m_statusPanel);
  m_splitter->setStretchFactor(1, 10);

  m_splitter->setHandleWidth(0);

  this->setCentralWidget(m_splitter);

  this->buildMenus();

  connect(ClientRoot::log().get(), SIGNAL(newException(QString)),
          this, SLOT(newException(QString)));

  connect(ClientRoot::log().get(), SIGNAL(newMessage(QString, CF::GUI::Network::LogMessage::Type)),
          this, SLOT(newLogMessage(QString,CF::GUI::Network::LogMessage::Type)));

  connect(ClientRoot::core().get(), SIGNAL(connectedToServer()),
          this, SLOT(connectedToServer()));

  connect(ClientRoot::core().get(), SIGNAL(disconnectedFromServer()),
          this, SLOT(disconnectedFromServer()));

  connect(ClientRoot::tree().get(), SIGNAL(currentIndexChanged(QModelIndex,QModelIndex)),
          this, SLOT(currentIndexChanged(QModelIndex,QModelIndex)));

  connect(m_tabWindow, SIGNAL(currentChanged(int)), this, SLOT(tabClicked(int)));

  this->setConnectedState(false);

  ClientRoot::log()->addMessage("Client successfully launched.");
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MainWindow::~MainWindow()
{
  m_logFile.flush();

  delete m_treeView;
  delete m_centralPanel;
  delete m_statusPanel;

  delete m_logList;
  delete m_mnuView;
  delete m_mnuFile;
  delete m_mnuHelp;
  delete m_aboutCFDialog;

//  m_logFile.close();
}

 // PRIVATE METHODS

void MainWindow::buildMenus()
{
  MenuActionInfo actionInfo;
  QAction * tmpAction;

  m_mnuFile = new QMenu("&File", this);
  m_mnuOpenFile = new QMenu("&Open file", this);
  m_mnuSaveFile = new QMenu("&Save file", this);
  m_mnuView = new QMenu("&View", this);
  m_mnuHelp = new QMenu("&Help", this);

  actionInfo.initDefaults();
  actionInfo.m_menu = m_mnuFile;
  actionInfo.m_text = "&Connect to server";
  actionInfo.m_shortcut = tr("ctrl+shift+C");
  actionInfo.m_slot = SLOT(connectToServer());

  m_actions[MainWindow::ACTION_CONNECT_TO_SERVER] = actionInfo.buildAction(this);

  //-----------------------------------------------

  actionInfo.initDefaults();
  actionInfo.m_menu = m_mnuFile;
  actionInfo.m_text = "&Disconnect from server";
  actionInfo.m_shortcut = tr("ctrl+shift+x");
  actionInfo.m_slot = SLOT(disconnectFromServer());

  m_actions[MainWindow::ACTION_DISCONNECT_FROM_SERVER] = actionInfo.buildAction(this);

  //-----------------------------------------------

  actionInfo.initDefaults();
  actionInfo.m_menu = m_mnuFile;
  actionInfo.m_text = "&Shutdown the server";
  actionInfo.m_shortcut = tr("ctrl+shift+K");
  actionInfo.m_slot = SLOT(disconnectFromServer());

  m_actions[MainWindow::ACTION_SHUTDOWN_SERVER] = actionInfo.buildAction(this);


  actionInfo.initDefaults();
  actionInfo.m_menu = m_mnuFile;
  actionInfo.m_text = "&Do hard work";
  actionInfo.m_slot = SLOT(hardWork());

  m_actions[MainWindow::ACTION_SHUTDOWN_SERVER] = actionInfo.buildAction(this);


  //-----------------------------------------------

  m_mnuFile->addSeparator();

  //-----------------------------------------------

  actionInfo.initDefaults();
  actionInfo.m_menu = m_mnuOpenFile;
  actionInfo.m_text = "&Locally";
  actionInfo.m_shortcut = tr("ctrl+O");
  actionInfo.m_slot = SLOT(openFileLocally());

  m_actions[MainWindow::ACTION_OPEN_LOCALLY] = actionInfo.buildAction(this);

  //-----------------------------------------------

  actionInfo.initDefaults();
  actionInfo.m_menu = m_mnuOpenFile;
  actionInfo.m_text = "&Remotely";
  actionInfo.m_shortcut = tr("ctrl+shift+O");
  actionInfo.m_slot = SLOT(openFileRemotely());

  m_actions[MainWindow::ACTION_OPEN_REMOTELY] = actionInfo.buildAction(this);

  //-----------------------------------------------

  actionInfo.initDefaults();
  actionInfo.m_menu = m_mnuSaveFile;
  actionInfo.m_text = "&Locally";
  actionInfo.m_shortcut = tr("ctrl+S");
  actionInfo.m_slot = SLOT(saveFileLocally());

  m_actions[MainWindow::ACTION_SAVE_LOCALLY] = actionInfo.buildAction(this);

  //-----------------------------------------------

  actionInfo.initDefaults();
  actionInfo.m_menu = m_mnuSaveFile;
  actionInfo.m_text = "&Remotely";
  actionInfo.m_shortcut = tr("ctrl+shift+S");
  actionInfo.m_slot = SLOT(saveFileRemotely());

  m_actions[MainWindow::ACTION_SAVE_REMOTELY] = actionInfo.buildAction(this);

  //-----------------------------------------------

  m_mnuFile->addMenu(m_mnuOpenFile);
  m_mnuFile->addMenu(m_mnuSaveFile);
  m_mnuFile->addSeparator();

  //-----------------------------------------------

  actionInfo.initDefaults();
  actionInfo.m_menu = m_mnuFile;
  actionInfo.m_text = "&Update tree";
  actionInfo.m_shortcut = tr("ctrl+U");

  tmpAction = actionInfo.buildAction(this);
  connect(tmpAction, SIGNAL(triggered()), ClientRoot::core().get(), SLOT(updateTree()));
  m_actions[MainWindow::ACTION_UPDATE_TREE] = tmpAction;


  //-----------------------------------------------

  actionInfo.initDefaults();
  actionInfo.m_menu = m_mnuView;
  actionInfo.m_text = "&Clear log messages";

  tmpAction = actionInfo.buildAction(this);

  m_actions[MainWindow::ACTION_CLEAR_LOG] = tmpAction;
  connect(tmpAction, SIGNAL(triggered()), this->m_logList , SLOT(clearLog()));

  //-----------------------------------------------

  m_mnuView->addSeparator();

  //-----------------------------------------------

  actionInfo.initDefaults();
  actionInfo.m_menu = m_mnuView;
  actionInfo.m_text = "&Find component";
  actionInfo.m_shortcut = tr("ctrl+F");

  tmpAction = actionInfo.buildAction(this);

  connect(tmpAction, SIGNAL(triggered()), m_treeBrowser, SLOT(focusFilter()));


  actionInfo.initDefaults();
  actionInfo.m_menu = m_mnuView;
  actionInfo.m_text = "&Toggle information pane";
  actionInfo.m_shortcut = tr("ctrl+I");

  tmpAction = actionInfo.buildAction(this);

  tmpAction->setCheckable(true);

  tmpAction->setChecked(true);
  m_tabWindow->setVisible(true);

  m_actions[MainWindow::ACTION_TOGGLE_INFO_PANE] = tmpAction;

  // QTabWidget overrides setVisible(bool) slot to hide/show the widget
  // contained in the tab. Since we want to hide/show the whole pane
  // (including tab bar), we need to cast the object to QWidget.
  connect(tmpAction, SIGNAL(toggled(bool)), (QWidget*)this->m_tabWindow, SLOT(setVisible(bool)));

  //-----------------------------------------------

  actionInfo.initDefaults();
  actionInfo.m_menu = m_mnuView;
  actionInfo.m_text = "Toggle &advanced mode";
  actionInfo.m_slot = SLOT(toggleAdvanced());
  actionInfo.m_shortcut = tr("ctrl+X");
  actionInfo.m_checkable = true;

  m_actions[MainWindow::ACTION_TOGGLE_ADVANCED_MODE] = actionInfo.buildAction(this);

  //-----------------------------------------------

  actionInfo.initDefaults();
  actionInfo.m_menu = m_mnuView;
  actionInfo.m_text = "Toggle &debug mode";
  actionInfo.m_slot = SLOT(toggleDebugMode());
  actionInfo.m_shortcut = tr("ctrl+D");
  actionInfo.m_checkable = true;

  m_actions[MainWindow::ACTION_TOGGLE_DEBUG_MODE] = actionInfo.buildAction(this);

  //-----------------------------------------------

  actionInfo.initDefaults();
  actionInfo.m_menu = m_mnuView;
  actionInfo.m_text = "&Show/Hide status panel";
  actionInfo.m_slot = SLOT(showHideStatus());
  actionInfo.m_checkable = true;

  m_actions[MainWindow::ACTION_SHOW_HIDE_STATUS_PANEL] = actionInfo.buildAction(this);
  m_actions[MainWindow::ACTION_SHOW_HIDE_STATUS_PANEL]->setChecked(true);

  //----------------------------------------------------
  //----------------------------------------------------

  actionInfo.initDefaults();
  actionInfo.m_menu = m_mnuHelp;
  actionInfo.m_text = "&Help";
  actionInfo.m_shortcut = tr("F1");
  actionInfo.m_slot = SLOT(showHelp());

  m_actions[MainWindow::ACTION_HELP] = actionInfo.buildAction(this);

  //-----------------------------------------------

  m_mnuView->addSeparator();

  //-----------------------------------------------

  actionInfo.initDefaults();
  actionInfo.m_menu = m_mnuHelp;
  actionInfo.m_text = "&About CF";

  tmpAction = actionInfo.buildAction(this);

  m_actions[MainWindow::ACTION_ABOUT_COOLFLUID] = tmpAction;
  connect(tmpAction, SIGNAL(triggered()), m_aboutCFDialog, SLOT(exec()));

  //-----------------------------------------------


  actionInfo.initDefaults();
  actionInfo.m_menu = m_mnuHelp;
  actionInfo.m_text = "&About Qt";

  tmpAction = actionInfo.buildAction(this);

  m_actions[MainWindow::ACTION_ABOUT_COOLFLUID] = tmpAction;
  connect(tmpAction, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

  //----------------------------------------------------
  //----------------------------------------------------

//  m_treeView->addSimToMenuBar(this->menuBar());
  this->menuBar()->addMenu(m_mnuFile);
  this->menuBar()->addMenu(m_mnuView);
  this->menuBar()->addMenu(m_mnuHelp);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void MainWindow::setSimRunning(bool simRunning)
{
  //  this->optionPanel->setReadOnly(simRunning);
  //  this->treeView->setReadOnly(simRunning);

  m_mnuOpenFile->setEnabled(!simRunning);
  m_mnuSaveFile->setEnabled(!simRunning);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void MainWindow::setFileOpen(bool fileOpen)
{
  m_mnuSaveFile->setEnabled(fileOpen);
  m_centralPanel->setVisible(fileOpen);

  m_treeView->setVisible(fileOpen);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int MainWindow::confirmClose()
{
  int answer;
  QMessageBox discBox(this);
  QPushButton * btDisc = nullptr;
  QPushButton * btCancel = nullptr;
  QPushButton * btShutServer = nullptr;

  btDisc = discBox.addButton("Disconnect", QMessageBox::NoRole);
  btCancel = discBox.addButton(QMessageBox::Cancel);
  btShutServer = discBox.addButton("Shutdown server", QMessageBox::YesRole);

  discBox.setWindowTitle("Confirmation");
  discBox.setText("You are about to disconnect from the server.");
  discBox.setInformativeText("What do you want to do ?");

  // show the message box
  discBox.exec();

  if(discBox.clickedButton() == btDisc)
    answer = CLOSE_DISC;
  else if(discBox.clickedButton() == btShutServer)
    answer = CLOSE_SHUTDOWN;
  else
    answer = CLOSE_CANCEL;

  delete btDisc;
  delete btCancel;
  delete btShutServer;

  return answer;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool MainWindow::saveFromInfos()
{
  bool ok = false;
  // if the file has to be saved
//  if(!m_infos.filename.isEmpty())
//  {
//    // if user wants to save it locally...
//    if(m_infos.saveLocally)
//    {
//      if(!this->saveToFileLocally(m_infos.filename))
//      {
//        this->showError(QString("Configuration could not be saved to %1")
//                        .arg(m_infos.filename));
//      }

//      else
//        ok = true;
//    }
//    // ... or remotely
//    else
//      ok = this->saveToFileRemotely(m_infos.filename);
//  } // for "if(!this->infos.filename.isEmpty())"

  return ok;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool MainWindow::saveToFileLocally(const QString & filename)
{
  bool retValue = false;

  if(filename.isEmpty())
    return false;

  try
  {
//    QFile file(filename);
//    QTextStream out;
//    QString tree = m_treeModel->getDocument().toString();
//    XMLNode xmlNode = ConverterTools::xmlToXCFcase(tree.toStdString());

//    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
//    {
//      QString error = "Could not open file '%1' for write access: %2";
//      ClientRoot::getLog()->addError(error.arg(filename).arg(file.errorString()));
//    }
//    else
//    {
//      out.setDevice(&file);

//      if(filename.endsWith(".CFcase"))
//      {
//        ConfigArgs args = ConverterTools::xCFcaseToConfigArgs(xmlNode);
//        out << ConverterTools::configArgsToCFcase(args).c_str();
//      }

//      else
//        out << xmlNode.createXMLString();

//      file.close();

//      ClientRoot::getLog()->addMessage(QString("The configuration has been successfully "
//                                 "written to '%1'.").arg(filename));
//      retValue = true;
//    }
  }
  catch(Exception & e)
  {
    ClientRoot::log()->addException(e.what());
  }

  return retValue;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool MainWindow::saveToFileRemotely(const QString & filename)
{
  if(!filename.isEmpty())
  {
//    QDomDocument doc = m_treeModel->getDocument();
//    XMLNode node = ConverterTools::xmlToXCFcase(doc.toString().toStdString());
//    doc.setContent(QString(node.createXMLString()));

    return true;
  }

  else
    return false;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void MainWindow::showError(const QString & errorMessage)
{
  QMessageBox::critical(this, "Error", errorMessage);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void MainWindow::showMessage(const QString & message)
{
  QMessageBox::information(this, "Information", message);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void MainWindow::showWarning(const QString & message)
{
  QMessageBox::warning(this, "Warning", message);
}

/****************************************************************************

 PROTECTED METHOD

 ****************************************************************************/

void MainWindow::closeEvent(QCloseEvent * event)
{
  // if the event is accepted, we write the current workspace to the disk
  if(event->isAccepted())
  {
//    QDomDocument doc = m_treeModel->getDocument();
//    QFile configFile(WORKSPACE_FILE);

//    if(configFile.open(QIODevice::WriteOnly))
//    {
//      QTextStream out(&configFile);
//      out << doc.toString();
//      configFile.close();
//    }
//    else
//      QMessageBox::critical(this, "Error", "Could not save current workspace to disk.");
  }

  return;
}

/****************************************************************************

 SLOTS

 ****************************************************************************/

void MainWindow::quit()
{
  ClientRoot::core()->disconnectFromServer(false);
  qApp->exit(0);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void MainWindow::toggleAdvanced()
{
  bool advanced = m_actions[ ACTION_TOGGLE_ADVANCED_MODE ]->isChecked();
  ClientRoot::tree()->setAdvancedMode(advanced);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void MainWindow::toggleDebugMode()
{
  bool debug = m_actions[ ACTION_TOGGLE_DEBUG_MODE ]->isChecked();
  ClientRoot::tree()->setDebugModeEnabled(debug);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void MainWindow::showHideStatus()
{
//  bool show = m_actions[ ACTION_SHOW_HIDE_STATUS_PANEL ]->isChecked();
//  m_statusPanel->setVisible(show);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void MainWindow::showHelp()
{
  this->showError("There is no help for now!");
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void MainWindow::newException(const QString & msg)
{
  this->showError(msg);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void MainWindow::connectToServer()
{
  ConnectionDialog dlg(this);
  TSshInformation sshInfo;

  if(dlg.show(false, sshInfo))
  {
    ClientRoot::core()->connectToServer(sshInfo);
  }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void MainWindow::disconnectFromServer()
{
  ClientRoot::core()->disconnectFromServer(sender() == m_actions[ACTION_SHUTDOWN_SERVER]);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void MainWindow::connectedToServer()
{
  this->setConnectedState(true);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void MainWindow::disconnectedFromServer()
{
  this->setConnectedState(false);
  ClientRoot::tree()->clearTree();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void MainWindow::setConnectedState(bool connected)
{
  m_actions[ACTION_CONNECT_TO_SERVER]->setEnabled(!connected);
  m_actions[ACTION_DISCONNECT_FROM_SERVER]->setEnabled(connected);
  m_actions[ACTION_SHUTDOWN_SERVER]->setEnabled(connected);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void MainWindow::saveFileLocally()
{
  QFileDialog dlg;

  dlg.setAcceptMode(QFileDialog::AcceptSave);
  dlg.exec();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void MainWindow::saveFileRemotely()
{
  NRemoteSave::Ptr flg;

  flg->show();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void MainWindow::openFileLocally()
{
  QFileDialog dlg;

  dlg.setAcceptMode(QFileDialog::AcceptOpen);
  dlg.exec();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void MainWindow::openFileRemotely()
{
  NRemoteOpen::Ptr rop = NRemoteOpen::create();
  rop->show();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void MainWindow::newLogMessage(const QString & message, CF::GUI::Network::LogMessage::Type type)
{
  m_logFile << message << '\n';
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void MainWindow::tabClicked(int num)
{
  if(m_tabWindow->currentWidget() == m_propertyView)
    m_propertyView->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void MainWindow::currentIndexChanged(const QModelIndex & newIndex, const QModelIndex & oldIndex)
{
  QString text = "<b>%1</b><br><br>%2";
  QMap<QString, QString> data;

  ClientRoot::tree()->listNodeProperties(newIndex, data);

  text = text.arg(data["brief"]).arg(data["description"]);
  m_labDescription->setText(text.replace("\n","<br>"));
}

void MainWindow::hardWork()
{
  NCore::Ptr core = ClientRoot::core();
  boost::shared_ptr<XmlDoc> xmldoc = XmlOps::create_doc();
  XmlNode & xmlnode = *XmlOps::goto_doc_node(*xmldoc.get());

  XmlOps::add_signal_frame(xmlnode, "heavy_compute", core->full_path(), SERVER_CORE_PATH, true);

  core->sendSignal(*xmldoc.get());
}
