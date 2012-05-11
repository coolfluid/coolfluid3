// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_ui_Graphics_MainWindow_hpp
#define cf3_ui_Graphics_MainWindow_hpp

////////////////////////////////////////////////////////////////////////////////

#include <QMainWindow>
#include <QMap>
#include <QList>
#include <QTextStream>

#include "ui/uicommon/LogMessage.hpp"

#include "ui/graphics/LibGraphics.hpp"

class QLabel;
class QModelIndex;
class QScrollArea;
class QSplitter;
class QTabWidget;
class QTextEdit;
class QTableView;

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {

namespace Tools { namespace Shell { class Interpreter; } }

namespace ui {

namespace core {
  class PropertyModel;
  class SSHTunnel;
}

namespace graphics {

////////////////////////////////////////////////////////////////////////////////

  class LoggingList;
  class CentralPanel;
  class TreeView;
  class AboutCFDialog;
  class TreeBrowser;
  class PythonConsole;
  class PythonCodeEditor;

////////////////////////////////////////////////////////////////////////////////

  /// @brief Client main window.

  /// @author Quentin Gasper.
  class Graphics_API MainWindow : public QMainWindow
  {
    Q_OBJECT

    enum MainWinActions
    {
      ACTION_CONNECT_TO_SERVER,

      ACTION_CREATE_SSH_TUNNEL,

      ACTION_CREATE_REVERSE_SSH_TUNNEL,

      ACTION_DISCONNECT_FROM_SERVER,

      ACTION_SHUTDOWN_SERVER,

      ACTION_UPDATE_TREE,

      ACTION_OPEN_REMOTELY,

      ACTION_OPEN_LOCALLY,

      ACTION_SAVE_REMOTELY,

      ACTION_SAVE_LOCALLY,

      ACTION_RUN_SCRIPT,

      ACTION_QUIT,

      ACTION_TOGGLE_DEBUG_MODE,

      ACTION_TOGGLE_ADVANCED_MODE,

      ACTION_TOGGLE_INFO_PANE,

      ACTION_CLEAR_LOG,

      ACTION_HELP,

      ACTION_GOTO_WIKI,

      ACTION_GOTO_WEBSITE,

      ACTION_ABOUT_COOLFLuiD,

      ACTION_ABOUT_QT,

      ACTION_NEW_PYTHON_EDITOR,

      ACTION_NEW_REMOTE_FILE_COPY

    }; // MainWinActions

  public:

    /// @brief Constructor.

    /// Builds all components used by the window. After the constructor, the
    /// window is visible and in a "@e Not connected" state.
    MainWindow();

    /// @brief Destructor.

    /// Frees the allocated memory.
    ~MainWindow();


    /// @brief create a new PythonCodeEditor, called from the PythonConsole
    PythonCodeEditor* create_new_python_editor();

  protected:
    /// @brief Overrides @c QWidget::closeEvent().

    /// This method is called when the user closes the window. If a network
    /// communication is active, he is prompt to confirm his action.
    /// @param event Close event to manage the window closing.
    virtual void closeEvent(QCloseEvent * event);

  private slots:

    /// @brief Slot called when the user wants to quit the application.

    /// The client disconnects form the server and exits immediately.
    void quit();

    /// @brief Slot called when the user wants to to toggle
    /// basic/advanced mode.
    void toggle_advanced();

    void toggle_debug_mode();

    void show_help();

    void go_to_web_site();

    void new_exception(const QString &);

    void connect_to_server();

    void create_ssh_tunnel();

    void create_reverse_ssh_tunnel();

    void disconnect_from_server();

    void network_connected();

    void network_disconnected( common::SignalArgs & args );

    void save_file_locally();

    void save_file_remotely();

    void open_file_locally();

    void open_file_remotely();

    void run_script();

    void new_python_script_editor();

    void new_remote_file_copy();

    void new_log_message(const QString & message, uiCommon::LogMessage::Type type);

    void tab_clicked(int num);

    void current_index_changed(const QModelIndex & newIndex, const QModelIndex & oldIndex);

    void script_finished();

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
    TreeView * m_tree_view;

    AboutCFDialog * m_about_cf_dialog;

    /// @brief Panel used to display and modify options for a selected
    /// object.
    CentralPanel * m_central_panel;

    /// @brief Hashmap containing all available actions for menu m_items.

    /// The key is a number defined by one of the constant integer attributes
    /// of this class. The value is the action corresponding to this number.
    QMap<MainWindow::MainWinActions, QAction *> m_actions;

    /// @brief "File" menu
    QMenu * m_mnu_file;

    /// @brief "View" menu
    QMenu * m_mnu_view;

    QMenu * m_mnu_help;

    /// @brief "Open file" sub-menu
    QMenu * m_mnu_open_file;

    /// @brief "Save file" sub-menu
    QMenu * m_mnu_save_file;

    /// @brief Text area displaying the log messages.
    LoggingList * m_log_list;

    /// @brief Splitter used to allow user to resize the Client.
    QSplitter * m_splitter;

    QTextStream m_log_file;

    QSplitter * m_central_splitter;

    QSplitter * m_python_tab_splitter;

    /// @brief Simple console to execute python command on the server
    PythonConsole * m_python_console;

    QTabWidget * m_tab_window;

    QTableView * m_property_view;

    core::PropertyModel * m_property_model;

    QLabel * m_lab_description;

    QScrollArea * m_scroll_description;

    TreeBrowser * m_tree_browser;

    Tools::Shell::Interpreter * m_script_runner;

    core::SSHTunnel * current_tunnel;

    /// @brief Creates actions and menus
    void build_menus();

    /// @brief Sets the client to a <i>file open</i> or a <i>file
    /// not open</i> state by enabling or disabling certain options().

    /// @param fileOpen If @c true, the client is set to a <i>file open</i>
    /// running state, otherwise it is set to a <i>file not open</i> state.
    void set_file_open(bool fileOpen);

    /// @brief Asks to the user to confirm his request to close the
    /// connection or window.

    /// @return Returns @c CLOSE_DISC if the user just wants to disconnect from
    /// the server, @c CLOSE_SHUTDOWN if the user wants to shutdown the server
    /// or @c SHUT_CANCEL if the user wants to cancel his action.
    int confirm_close();

    /// @brief Shows an error message in a message box.

    /// @param errorMessage Error message to show.
    void show_error(const QString & errorMessage);

    void set_connected_state(bool connected);

    void set_running_script_state(bool running);

  }; // class MainWindow

//////////////////////////////////////////////////////////////////////////////

} // Graphics
} // ui
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_ui_Graphics_MainWindow_hpp
