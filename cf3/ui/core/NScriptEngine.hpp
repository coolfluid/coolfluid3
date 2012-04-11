// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#ifndef cf3_ui_core_NScriptEngine_hpp
#define cf3_ui_core_NScriptEngine_hpp


//////////////////////////////////////////////////////////////////////////////

#include "ui/core/CNode.hpp"
#include "QVector"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {

namespace ui {
namespace core {

/////////////////////////////////////////////////////////////////////////////

/// @brief Make the link between ScriptEngine server side and the python console client side
/// @author Bolsee Vivian.

class Core_API NScriptEngine :
    public QObject,
    public CNode
{
  Q_OBJECT
public:
  enum debug_command {
    INVALID=-1,
    STOP=0,
    CONTINUE=1,
    LINE_BY_LINE_EXECUTION=2,
    NORMAL_EXECUTION=3,
    BREAK=4,
    TOGGLE_BREAK_POINT=5
  };

  NScriptEngine();


  /// @brief Gives the text to put on a tool tip
  /// @return The name of the class.
  virtual QString tool_tip() const;

  /// @brief Acces to the "singelton" instance
  static Handle<NScriptEngine> global();

  void execute_line( const QString & line, int fragment_number, QVector<int> break_points);

  void emit_debug_command(debug_command command,int fragment=0,int line=0);

  void request_documentation(QString &doc);

  /// @brief Called when the server send new documentation about an expression
  /// @param node Signal node
  void signal_documentation(common::SignalArgs & node);

  /// @brief Called when the server send his completion list
  /// @param node Signal node
  void signal_completion(common::SignalArgs & node);

  /// @brief Called when the server send debugging information
  /// @param node Signal node
  void signal_debug_trace(common::SignalArgs & node);

  /// @brief Called after an execute reply to give the real fragment number
  /// @param node Signal node
  void signal_execute_script_reply(common::SignalArgs & node);

  /// @brief Allow to execute script on the console (not used anymore)
  void append_command_to_python_console(std::string & command);

  /// @brief Allow to append command that are juste display on the console (used in signal conversion)
  void append_false_command_to_python_console(std::string & command);
public slots:
  /// @brief Called when the client is connected to the server, this allow NScriptEngine to send signal to the server.
  void client_connected();

signals:
  /// @brief Signal emitted when the server find new entries in the python scope.
  /// @param word_list scope entry name
  /// @param doc_list entry documentation (__doc__ membre)
  void completion_list_received(const QStringList & add,const QStringList & sub);

  /// @brief Send the debug trace to the python console
  void debug_trace_received(int fragment,int line);

  /// @brief Emitted when a documentation string is sent fromthe server.
  void documentation_received(const QString & documentation);

  /// @brief Emitted when the server compile a fragment and send the real fragment number, for multi client execution
  void change_fragment_request(int old_fragment,int new_fragment);

  /// @brief Send a command to execute to the python console
  void execute_code_request(QString);

  /// @brief Send a command to execute to the python console
  void append_false_command_request(QString);

protected:

  /// Disables the local signals that need to.
  /// @param localSignals Map of local signals. All values are set to true
  /// by default.
  virtual void disable_local_signals(QMap<QString, bool> & localSignals) const {}
private:
  QStringList std_vector_to_QStringList(std::vector<std::string> vector);
  bool connected;
  int vector_int_type;
}; // class NScriptEngine

///////////////////////////////////////////////////////////////////////////

} // Core
} // ui
} // cf3

/////////////////////////////////////////////////////////////////////////////


#endif // cf3_ui_core_NScriptEngine_hpp
