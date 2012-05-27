// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
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
#include <QPushButton>
#include <QScrollArea>
#include <QSplitter>
#include <QTableView>
#include <QUrl>

#include <boost/program_options.hpp>

#include "common/Signal.hpp"
#include "common/XML/SignalFrame.hpp"

#include "Tools/Shell/BasicCommands.hpp"
#include "Tools/Shell/Interpreter.hpp"

#include "ui/core/TreeThread.hpp"
#include "ui/core/NetworkQueue.hpp"
#include "ui/core/NetworkThread.hpp"
#include "ui/core/NLog.hpp"
#include "ui/core/NTree.hpp"
#include "ui/core/PropertyModel.hpp"
#include "ui/core/RemoteDispatcher.hpp"
#include "ui/core/SSHTunnel.hpp"
#include "ui/core/ThreadManager.hpp"

#include "ui/graphics/AboutCFDialog.hpp"
#include "ui/graphics/BrowserDialog.hpp"
#include "ui/graphics/LoggingList.hpp"
#include "ui/graphics/CentralPanel.hpp"
#include "ui/graphics/PythonCodeEditor.hpp"
#include "ui/graphics/PythonConsole.hpp"
#include "ui/graphics/SignatureDialog.hpp"
#include "ui/graphics/TabBuilder.hpp"
#include "ui/graphics/TreeBrowser.hpp"
#include "ui/graphics/TreeView.hpp"
#include "ui/graphics/RemoteFileCopy.hpp"

#include "ui/uicommon/ComponentNames.hpp"

#include "ui/graphics/MainWindow.hpp"

#define WORKSPACE_FILE QDir::homePath() + "/CF3_workspace.xml"


using namespace cf3::common;
using namespace cf3::common::XML;
using namespace cf3::Tools::Shell;
using namespace cf3::ui::core;
using namespace cf3::ui::uiCommon;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace graphics {

////////////////////////////////////////////////////////////////////////////

MainWindow::MainWindow()
  : m_log_file(new QFile(QString("coolfluid_client-") +
            QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + ".log"))
{
  this->setWindowTitle("COOLFluiD client");

  this->statusBar(); // has to be called to create the status bar

  m_log_file.device()->open(QIODevice::WriteOnly);

  // create the components
  m_central_panel = new CentralPanel(this);
  m_tree_view = new TreeView(m_central_panel, this);
  m_splitter = new QSplitter(/*Qt::Horizontal, this*/);
  m_central_splitter = new QSplitter(Qt::Vertical/*, this*/);
  m_python_tab_splitter = new QSplitter(this);
  m_tab_window = new QTabWidget(m_central_panel);
  m_log_list = new LoggingList(m_tab_window);
  m_property_model = new PropertyModel();
  m_property_view = new QTableView(m_tab_window);
  m_lab_description = new QLabel(m_tab_window);
  m_python_console = new PythonConsole(m_tab_window, this);
  m_tree_browser = new TreeBrowser(m_tree_view, this);
  m_scroll_description = new QScrollArea(this);
  m_about_cf_dialog = new AboutCFDialog(this);

  boost::program_options::options_description desc;

  desc.add( BasicCommands::description() );

  m_script_runner = new Interpreter(desc);

  // configure components

  m_property_view->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
  m_property_view->horizontalHeader()->setStretchLastSection(true);

  m_property_view->setModel(m_property_model);

  m_lab_description->setTextFormat(Qt::RichText);
  m_lab_description->setAlignment(Qt::AlignTop);
  m_lab_description->setWordWrap(true);

  m_scroll_description->setWidgetResizable(true);
  m_scroll_description->setWidget(m_lab_description);

  m_tab_window->addTab(m_log_list, "Log");
  m_tab_window->addTab(m_property_view, "Properties");
  m_tab_window->addTab(m_scroll_description, "Description");
  m_tab_window->addTab(m_python_console,"Python Console");

  TabBuilder::instance()->addTab(m_central_panel, "Options");


  // add the components to the splitter
  m_splitter->addWidget(m_tree_browser);

  QWidget* m_python_tab_widget=new QWidget(this);

  m_central_splitter->addWidget(TabBuilder::instance());
  m_central_splitter->addWidget(m_tab_window);

  m_python_tab_splitter->addWidget(m_central_splitter);
  m_python_tab_splitter->addWidget(m_python_tab_widget);

  m_python_tab_splitter->setStretchFactor(0,10);

  m_python_console->create_python_area(m_python_tab_widget);

  m_splitter->addWidget(m_python_tab_splitter);

  m_central_splitter->setStretchFactor(0, 10);
  m_splitter->setStretchFactor(1, 10);

  m_splitter->setHandleWidth(0);

  this->setCentralWidget(m_splitter);

  this->build_menus();

  NRoot* root = ThreadManager::instance().tree().root().get();

  connect(NLog::global().get(), SIGNAL(new_exception(QString)),
          this, SLOT(new_exception(QString)));

  connect(NLog::global().get(),
          SIGNAL(new_message(QString, uiCommon::LogMessage::Type)),
          this, SLOT(new_log_message(QString, uiCommon::LogMessage::Type)));

  connect(root, SIGNAL(connected()), this, SLOT(network_connected()));

  ThreadManager::instance().network().signal( "network_disconnected" )
      ->connect( boost::bind( &MainWindow::network_disconnected, this, _1));

//  connect(&ThreadManager::instance().network(), SIGNAL(network_disconnected(bool)),
//          this, SLOT(network_disconnected(bool)));

  connect(NTree::global().get(),
          SIGNAL(current_index_changed(QModelIndex,QModelIndex)),
          this, SLOT(current_index_changed(QModelIndex,QModelIndex)));

  connect(m_tab_window, SIGNAL(currentChanged(int)), this, SLOT(tab_clicked(int)));

  current_tunnel=NULL;

  this->set_connected_state(false);

  NLog::global()->add_message("Client successfully launched.");
}

///////////////////////////////////////////////////////////////////////////

MainWindow::~MainWindow()
{
  m_log_file.flush();

  delete m_tree_view;
  delete m_central_panel;

  delete m_log_list;
  delete m_mnu_view;
  delete m_mnu_file;
  delete m_mnu_help;
  delete m_about_cf_dialog;

//  m_logFile.close();
}


//========================================================================
//                                PRIVATE METHODS
//========================================================================

void MainWindow::build_menus()
{
  QAction * action;

  m_mnu_file = new QMenu("&File", this);
  m_mnu_open_file = new QMenu("&Open file", this);
  m_mnu_save_file = new QMenu("&Save file", this);
  m_mnu_view = new QMenu("&View", this);
  m_mnu_help = new QMenu("&Help", this);


  action = m_mnu_file->addAction("&Connect to server", this,
                                SLOT(connect_to_server()), tr("ctrl+shift+C"));
  m_actions[ACTION_CONNECT_TO_SERVER] = action;

  //////
#if defined(unix) || defined(__unix__) || defined(__unix)
  action = m_mnu_file->addAction("Create an ssh &tunnel", this,
                                SLOT(create_ssh_tunnel()), tr("ctrl+shift+T"));
  m_actions[ACTION_CREATE_SSH_TUNNEL] = action;

  action = m_mnu_file->addAction("Create a &reverse ssh tunnel", this,
                                SLOT(create_reverse_ssh_tunnel()), tr("ctrl+shift+R"));
  m_actions[ACTION_CREATE_REVERSE_SSH_TUNNEL] = action;
#endif
  //////

  action = m_mnu_file->addAction("&Disconnect from server", this,
                                SLOT(disconnect_from_server()), tr("ctrl+shift+X"));
  m_actions[ACTION_DISCONNECT_FROM_SERVER] = action;

  action = m_mnu_file->addAction("&Shutdown the server", this,
                                SLOT(disconnect_from_server()), tr("ctrl+shift+K"));
  m_actions[ACTION_SHUTDOWN_SERVER] = action;

  m_mnu_file->addSeparator();

  action = m_mnu_file->addAction("Run &script", this,
                                SLOT(run_script()), tr("ctrl+shift+S"));
  m_actions[ACTION_RUN_SCRIPT] = action;

  action = m_mnu_file->addAction("&New python editor", this,
                                SLOT(new_python_script_editor()), tr("ctrl+shift+N"));
  m_actions[ACTION_NEW_PYTHON_EDITOR] = action;

  m_mnu_file->addSeparator();

  //-----------------------------------------------

  action = m_mnu_open_file->addAction("&Locally", this, SLOT(open_file_locally()), tr("ctrl+o"));
  m_actions[ACTION_OPEN_LOCALLY] = action;

  action = m_mnu_open_file->addAction("&Remotely", this, SLOT(open_file_remotely()), tr("ctrl+shift+o"));
  m_actions[ACTION_OPEN_REMOTELY] = action;

  action = m_mnu_open_file->addAction("&Locally", this, SLOT(save_file_locally()), tr("ctrl+s"));
  m_actions[ACTION_SAVE_LOCALLY] = action;

  action = m_mnu_open_file->addAction("&Remotely", this, SLOT(save_file_remotely()), tr("ctrl+shift+s"));
  m_actions[ACTION_SAVE_REMOTELY] = action;

  m_mnu_file->addMenu(m_mnu_open_file);
  m_mnu_file->addMenu(m_mnu_save_file);

  action = m_mnu_file->addAction("New remote &file copy view", this,
                                SLOT(new_remote_file_copy()), tr("ctrl+shift+F"));
  m_actions[ACTION_NEW_REMOTE_FILE_COPY] = action;
  m_mnu_file->addSeparator();

  //-----------------------------------------------

  action = m_mnu_file->addAction("&Update tree", NTree::global().get(),
                                SLOT(update_tree()), tr("ctrl+u") );
  m_actions[ACTION_UPDATE_TREE] = action;

  m_mnu_file->addSeparator();

  //-----------------------------------------------

  action = m_mnu_file->addAction("&Quit", this,
                                SLOT(quit()), tr("ctrl+Q"));
  m_actions[ACTION_QUIT] = action;

  //-----------------------------------------------
  //-----------------------------------------------

  action = m_mnu_view->addAction("&Clear log messages", this->m_log_list , SLOT(clear_log()));
  m_actions[ACTION_CLEAR_LOG] = action;

  m_mnu_view->addSeparator();

  //-----------------------------------------------

  m_mnu_view->addAction("&Find component", m_tree_browser, SLOT(focus_filter()), tr("ctrl+f") );

  action = m_mnu_view->addAction("&Toggle information pane");
  action->setCheckable(true);
  action->setChecked(true);
  action->setShortcut( tr("ctrl+i") );
  m_tab_window->setVisible(true);
  m_actions[ACTION_TOGGLE_INFO_PANE] = action;

  // QTabWidget overrides setVisible(bool) slot to hide/show the widget
  // contained in the tab. Since we want to hide/show the whole pane
  // (including tab bar), we need to cast the object to QWidget.
  connect(action, SIGNAL(toggled(bool)), (QWidget*)this->m_tab_window, SLOT(setVisible(bool)));

  //-----------------------------------------------

  action = m_mnu_view->addAction("Toggle &advanced mode", this, SLOT(toggle_advanced()), tr("ctrl+x"));
  action->setCheckable(true);
  m_actions[ACTION_TOGGLE_ADVANCED_MODE] = action;

  action = m_mnu_view->addAction("Toggle &debug mode", this, SLOT(toggle_debug_mode()), tr("ctrl+d"));
  action->setCheckable(true);
  m_actions[ACTION_TOGGLE_DEBUG_MODE] = action;

  //-----------------------------------------------
  //-----------------------------------------------

  action = m_mnu_help->addAction("&Help", this, SLOT(show_help()), tr("F1"));
  m_actions[ACTION_HELP] = action;

  m_mnu_view->addSeparator();

  action = m_mnu_help->addAction("&COOLFluiD Website", this, SLOT(go_to_web_site()));
  m_actions[ACTION_GOTO_WEBSITE] = action;

  action = m_mnu_help->addAction("&COOLFluiD Wiki Page", this, SLOT(go_to_web_site()));
  m_actions[ACTION_GOTO_WIKI] = action;

  m_mnu_view->addSeparator();

  action = m_mnu_help->addAction("&About COOLFluiD", m_about_cf_dialog, SLOT(exec()));
  m_actions[ACTION_ABOUT_COOLFLuiD] = action;

  action = m_mnu_help->addAction("&About Qt", qApp, SLOT(aboutQt()));
  m_actions[ACTION_ABOUT_QT] = action;

  //----------------------------------------------------
  //----------------------------------------------------

  this->menuBar()->addMenu(m_mnu_file);
  this->menuBar()->addMenu(m_mnu_view);
  this->menuBar()->addMenu(m_mnu_help);
}

////////////////////////////////////////////////////////////////////////////

void MainWindow::set_file_open(bool fileOpen)
{
  m_mnu_save_file->setEnabled(fileOpen);
  m_central_panel->setVisible(fileOpen);

//  m_treeView->setVisible(fileOpen);
}

////////////////////////////////////////////////////////////////////////////

int MainWindow::confirm_close()
{
  int answer = CLOSE_DISC;
  QMessageBox discBox(this);
  QPushButton * btDisc = nullptr;
  QPushButton * btCancel = nullptr;
  QPushButton * btShutServer = nullptr;

  if(m_tree_view->try_commit())
  {
    btDisc = discBox.addButton("Disconnect", QMessageBox::NoRole);
    btCancel = discBox.addButton(QMessageBox::Cancel);
    btShutServer = discBox.addButton("Shutdown server", QMessageBox::YesRole);

    discBox.setWindowTitle("Confirmation");
    discBox.setText("You are about to quit this application.");
    discBox.setInformativeText("What do you want to do ?");

    // show the message box
    if(ThreadManager::instance().network().is_connected())
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

void MainWindow::show_error(const QString & errorMessage)
{
  QMessageBox::critical(this, "Error", errorMessage);
}

/****************************************************************************

 PROTECTED METHOD

 ****************************************************************************/

void MainWindow::closeEvent(QCloseEvent * event)
{
  int answer = confirm_close();

  if( answer == CLOSE_CANCEL )
    event->ignore();
  else
  {
    event->accept();
    ThreadManager::instance().network().disconnect_from_server( answer == CLOSE_SHUTDOWN );
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
  QCloseEvent e;

  closeEvent( &e );

  if( e.isAccepted() )
    qApp->exit(0);
}

////////////////////////////////////////////////////////////////////////////

void MainWindow::toggle_advanced()
{
  bool advanced = m_actions[ ACTION_TOGGLE_ADVANCED_MODE ]->isChecked();
  NTree::global()->set_advanced_mode(advanced);
}

////////////////////////////////////////////////////////////////////////////

void MainWindow::toggle_debug_mode()
{
  bool debug = m_actions[ ACTION_TOGGLE_DEBUG_MODE ]->isChecked();
  NTree::global()->set_debug_mode_enabled(debug);
}

////////////////////////////////////////////////////////////////////////////

void MainWindow::show_help()
{
  this->show_error("There is no help for now!");
}

////////////////////////////////////////////////////////////////////////////

void MainWindow::go_to_web_site()
{
  QUrl url;
  if(sender() == m_actions[ACTION_GOTO_WIKI])
    url.setUrl("https://coolfluidsrv.vki.ac.be/redmine/projects/coolfluid3/wiki");
  else
    url.setUrl("http://coolfluidsrv.vki.ac.be/trac/coolfluid");

  QDesktopServices::openUrl(url);
}

////////////////////////////////////////////////////////////////////////////

void MainWindow::new_exception(const QString & msg)
{
  this->show_error(msg);
}

////////////////////////////////////////////////////////////////////////////

void MainWindow::connect_to_server()
{
  SignalFrame frame("connect", "", "");
  SignalOptions options( frame );
  SignatureDialog dlg( this );

  options.add( "hostname", std::string("localhost") )
      .pretty_name( "Hostname" )
      .description( "Name of the computer that hosts the server.");

  options.add( "port_number",cf3::Uint(62784) )
      .pretty_name( "Port Number" )
      .description( "The port number the server is listening to." );

  options.flush();

  if( dlg.show( options.main_map.content, "Connect to server", true) )
  {
    std::string hostname = options.main_map.get_value<std::string>( "hostname" );
    quint16 port = options.main_map.get_value<cf3::Uint>( "port_number" );

    ThreadManager::instance().network().connect_to_host( hostname, port );
  }
}

////////////////////////////////////////////////////////////////////////////

void MainWindow::create_ssh_tunnel()
{
  SSHTunnel* tunnel=SSHTunnel::simple_tunnel_popup(this);
  if (tunnel){
    current_tunnel=tunnel;
  }
    //ThreadManager::instance().network().connect_to_host( "localhost", gateway_port );
}

////////////////////////////////////////////////////////////////////////////

void MainWindow::create_reverse_ssh_tunnel()
{
  SSHTunnel* tunnel=SSHTunnel::reverse_tunnel_popup(this);
  if (tunnel){
    current_tunnel=tunnel;
  }
    //ThreadManager::instance().network().connect_to_host( "localhost", gateway_port );
}

////////////////////////////////////////////////////////////////////////////

void MainWindow::disconnect_from_server()
{
  ThreadManager::instance().network().disconnect_from_server(sender() == m_actions[ACTION_SHUTDOWN_SERVER]);
}

////////////////////////////////////////////////////////////////////////////

void MainWindow::network_connected()
{
  this->set_connected_state(true);
}

////////////////////////////////////////////////////////////////////////////

void MainWindow::network_disconnected( SignalFrame & )
{
  this->set_connected_state(false);
  NLog::global()->add_message("Disconnected from the server.");
  NTree::global()->clear_tree();
}

////////////////////////////////////////////////////////////////////////////

void MainWindow::set_connected_state(bool connected)
{
  m_actions[ACTION_CONNECT_TO_SERVER]->setEnabled(!connected);
  m_actions[ACTION_DISCONNECT_FROM_SERVER]->setEnabled(connected);
  m_actions[ACTION_SHUTDOWN_SERVER]->setEnabled(connected);
  m_actions[ACTION_RUN_SCRIPT]->setEnabled(connected);
  m_actions[ACTION_NEW_REMOTE_FILE_COPY]->setEnabled(connected);
}

////////////////////////////////////////////////////////////////////////////

void MainWindow::set_running_script_state(bool running)
{
  m_actions[ACTION_RUN_SCRIPT]->setEnabled(running);
}

////////////////////////////////////////////////////////////////////////////

void MainWindow::save_file_locally()
{
  QFileDialog dlg;

  dlg.setAcceptMode(QFileDialog::AcceptSave);
  dlg.exec();
}

////////////////////////////////////////////////////////////////////////////

void MainWindow::save_file_remotely()
{
//  Handle< NRemoteSave > flg;

//  flg->show();
}

////////////////////////////////////////////////////////////////////////////

void MainWindow::open_file_locally()
{
  QFileDialog dlg;

  dlg.setAcceptMode(QFileDialog::AcceptOpen);
  dlg.exec();
}

////////////////////////////////////////////////////////////////////////////

void MainWindow::open_file_remotely()
{
  BrowserDialog dlg(this);
//  Handle< NRemoteOpen > rop = NRemoteOpen::create();
//  rop->show();
  QVariant selected;
  dlg.show(false, selected);
}

////////////////////////////////////////////////////////////////////////////

void MainWindow::run_script()
{
  QFileDialog dlg;

#ifndef Q_WS_MAC
  dlg.setOption(QFileDialog::DontUseNativeDialog);
#endif

  dlg.setAcceptMode(QFileDialog::AcceptOpen);
  dlg.setNameFilters( QStringList() << "COOLFluiD scripts (*.cfscript *.py)" << "All files (*.*)" );
  dlg.setDirectory( QDir::home() );

  try
  {
    if( dlg.exec() == QFileDialog::Accepted )
      NetworkQueue::global()->execute_script( dlg.selectedFiles().first() );
  }
  catch( Exception & e)
  {
    NLog::global()->add_exception( e.what() );
  }
}

void MainWindow::new_python_script_editor(){
    TabBuilder::instance()->addTab(new PythonCodeEditor(this), "Python editor");
}

void MainWindow::new_remote_file_copy(){
  TabBuilder::instance()->addTab(new RemoteFileCopy(this),"File Copy Thing");
}

PythonCodeEditor* MainWindow::create_new_python_editor(){
  PythonCodeEditor *editor=new PythonCodeEditor(this);
  TabBuilder::instance()->addTab(editor, "Python editor");
  return editor;
}

////////////////////////////////////////////////////////////////////////////

void MainWindow::new_log_message(const QString & message, LogMessage::Type type)
{
  m_log_file << message << '\n';
}

////////////////////////////////////////////////////////////////////////////

void MainWindow::tab_clicked(int num)
{
  if( m_tab_window->currentWidget() == m_property_view )
    m_property_view->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
}

////////////////////////////////////////////////////////////////////////////

void MainWindow::current_index_changed(const QModelIndex & newIndex,
                                     const QModelIndex & oldIndex)
{
  QString text = "<b>%1</b><br><br>%2";
  QMap<QString, QString> data;

  NTree::global()->list_node_properties(newIndex, data);

  text = text.arg(data["brief"]).arg(data["description"]);
  m_lab_description->setText(text.replace("\n","<br>"));
}

//////////////////////////////////////////////////////////////////////////////

void MainWindow::script_finished()
{
  set_running_script_state(false);
}

//////////////////////////////////////////////////////////////////////////////

} // Graphics
} // ui
} // cf3
