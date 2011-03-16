// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Client_UI_MainWindow_hpp
#define CF_GUI_Client_UI_MainWindow_hpp

////////////////////////////////////////////////////////////////////////////////

#include <QMainWindow>
#include <QMap>
#include <QList>
#include <QMenu>
#include <QTextStream>

#include "GUI/Network/LogMessage.hpp"

#include "GUI/Client/UI/LibClientUI.hpp"

class QLabel;
class QModelIndex;
class QScrollArea;
class QSplitter;
class QTabWidget;
class QTextEdit;
class QTableView;

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {

namespace ClientCore {
  class PropertyModel;
}

namespace ClientUI {

////////////////////////////////////////////////////////////////////////////////

  class Graph;
  class LoggingList;
  class CentralPanel;
  class TreeView;
  class AboutCFDialog;
  class TreeBrowser;

////////////////////////////////////////////////////////////////////////////////

  /// @brief Client main window.

  /// @author Quentin Gasper.
  class ClientUI_API MainWindow : public QMainWindow
  {
    Q_OBJECT

    enum MainWinActions
    {
      ACTION_CONNECT_TO_SERVER,

      ACTION_DISCONNECT_FROM_SERVER,

      ACTION_SHUTDOWN_SERVER,

      ACTION_UPDATE_TREE,

      ACTION_OPEN_REMOTELY,

      ACTION_OPEN_LOCALLY,

      ACTION_SAVE_REMOTELY,

      ACTION_SAVE_LOCALLY,

      ACTION_TOGGLE_DEBUG_MODE,

      ACTION_TOGGLE_ADVANCED_MODE,

      ACTION_TOGGLE_INFO_PANE,

      ACTION_CLEAR_LOG,

      ACTION_HELP,

      ACTION_ABOUT_COOLFLUID,

      ACTION_ABOUT_QT
    };

  protected:
    /// @brief Overrides @c QWidget::closeEvent().

    /// This method is called when the user closes the window. If a network
    /// communication is active, he is prompt to confirm his action.
    /// @param event Close event to manage the window closing.
    virtual void closeEvent(QCloseEvent * event);

  public:

    /// @brief Constructor.

    /// Builds all components used by the window. After the constructor, the
    /// window is visible and in a "@e Not connected" state.
    MainWindow();

    /// @brief Destructor.

    /// Frees the allocated memory.
    ~MainWindow();

  private slots:

    /// @brief Slot called when the user wants to quit the application.

    /// The client disconnects form the server and exits immediately.
    void quit();

    /// @brief Slot called when the user wants to to toggle
    /// basic/advanced mode.
    void toggleAdvanced();

    void toggleDebugMode();

    void showHelp();

    void newException(const QString &);

    void connectToServer();

    void disconnectFromServer();

    void connectedToServer();

    void disconnectedFromServer();

    void saveFileLocally();

    void saveFileRemotely();

    void openFileLocally();

    void openFileRemotely();

    void newLogMessage(const QString & message, CF::GUI::Network::LogMessage::Type type);

    void tabClicked(int num);

    void currentIndexChanged(const QModelIndex & newIndex, const QModelIndex & oldIndex);

  private:

    /// @brief Indicates that the user wants to disconnect from the server.

    /// Used when the user does "Disconnect", "Quit", or closes the window.
    static const int CLOSE_DISC = 0;

    /// @brief Indicates that the user wants to shutdown the server.

    /// Used when the user does "Disconnect", "Quit", or closes the window.
    static const int CLOSE_SHUTDOWN = 1;

    /// @brief Indicates that the user wants cancel his request to close the
    /// connection/window.

    /// Used when the user does "Disconnect", "Quit", or closes the window.
    static const int CLOSE_CANCEL = 2;

    /// @brief The Client that displays the model.
    TreeView * m_treeView;

    AboutCFDialog * m_aboutCFDialog;

    /// @brief Panel used to display and modify options for a selected
    /// object.
    CentralPanel * m_centralPanel;

    /// @brief Hashmap containing all available actions for menu m_items.

    /// The key is a number defined by one of the constant integer attributes
    /// of this class. The value is the action corresponding to this number.
    QMap<MainWindow::MainWinActions, QAction *> m_actions;

    /// @brief "File" menu
    QMenu * m_mnuFile;

    /// @brief "View" menu
    QMenu * m_mnuView;

    QMenu * m_mnuHelp;

    /// @brief "Open file" sub-menu
    QMenu * m_mnuOpenFile;

    /// @brief "Save file" sub-menu
    QMenu * m_mnuSaveFile;

    /// @brief Text area displaying the log messages.
    LoggingList * m_logList;

    /// @brief Splitter used to allow user to resize the Client.
    QSplitter * m_splitter;

    QTextStream m_logFile;

    QSplitter * m_centralSplitter;

    QTabWidget * m_tabWindow;

    QTabWidget * m_centralTab;

    QTableView * m_propertyView;

    ClientCore::PropertyModel * m_propertyModel;

    QLabel * m_labDescription;

    QScrollArea * m_scrollDescription;

    TreeBrowser * m_treeBrowser;

    Graph * m_graphXYPlot;

    /// @brief Creates actions and menus
    void buildMenus();

    /// @brief Sets the client to a <i>file open</i> or a <i>file
    /// not open</i> state by enabling or disabling certain m_options.

    /// @param fileOpen If @c true, the client is set to a <i>file open</i>
    /// running state, otherwise it is set to a <i>file not open</i> state.
    void setFileOpen(bool fileOpen);

    /// @brief Asks to the user to confirm his request to close the
    /// connection or window.

    /// @return Returns @c CLOSE_DISC if the user just wants to disconnect from
    /// the server, @c CLOSE_SHUTDOWN if the user wants to shutdown the server
    /// or @c SHUT_CANCEL if the user wants to cancel his action.
    int confirmClose();

    /// @brief Shows an error message in a message box.

    /// @param errorMessage Error message to show.
    void showError(const QString & errorMessage);

    void setConnectedState(bool connected);

  }; // class MainWindow

//////////////////////////////////////////////////////////////////////////////

} // ClientUI
} // GUI
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_UI_MainWindow_hpp
