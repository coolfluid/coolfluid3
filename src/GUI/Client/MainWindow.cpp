#include <QtCore>
#include <QtGui>
#include <QtXml>

#include "Common/Exception.hpp"
#include "Common/ConfigArgs.hpp"

#include "GUI/Client/TypeAndNameDialog.hpp"
#include "GUI/Client/ConnectionDialog.hpp"
#include "GUI/Client/LoggingList.hpp"
#include "GUI/Client/OptionPanel.hpp"
#include "GUI/Client/OptionType.hpp"
#include "GUI/Client/RemoteFSBrowser.hpp"
#include "GUI/Client/RemoteSaveFile.hpp"
#include "GUI/Client/RemoteOpenFile.hpp"
#include "GUI/Client/SelectFileDialog.hpp"
#include "GUI/Client/StatusModel.hpp"
#include "GUI/Client/StatusPanel.hpp"
#include "GUI/Client/ClientCore.hpp"
#include "GUI/Client/MenuActionInfo.hpp"
#include "GUI/Client/TreeView.hpp"
#include "GUI/Client/NLog.hpp"
#include "GUI/Client/AboutCFDialog.hpp"
#include "GUI/Client/NLog.hpp"
#include "GUI/Client/ClientRoot.hpp"

#include "GUI/Network/HostInfos.hpp"
#include "GUI/Network/ComponentNames.hpp"
#include "GUI/Network/SignalInfo.hpp"

#include "GUI/Client/MainWindow.hpp"

#define connectSig(comm,slotSig) connect(comm, SIGNAL(slotSig), this, SLOT(slotSig));
#define connectKernel(slotSig) connect(m_treeView, SIGNAL(slotSig), \
&ClientCore::instance(), SLOT(slotSig));
#define WORKSPACE_FILE QDir::homePath() + "/CF_workspace.xml"

using namespace CF::GUI::Client;
using namespace CF::GUI::Network;

using namespace CF::Common;
using namespace CF::Common;

MainWindow::MainWindow()
{
  this->setWindowTitle("COOLFluiD client");

//  // create the components
  m_optionPanel = new OptionPanel(this);
  m_logWindow = new QDockWidget("Log Window", this);
  m_treeView = new TreeView(m_optionPanel);
  m_statusModel = new StatusModel(QDomDocument(), this);
  m_statusPanel = new StatusPanel(m_statusModel, this);
  m_logList = new LoggingList(m_logWindow);
  m_splitter = new QSplitter(this);

  m_aboutCFDialog = new AboutCFDialog(this);

//  m_treeView->setModel(ClientRoot::getTree().get());

  // configure components
  m_logWindow->setWidget(m_logList);
  m_logWindow->setFeatures(QDockWidget::NoDockWidgetFeatures |
                           QDockWidget::DockWidgetClosable);

  // add the components to the splitter
  m_splitter->addWidget(m_treeView);

  m_splitter->addWidget(m_optionPanel);
  m_splitter->addWidget(m_statusPanel);
  m_splitter->setStretchFactor(1, 10);

  this->setCentralWidget(m_splitter);
  this->addDockWidget(Qt::BottomDockWidgetArea, m_logWindow);

  this->buildMenus();


  ////////////////////////////////////////////////////////////////////////

  // connect useful signals to slots
//  connectKernel(addNode(const QString &));
//  connectKernel(renameNode(const QDomNode &, const QString &));
//  connectKernel(deleteNode(const QDomNode &));
//  connectKernel(commitChanges(const QDomDocument &));
//  connectKernel(disconnectSimulation(const QModelIndex &, bool));
//  connectKernel(runSimulation(const QModelIndex &));
//  connectKernel(stopSimulation(const QModelIndex &));
//  connectKernel(activateSimulation(const QModelIndex &));
//  connectKernel(deactivateSimulation(const QModelIndex &));
//  connectKernel(updateTree(const QModelIndex &));

//  connectKernel(addLink(const QModelIndex &, const QString &,
//                        const QModelIndex &));

  ////////////////////////////////////////////////////////////////////////

  connectSig(m_treeView, openSimulation(const QModelIndex &));

  connect(ClientRoot::getLog().get(), SIGNAL(newException(const QString &)),
          this, SLOT(newException(const QString &)));

  ClientRoot::getLog()->addMessage("Client successfully launched.");

  ClientCore::instance().setStatusModel(m_statusModel);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MainWindow::~MainWindow()
{
  delete m_treeView;
  delete m_optionPanel;
  delete m_statusPanel;

  delete m_logList;
  delete m_logWindow;
  delete m_mnuView;
  delete m_mnuFile;
  delete m_mnuHelp;
  delete m_aboutCFDialog;
}

 // PRIVATE METHODS

void MainWindow::buildMenus()
{
  MenuActionInfo actionInfo;
  QAction * tmpAtion;

  m_mnuFile = new QMenu("&File", this);
  m_mnuView = new QMenu("&View", this);
  m_mnuHelp = new QMenu("&Help", this);

  actionInfo.initDefaults();
  actionInfo.m_menu = m_mnuView;
  actionInfo.m_text = "&Clear log messages";

  tmpAtion = actionInfo.buildAction(this);

  actionInfo.initDefaults();
  actionInfo.m_menu = m_mnuFile;
  actionInfo.m_text = "&Connect to server";
  actionInfo.m_slot = SLOT(connectToServer());

  m_actions[MainWindow::ACTION_TOGGLE_ADVANCED_MODE] = actionInfo.buildAction(this);

  //-----------------------------------------------


  m_actions[MainWindow::ACTION_CLEAR_LOG] = tmpAtion;
  //  connect(tmpAtion, SIGNAL(triggered()), this->logList, SLOT(clearLog()));

  //-----------------------------------------------

  m_mnuView->addSeparator();

  //-----------------------------------------------

  m_mnuView->addAction(m_logWindow->toggleViewAction());

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

  tmpAtion = actionInfo.buildAction(this);

  m_actions[MainWindow::ACTION_ABOUT_COOLFLUID] = tmpAtion;
  connect(tmpAtion, SIGNAL(triggered()), m_aboutCFDialog, SLOT(exec()));

  //-----------------------------------------------


  actionInfo.initDefaults();
  actionInfo.m_menu = m_mnuHelp;
  actionInfo.m_text = "&About Qt";

  tmpAtion = actionInfo.buildAction(this);

  m_actions[MainWindow::ACTION_ABOUT_COOLFLUID] = tmpAtion;
  connect(tmpAtion, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

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
  m_optionPanel->setVisible(fileOpen);

  m_treeView->setVisible(fileOpen);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int MainWindow::confirmClose()
{
  int answer;
  QMessageBox discBox(this);
  QPushButton * btDisc = CFNULL;
  QPushButton * btCancel = CFNULL;
  QPushButton * btShutServer = CFNULL;

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
    ClientRoot::getLog()->addException(e.what());
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

  /// @todo adapt the following code to work with multiple simulations
  /*************************************************

   CloseConfirmationDialog ccd(this);

   infos = CloseConfirmationInfos();

   this->optionPanel->getModifiedOptions(this->infos.m_commitDetails);

   // if we are still connected to the server
   if(m_connectedToServer)
   ccd.addConfirmation(CLOSE_SHUT_DOWN);

   // if the current configuration has been modified but not saved...
   if(this->configModified)
   ccd.addConfirmation(CLOSE_SAVE_FILE);

   // if modified haven't been committed
   if(this->optionPanel->isModified())
   {
   ccd.addConfirmation(CLOSE_COMMIT);

   // if the configuration hasn't been modified, it will be
   if(!this->configModified)
   ccd.addConfirmation(CLOSE_SAVE_FILE, true);
   }

   if(ccd.show(this->infos))
   {
   this->askedInfos = true;

   if(!this->infos.commitRequested && this->infos.filename.isEmpty())
   {
   //    this->communication->disconnectFromServer(this->infos.shutdownServerRequested);
   event->accept(); // we accept the event: the window will close
   }

   else
   {
   event->ignore(); // we reject the event: the window will not close (for now!)
   this->waitingToSave = !infos.filename.isEmpty();
   this->waitingToExit = true;
   this->shutdownServerOnExit = infos.shutdownServerRequested;

   if(infos.commitRequested)
   this->optionPanel->commit();

   else
   {
   this->saveFromInfos();
   //     this->communication->disconnectFromServer(this->infos.shutdownServerRequested);
   event->accept(); // we accept the event: the window will close
   }
   }
   }

   else
   event->ignore(); // we reject the event: the window will not close

   *********************************************************/
}

/****************************************************************************

 SLOTS

 ****************************************************************************/

void MainWindow::quit()
{
  QApplication::exit(0);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void MainWindow::getTree()
{
  //  this->communication->sendGetTree();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void MainWindow::toggleAdvanced()
{
  bool advanced = m_actions[ ACTION_TOGGLE_ADVANCED_MODE ]->isChecked();
  ClientRoot::getTree()->setAdvancedMode(advanced);
//  m_treeModel->setAdvancedMode(advanced);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void MainWindow::toggleDebugMode()
{
  bool debug = m_actions[ ACTION_TOGGLE_DEBUG_MODE ]->isChecked();
  ClientRoot::getTree()->setDebugModeEnabled(debug);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void MainWindow::showHideStatus()
{
  bool show = m_actions[ ACTION_SHOW_HIDE_STATUS_PANEL ]->isChecked();
  m_statusPanel->setVisible(show);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void MainWindow::errorCommitOnExit()
{
  /// @todo show message : "couldn't commit but saving file anyway"
  //  if(this->configModified && !this->infos.filename.isEmpty())
  //   this->saveFromInfos();

  //  else //if(this->infos.filename.isEmpty())
//  {
//    int answer;
//    QMessageBox errorBox(this);
//    QString message = "Modifications could not be committed. To ensure that "
//    "data will not be lost, you can choose to save these modifications to a "
//    "text file, and redo them manually later.\nClick on \"<i>Show "
//    "Details...</i>\" for further information.";

//    QString details = "Click on \"Yes\" to selet a file, on \"No\" to not "
//    "save the modifications (they will be lost) or on \"Cancel\" to cancel "
//    "the application closing.\nNotes: \n- clicking on \"Yes\" and then "
//    "cancel the file selection is like directly clicking on \"No\"\n- if "
//    "the file cannot be saved, you will be asked to select another file; "
//    "repeatedly until the file is successfuly saved or you cancel\n- clicking "
//    "on \"Yes\" or \"No\" will close the application";

//    errorBox.setTextFormat(Qt::RichText);
//    errorBox.setWindowTitle("Error");
//    errorBox.setText(message);
//    errorBox.setDetailedText(details);
//    errorBox.setInformativeText("Do you want to save modifications?");
//    errorBox.setIcon(QMessageBox::Critical);

//    errorBox.addButton(QMessageBox::Yes);
//    errorBox.addButton(QMessageBox::No);
//    errorBox.addButton(QMessageBox::Cancel);

//    answer = errorBox.exec();

//    switch(answer)
//    {
//      case QMessageBox::Yes:
//      {
//        SelectFileDialog sfd(this);
//        QString filename;
//        bool ok = false;
//        sfd.addFileType("Text", "txt");

//        while(!ok)
//        {
//          filename = sfd.show(QFileDialog::AcceptSave);

//          if(filename.isEmpty())
//            this->quit();

//          else
//          {
//            QFile file(filename);
//            QTextStream out;
//            //      bool saved = false;
//            QString username;
//            QRegExp regex("^USER=");
//            QStringList environment = QProcess::systemEnvironment().filter(regex);

//            if(environment.size() == 1)
//            {
//              username = environment.at(0);
//              username.remove(regex);
//            }

//            if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
//            {
//              QString error = "Could open file '%1' for write access: %2";
//              this->showError(error.arg(filename).arg(file.errorString()));
//            }

//            else
//            {
//              QString date = QDate::currentDate().toString("MM/dd/yyyy");
//              QString time = QTime::currentTime().toString("hh:mm:ss");
//              QString dateTime = QString("%1 at %2").arg(date).arg(time);
//              QString separator = QString("\n").rightJustified(30, '+');

//              out.setDevice(&file);

//              out << "### CF -- GUI Module\n";
//              out << "### This file contains modifications details that could "
//              "not be comitted on Client application exit.\n";
//              out << "### Written by '" << username << "' on " << dateTime << "\n";
//              out << "### Working node path: " << m_optionPanel->getCurrentPath();
//              out << "\n\n";

//              out << m_infos.commitDetails.toString();

//              file.close();

//              this->showMessage(QString("Modification were successfully written to "
//                                        "'%1'").arg(filename));
//              //        this->configModified = false;
//              ok = true;
//            }

//            if(!ok)
//            {
//              int ret;
//              message = "Saving file failed. Do you want select another file ? "
//              "(clicking on \"No\" will directly close the application)";

//              errorBox.setText(message);
//              errorBox.setDetailedText("");
//              errorBox.setIcon(QMessageBox::Critical);
//              errorBox.addButton(QMessageBox::Yes);
//              errorBox.addButton(QMessageBox::No);
//              ret = errorBox.exec();

//              if(ret == QMessageBox::No && !m_infos.filename.isEmpty())
//                this->quit();
//            }
//          }
//        }

//        if(m_infos.filename.isEmpty())
//          this->quit();
//        break;
//      }

//      case QMessageBox::Cancel:
//        //     this->waitingToExit = false;
//        break;
//    }
//  }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void MainWindow::openSimulation(const QModelIndex & index)
{
//  if(!m_treeModel->isSimulationConnected(index))
//    ClientRoot::getLog()->addError("This simulation is not connected.");
//  else
  {
    RemoteOpenFile open(this);

    open.setIncludeFiles(true);
    open.setExtensions(QStringList() << "xml" << "CFcase");
    open.setIncludeNoExtension(false);

    QString file = open.show("");

    if(!file.isEmpty())
      ClientRoot::getLog()->addException("Cannot open a file for now!");

//    if(!file.isEmpty())
//      ClientCore::instance().openFile(index, file);
  }
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
    ClientCore::instance().connectToServer(sshInfo);
  }
}
