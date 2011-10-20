// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/python.hpp>

#include <coolfluid-paths.hpp>

#include "common/BoostFilesystem.hpp"
#include "common/CBuilder.hpp"
#include "common/LibCommon.hpp"
#include "common/Log.hpp"
#include "common/OptionT.hpp"
#include "common/OSystem.hpp"
#include "common/Signal.hpp"

#include "common/XML/Protocol.hpp"
#include "common/XML/SignalOptions.hpp"

#include "Python/ScriptEngine.hpp"

namespace cf3 {
namespace Python {

using namespace common;
using namespace common::XML;

////////////////////////////////////////////////////////////////////////////////////////////

ComponentBuilder < ScriptEngine, Component, LibPython > ScriptEngine_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

ScriptEngine::ScriptEngine ( const std::string& name ) : Component ( name )
{
  if(!Py_IsInitialized())
  {
    const boost::filesystem::path dso_dir = boost::filesystem::path(CF3_BUILD_DIR) / boost::filesystem::path("dso");
    OSystem::setenv("PYTHONPATH", dso_dir.string());
    Py_Initialize();
  }
  
  regist_signal( "execute_script" )
    ->connect( boost::bind( &ScriptEngine::signal_execute_script, this, _1 ) )
    ->description("Execute a python script, passed as string")
    ->pretty_name("Execute Script")
    ->signature( boost::bind( &ScriptEngine::signature_execute_script, this, _1 ) );
}


ScriptEngine::~ScriptEngine()
{
}

////////////////////////////////////////////////////////////////////////////////////////////

void ScriptEngine::execute_script(std::string script)
{
  //TODO: Redirect output
  //PySys_SetObject(const_cast<char*>("stdout"), boost::python::object(*stdout_signal).ptr());
  //PySys_SetObject(const_cast<char*>("stderr"), boost::python::object(*stderr_signal).ptr());


  // Remove dos line endings
  script.erase(std::remove(script.begin(), script.end(), '\r'), script.end());

  boost::python::handle<> main_module(boost::python::borrowed( PyImport_AddModule("__main__") ));
  boost::python::handle<> main_namespace(boost::python::borrowed( PyModule_GetDict(main_module.get()) ));

  CFdebug << "Running script:\n" << script << CFendl;

  boost::python::handle<> result;
  try
  {
    result = boost::python::handle<>(PyRun_String(const_cast<char*>(script.c_str()), Py_file_input, main_namespace.get(), main_namespace.get()));
  }
  catch(...)
  {
    CFerror << "Error executing python script" << CFendl;
  }
  if(!result.get())
  {
    PyErr_Print();
  }
}

void ScriptEngine::signal_execute_script(SignalArgs& node)
{
  SignalOptions options( node );
  execute_script(options.option("script").value<std::string>());
}

void ScriptEngine::signature_execute_script(SignalArgs& node)
{
  SignalOptions options( node );
  
  options.add_option< OptionT<std::string> >( "script", std::string() )
    ->description("Script to execute")
    ->pretty_name("Script");
}


////////////////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3
