// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QApplication>
#include <QCloseEvent>
#include <QDateTime>
#include <QDesktopServices>
#include <QFileDialog>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QMenuBar>
#include <QScrollArea>
#include <QSplitter>
#include <QUrl>

#include "GUI/Core/TreeThread.hpp"
#include "GUI/Core/NetworkThread.hpp"
#include "GUI/Core/NLog.hpp"
#include "GUI/Core/NTree.hpp"
#include "GUI/Core/PropertyModel.hpp"
#include "GUI/Core/ThreadManager.hpp"

#include "GUI/Graphics/AboutCFDialog.hpp"
#include "GUI/Graphics/LoggingList.hpp"
#include "GUI/Graphics/CentralPanel.hpp"
#include "GUI/Graphics/Graph.hpp"
#include "GUI/Graphics/NRemoteOpen.hpp"
#include "GUI/Graphics/SignatureDialog.hpp"
#include "GUI/Client/UI/TabBuilder.hpp"
#include "GUI/Graphics/TreeBrowser.hpp"
#include "GUI/Graphics/TreeView.hpp"

#include "GUI/UICommon/ComponentNames.hpp"

#include "GUI/Graphics/MainWindow.hpp"

#define WORKSPACE_FILE QDir::homePath() + "/CF_workspace.xml"

using namespace CF::GUI::Core;
using namespace CF::GUI::UICommon;

using namespace CF::Common;
using namespace CF::Common::XML;

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace Graphics {

////////////////////////////////////////////////////////////////////////////

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
  m_splitter = new QSplitter(/*Qt::Horizontal, this*/);
  m_centralSplitter = new QSplitter(Qt::Vertical/*, this*/);
  m_tabWindow = new QTabWidget(m_centralPanel);
  m_logList = new LoggingList(m_tabWindow);
  m_propertyModel = new PropertyModel();
  m_propertyView = new QTableView(m_tabWindow);
  m_labDescription = new QLabel(m_tabWindow);
  m_treeBrowser = new TreeBrowser(m_treeView, this);
  m_scrollDescription = new QScrollArea(this);
  m_graphXYPlot = new Graph(this);

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

  TabBuilder::instance()->addTab(m_centralPanel, "Options");
  TabBuilder::instance()->addTab(m_graphXYPlot, "XY-Plot");

  m_centralSplitter->setStretchFactor(0, 10);

  // add the components to the splitter
  m_splitter->addWidget(m_treeBrowser);

  m_centralSplitter->addWidget(TabBuilder::instance());
  m_centralSplitter->addWidget(m_tabWindow);
  m_splitter->addWidget(m_centralSplitter);

  m_splitter->setStretchFactor(1, 10);

  m_splitter->setHandleWidth(0);

  this->setCentralWidget(m_splitter);

  this->buildMenus();

  NRoot* root = ThreadManager::instance().tree().root().get();

  connect(NLog::globalLog().get(), SIGNAL(newException(QString)),
          this, SLOT(newException(QString)));

  connect(NLog::globalLog().get(),
          SIGNAL(newMessage(QString, CF::GUI::UICommon::LogMessage::Type)),
          this, SLOT(newLogMessage(QString,CF::GUI::UICommon::LogMessage::Type)));

  connect(root, SIGNAL(connected()), this, SLOT(connectedToServer()));

  connect(&ThreadManager::instance().network(), SIGNAL(disconnectedFromServer()),
          this, SLOT(disconnectedFromServer()));

  connect(NTree::globalTree().get(),
          SIGNAL(currentIndexChanged(QModelIndex,QModelIndex)),
          this, SLOT(currentIndexChanged(QModelIndex,QModelIndex)));

  connect(m_tabWindow, SIGNAL(currentChanged(int)), this, SLOT(tabClicked(int)));

  this->setConnectedState(false);

  NLog::globalLog()->addMessage("Client successfully launched.");
}

///////////////////////////////////////////////////////////////////////////

MainWindow::~MainWindow()
{
  m_logFile.flush();

  delete m_treeView;
  delete m_centralPanel;

  delete m_logList;
  delete m_mnuView;
  delete m_mnuFile;
  delete m_mnuHelp;
  delete m_aboutCFDialog;

//  m_logFile.close();
}


//========================================================================
//                                PRIVATE METHODS
//========================================================================

void MainWindow::buildMenus()
{
  QAction * action;

  m_mnuFile = new QMenu("&File", this);
  m_mnuOpenFile = new QMenu("&Open file", this);
  m_mnuSaveFile = new QMenu("&Save file", this);
  m_mnuView = new QMenu("&View", this);
  m_mnuHelp = new QMenu("&Help", this);


  action = m_mnuFile->addAction("&Connect to server", this,
                                SLOT(connectToServer()), tr("ctrl+shift+C"));
  m_actions[ACTION_CONNECT_TO_SERVER] = action;

  action = m_mnuFile->addAction("&Disconnect from server", this,
                                SLOT(disconnectFromServer()), tr("ctrl+shift+x"));
  m_actions[ACTION_DISCONNECT_FROM_SERVER] = action;

  action = m_mnuFile->addAction("&Shutdown the server", this,
                                SLOT(disconnectFromServer()), tr("ctrl+shift+K"));
  m_actions[ACTION_SHUTDOWN_SERVER] = action;

  m_mnuFile->addSeparator();

  //-----------------------------------------------

  action = m_mnuOpenFile->addAction("&Locally", this, SLOT(openFileLocally()), tr("ctrl+o"));
  m_actions[ACTION_OPEN_LOCALLY] = action;

  action = m_mnuOpenFile->addAction("&Remotely", this, SLOT(openFileRemotely()), tr("ctrl+shift+o"));
  m_actions[ACTION_OPEN_REMOTELY] = action;

  action = m_mnuOpenFile->addAction("&Locally", this, SLOT(saveFileLocally()), tr("ctrl+s"));
  m_actions[ACTION_SAVE_LOCALLY] = action;

  action = m_mnuOpenFile->addAction("&Remotely", this, SLOT(saveFileRemotely()), tr("ctrl+shift+s"));
  m_actions[ACTION_SAVE_REMOTELY] = action;

  m_mnuFile->addMenu(m_mnuOpenFile);
  m_mnuFile->addMenu(m_mnuSaveFile);
  m_mnuFile->addSeparator();

  //-----------------------------------------------

  action = m_mnuFile->addAction("&Update tree", NTree::globalTree().get(),
                                SLOT(updateTree()), tr("ctrl+u") );
  m_actions[ACTION_UPDATE_TREE] = action;

  //-----------------------------------------------
  //-----------------------------------------------

  action = m_mnuView->addAction("&Clear log messages", this->m_logList , SLOT(clearLog()));
  m_actions[ACTION_CLEAR_LOG] = action;

  m_mnuView->addSeparator();

  //-----------------------------------------------

  m_mnuView->addAction("&Find component", m_treeBrowser, SLOT(focusFilter()), tr("ctrl+f") );

  action = m_mnuView->addAction("&Toggle information pane");
  action->setCheckable(true);
  action->setChecked(true);
  action->setShortcut( tr("ctrl+i") );
  m_tabWindow->setVisible(true);
  m_actions[ACTION_TOGGLE_INFO_PANE] = action;

  // QTabWidget overrides setVisible(bool) slot to hide/show the widget
  // contained in the tab. Since we want to hide/show the whole pane
  // (including tab bar), we need to cast the object to QWidget.
  connect(action, SIGNAL(toggled(bool)), (QWidget*)this->m_tabWindow, SLOT(setVisible(bool)));

  //-----------------------------------------------

  action = m_mnuView->addAction("Toggle &advanced mode", this, SLOT(toggleAdvanced()), tr("ctrl+x"));
  action->setCheckable(true);
  m_actions[ACTION_TOGGLE_ADVANCED_MODE] = action;

  action = m_mnuView->addAction("Toggle &debug mode", this, SLOT(toggleDebugMode()), tr("ctrl+d"));
  action->setCheckable(true);
  m_actions[ACTION_TOGGLE_DEBUG_MODE] = action;

  //-----------------------------------------------
  //-----------------------------------------------

  action = m_mnuHelp->addAction("&Help", this, SLOT(showHelp()), tr("F1"));
  m_actions[ACTION_HELP] = action;

  m_mnuView->addSeparator();

  action = m_mnuHelp->addAction("&COOLFluiD Website", this, SLOT(goToWebSite()));
  m_actions[ACTION_GOTO_WEBSITE] = action;

  action = m_mnuHelp->addAction("&COOLFluiD Wiki Page", this, SLOT(goToWebSite()));
  m_actions[ACTION_GOTO_WIKI] = action;

  m_mnuView->addSeparator();

  action = m_mnuHelp->addAction("&About COOLFluiD", m_aboutCFDialog, SLOT(exec()));
  m_actions[ACTION_ABOUT_COOLFLUID] = action;

  action = m_mnuHelp->addAction("&About Qt", qApp, SLOT(aboutQt()));
  m_actions[ACTION_ABOUT_QT] = action;

  //----------------------------------------------------
  //----------------------------------------------------

  this->menuBar()->addMenu(m_mnuFile);
  this->menuBar()->addMenu(m_mnuView);
  this->menuBar()->addMenu(m_mnuHelp);
}

////////////////////////////////////////////////////////////////////////////

void MainWindow::setFileOpen(bool fileOpen)
{
  m_mnuSaveFile->setEnabled(fileOpen);
  m_centralPanel->setVisible(fileOpen);

//  m_treeView->setVisible(fileOpen);
}

////////////////////////////////////////////////////////////////////////////

int MainWindow::confirmClose()
{
  int answer = CLOSE_DISC;
  QMessageBox discBox(this);
  QPushButton * btDisc = nullptr;
  QPushButton * btCancel = nullptr;
  QPushButton * btShutServer = nullptr;

  if(m_treeView->tryToCommit())
  {
    btDisc = discBox.addButton("Disconnect", QMessageBox::NoRole);
    btCancel = discBox.addButton(QMessageBox::Cancel);
    btShutServer = discBox.addButton("Shutdown server", QMessageBox::YesRole);

    discBox.setWindowTitle("Confirmation");
    discBox.setText("You are about to quit this application.");
    discBox.setInformativeText("What do you want to do ?");

    // show the message box
    if(ThreadManager::instance().network().isConnected())
    {
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
    }
  }
  else
    answer = CLOSE_CANCEL;

  return answer;
}

////////////////////////////////////////////////////////////////////////////

void MainWindow::showError(const QString & errorMessage)
{
  QMessageBox::critical(this, "Error", errorMessage);
}

/****************************************************************************

 PROTECTED METHOD

 ****************************************************************************/

void MainWindow::closeEvent(QCloseEvent * event)
{
  int answer = confirmClose();

  if( answer == CLOSE_CANCEL )
    event->ignore();
  else
  {
    event->accept();
    ThreadManager::instance().network().disconnectFromServer( answer == CLOSE_SHUTDOWN );
  }

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
//  NCore::globalCore()->disconnectFromServer(false);
  qApp->exit(0);
}

////////////////////////////////////////////////////////////////////////////

void MainWindow::toggleAdvanced()
{
  bool advanced = m_actions[ ACTION_TOGGLE_ADVANCED_MODE ]->isChecked();
  NTree::globalTree()->setAdvancedMode(advanced);
}

////////////////////////////////////////////////////////////////////////////

void MainWindow::toggleDebugMode()
{
  bool debug = m_actions[ ACTION_TOGGLE_DEBUG_MODE ]->isChecked();
  NTree::globalTree()->setDebugModeEnabled(debug);
}

////////////////////////////////////////////////////////////////////////////

void MainWindow::showHelp()
{
  this->showError("There is no help for now!");
}

////////////////////////////////////////////////////////////////////////////

void MainWindow::goToWebSite()
{
  QUrl url;
  if(sender() == m_actions[ACTION_GOTO_WIKI])
    url.setUrl("https://coolfluidsrv.vki.ac.be/redmine/projects/coolfluid3/wiki");
  else
    url.setUrl("http://coolfluidsrv.vki.ac.be/trac/coolfluid");

  QDesktopServices::openUrl(url);
}

////////////////////////////////////////////////////////////////////////////

void MainWindow::newException(const QString & msg)
{
  this->showError(msg);
}

////////////////////////////////////////////////////////////////////////////

void MainWindow::connectToServer()
{
  SignalFrame frame("connect", "", "");
  SignatureDialog dlg(this);

  frame.set_option("Hostname", std::string("localhost"),
                   "Name of the computer that hosts the server.");
  frame.set_option("Port number", CF::Uint(62784),
                   "The port number the server is listening to.");

  if(dlg.show(frame.main_map.content, "Connect to server", true))
  {
    QString hostname = frame.get_option<std::string>("Hostname").c_str();
    quint16 port = frame.get_option<CF::Uint>("Port number");


    ThreadManager::instance().network().connectToHost(hostname, port);
  }
}

////////////////////////////////////////////////////////////////////////////

void MainWindow::disconnectFromServer()
{
  ThreadManager::instance().network().disconnectFromServer(sender() == m_actions[ACTION_SHUTDOWN_SERVER]);
}

////////////////////////////////////////////////////////////////////////////

void MainWindow::connectedToServer()
{
  this->setConnectedState(true);
}

////////////////////////////////////////////////////////////////////////////

void MainWindow::disconnectedFromServer()
{
  this->setConnectedState(false);
  NTree::globalTree()->clearTree();
}

////////////////////////////////////////////////////////////////////////////

void MainWindow::setConnectedState(bool connected)
{
  m_actions[ACTION_CONNECT_TO_SERVER]->setEnabled(!connected);
  m_actions[ACTION_DISCONNECT_FROM_SERVER]->setEnabled(connected);
  m_actions[ACTION_SHUTDOWN_SERVER]->setEnabled(connected);
}

////////////////////////////////////////////////////////////////////////////

void MainWindow::saveFileLocally()
{
  QFileDialog dlg;

  dlg.setAcceptMode(QFileDialog::AcceptSave);
  dlg.exec();
}

////////////////////////////////////////////////////////////////////////////

void MainWindow::saveFileRemotely()
{
//  NRemoteSave::Ptr flg;

//  flg->show();
}

////////////////////////////////////////////////////////////////////////////

void MainWindow::openFileLocally()
{
  QFileDialog dlg;

  dlg.setAcceptMode(QFileDialog::AcceptOpen);
  dlg.exec();
}

////////////////////////////////////////////////////////////////////////////

void MainWindow::openFileRemotely()
{
  NRemoteOpen::Ptr rop = NRemoteOpen::create();
  rop->show();
}

////////////////////////////////////////////////////////////////////////////

void MainWindow::newLogMessage(const QString & message, CF::GUI::UICommon::LogMessage::Type type)
{
  m_logFile << message << '\n';
}

////////////////////////////////////////////////////////////////////////////

void MainWindow::tabClicked(int num)
{
  if(m_tabWindow->currentWidget() == m_propertyView)
    m_propertyView->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
}

////////////////////////////////////////////////////////////////////////////

void MainWindow::currentIndexChanged(const QModelIndex & newIndex, const QModelIndex & oldIndex)
{
  QString text = "<b>%1</b><br><br>%2";
  QMap<QString, QString> data;

  NTree::globalTree()->listNodeProperties(newIndex, data);

  text = text.arg(data["brief"]).arg(data["description"]);
  m_labDescription->setText(text.replace("\n","<br>"));
}

//////////////////////////////////////////////////////////////////////////////

} // Graphics
} // GUI
} // CF
