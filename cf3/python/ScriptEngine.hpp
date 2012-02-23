// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Python_ScriptEngine_hpp
#define cf3_Python_ScriptEngine_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Component.hpp"

#include "python/LibPython.hpp"

#include <boost/python/handle.hpp>

#include "common/PE/Manager.hpp"

#include "common/CommonAPI.hpp"

namespace cf3 {
namespace python {

////////////////////////////////////////////////////////////////////////////////

/// @brief Executes python scripts passed as a string
///
/// Exposes an execute_script signal, taking as single argument the string containing the python script to run
/// @author Bart Janssens
class Python_API ScriptEngine : public common::Component {

public: // typedefs

  /// pointer to this type



public: // functions

  /// Contructor
  /// @param name of the component
  ScriptEngine ( const std::string& name );

  /// Virtual destructor
  virtual ~ScriptEngine();

  /// Get the class name
  static std::string type_name () { return "ScriptEngine"; }

  /// Execute the script passed as a string
  void execute_script(std::string script);

  /// Signal to execute a script
  void signal_execute_script(common::SignalArgs& node);

  /// Signal to retrieve the completion list
  void signal_complation(common::SignalArgs& node);

private:
  /// Signature for the execute_script signal
  void signature_execute_script(common::SignalArgs& node);

  void emit_output(std::string output);

  void emit_complation_list();

  void execute_line(std::string script);

  bool check_scope_difference(PyObject* dict);

  void read_dictionary(PyObject* dict,std::vector<std::string> *word_list);

  void read_objects(PyObject* obj,std::string obj_name,std::vector<std::string> *word_list);

  void flush_python_stdout();

  std::string current_instruction;
  boost::python::handle<> global_scope, local_scope;
  bool new_command;
  Handle< common::PE::Manager > m_manager;
  static int python_close;
  std::vector<PyObject*> scope_diff;
}; // ScriptEngine

////////////////////////////////////////////////////////////////////////////////

} // python
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_Python_ScriptEngine_hpp
