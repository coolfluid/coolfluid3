// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Python_ScriptEngine_hpp
#define CF_Python_ScriptEngine_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"

#include "Python/LibPython.hpp"

namespace CF {
namespace Python {

////////////////////////////////////////////////////////////////////////////////

/// @brief Executes python scripts passed as a string
///
/// Exposes an execute_script signal, taking as single argument the string containing the python script to run
/// @author Bart Janssens
class Python_API ScriptEngine : public Common::Component {

public: // typedefs

  /// pointer to this type
  typedef boost::shared_ptr<ScriptEngine> Ptr;
  typedef boost::shared_ptr<ScriptEngine const> ConstPtr;

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
  void signal_execute_script(Common::SignalArgs& node);
  
private:
  /// Signature for the execute_script signal
  void signature_execute_script(Common::SignalArgs& node);
}; // ScriptEngine

////////////////////////////////////////////////////////////////////////////////

} // Python
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Python_ScriptEngine_hpp
