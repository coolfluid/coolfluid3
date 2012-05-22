// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Python_ScriptEngine_hpp
#define cf3_Python_ScriptEngine_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/python.hpp>
#include <boost/python/ssize_t.hpp>
#include <boost/python/handle.hpp>
#include <boost/python/object.hpp>
#include <boost/thread/mutex.hpp>

#include <frameobject.h>
#include <vector>
#include <string>

#include "common/Component.hpp"
#include "common/PE/Manager.hpp"
#include "common/CommonAPI.hpp"

#include "python/LibPython.hpp"

namespace cf3 {
namespace python {

////////////////////////////////////////////////////////////////////////////////

/// @brief python tracing function, used to
int python_trace(PyObject *obj, PyFrameObject *frame, int what, PyObject *arg);

/// @brief Threaded c function, used to contain the PyEval_CodeEval
void python_execute_function();

/// @brief Manage a python interpreter
///
/// Exposes an execute_script signal, taking as single argument the string containing the python script to run
/// @author Bolsee Vivian
class Python_API ScriptEngine : public common::Component {
public: // functions

  /// @brief Contructor
  /// @param name of the component
  ScriptEngine ( const std::string& name );

  virtual ~ScriptEngine();

  /// @brief Get the class name
  static std::string type_name () { return "ScriptEngine"; }

  /// @brief Execute the script passed as a string, code fragment is used for traceability
  int execute_script(std::string script,int code_fragment=-1);

  /// @brief Signal to execute a script
  void signal_execute_script(common::SignalArgs& node);

  /// @brief Modify the debug state the first argument is the debug_command named command, if the debug command is
  /// equal to TOGGLE_BREAK_POINT fragment and line argument must be defined (both int)
  void signal_change_debug_state(common::SignalArgs& node);

  /// @brief Signal to retrieve the completion list; no argument
  void signal_completion(common::SignalArgs& node);

  /// @brief Signal to get the documentation of the symbol under the mouse in the gui
  /// takes one argument, expression, must be a string containing the symbol
  void signal_get_documentation(common::SignalArgs& node);

  /// @brief Called by the trace function when a new line is reached
  int new_line_reached(int code_fragment,int line_number);

  /// @brief Verify if the python scope has changed, or if there is some data on the python sys.out
  /// code_fragment allow to redirect correctly the output if this was a documentation request
  void check_python_change();

  /// @brief Called when the interpreter have no more instruction avalaible
  /// used in debug to clear the debug trace at the end of the code
  void no_more_instruction();

private:
  /// @brief debugging commands
  enum debug_command {
    INVALID=-1, /// do nothing
    STOP=0, /// put the interpreter in stop state, he will stop before the next line
    CONTINUE=1, /// continue the execution, if the interpreter was in line by line state he will stay in this state, if he was in stop state he will go back normal execution state
    LINE_BY_LINE_EXECUTION=2, /// the interpreter will stop before each line
    NORMAL_EXECUTION=3,  /// the interpreter execute normaly the python code
    BREAK=4, /// the interpreter go out of the current execution frame
    TOGGLE_BREAK_POINT=5 /// ask to toggle the the break point (used in signal_change_debug_state)
  };
  /// @brief Allow to make a tree representation of the python scope.
  /// the root of the tree is supposed to be a python dictionnary, children are simple python objects
  class PythonDictEntry{
  public:
    PythonDictEntry(){}
    PythonDictEntry(const PythonDictEntry &entry)
      :py_ref(entry.py_ref)
      ,name(entry.name)
      ,value(entry.value)
      ,entry_list(entry.entry_list){}

    PyObject *py_ref;
    std::string name;
    std::string value;
    std::vector<PythonDictEntry> entry_list;
  };

  /// @brief Signature for the execute_script signal
  void signature_execute_script(common::SignalArgs& node);

  /// @brief Emit when the interpreter stop on a line
  void emit_debug_trace(int fragment,int line);

  /// @brief Send the list of known keywords to the client
  void emit_completion_list(std::vector<std::string> *add, std::vector<std::string> *sub);

  /// @brief Send the documentation string to the client, documntation strings are emitted when a fragment 0 code prints output. (so documentation request must be in fragment 0)
  void emit_documentation(std::string doc);

  /// @brief Recursive function that checks for scope modification,
  void check_scope_difference(PythonDictEntry &entry,std::string name,std::vector<std::string> *add, std::vector<std::string> *sub,int rec = 0,bool child=false);

  /// @brief Look if there are no outputs in python sys.out (replaced with a simple storing class), output are then sendend throught CFinfo
  void flush_python_stdout();

  void access_pe_manager();

  std::string current_instruction;
  bool new_command;
  Handle< common::PE::Manager > m_manager;
  PythonDictEntry local_scope_entry;
  PythonDictEntry builtin_scope_entry;
  PythonDictEntry local_frame_scope_entry;
  PythonDictEntry global_frame_scope_entry;
  debug_command interpreter_mode;
  //std::vector<std::vector<bool> >break_points;
  std::vector<std::pair<int,int> >break_points;
  static int python_close;
  int break_fragment;
  int break_line;
  int fragment_generator;
  bool stoped;
  boost::mutex compile_mutex;
}; // ScriptEngine

////////////////////////////////////////////////////////////////////////////////

} // python
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_Python_ScriptEngine_hpp
