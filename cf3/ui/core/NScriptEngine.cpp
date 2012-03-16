// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#include "NScriptEngine.hpp"

//////////////////////////////////////////////////////////////////////////////

#include "common/XML/SignalOptions.hpp"
#include "ui/core/NetworkQueue.hpp"
#include "ui/uicommon/ComponentNames.hpp"
#include "ui/core/NLog.hpp"
#include "common/Log.hpp"
#include "common/URI.hpp"

#include "ui/core/TreeThread.hpp"
#include "ui/core/ThreadManager.hpp"
#include "common/Signal.hpp"

#include "common/XML/Protocol.hpp"
#include "common/XML/SignalOptions.hpp"

#include "ui/graphics/PythonConsole.hpp"

#include <QStringList>

namespace cf3 {

namespace ui {
namespace core {

/////////////////////////////////////////////////////////////////////////////

NScriptEngine::NScriptEngine():CNode(CLIENT_SCRIPT_ENGINE,"NScriptEngine",CNode::LOCAL_NODE) {
  regist_signal("output")
      .description("Output of the python console")
      .pretty_name("").connect(boost::bind(&NScriptEngine::signal_output, this, _1));
  regist_signal("documentation")
      .description("Documentation about a python expression")
      .pretty_name("").connect(boost::bind(&NScriptEngine::signal_documentation, this, _1));
  regist_signal("completion")
      .description("Return the avalaibles keywords in Python")
      .pretty_name("").connect(boost::bind(&NScriptEngine::signal_completion, this, _1));
  regist_signal("debug_trace")
      .description("Debugging")
      .pretty_name("").connect(boost::bind(&NScriptEngine::signal_debug_trace, this, _1));
}

QString NScriptEngine::tool_tip() const {
  return this->component_type();
}

Handle< NScriptEngine > NScriptEngine::global() {
  static Handle< NScriptEngine > scr = ThreadManager::instance().tree().root_child<NScriptEngine>(CLIENT_SCRIPT_ENGINE);
  cf3_assert( is_not_null(scr.get()) );
  return scr;
}

void NScriptEngine::signal_output(common::SignalArgs & node){
  SignalOptions options(node);
  std::string message = options.value<std::string>("text");
  emit new_output(message.c_str());
}

void NScriptEngine::signal_documentation(common::SignalArgs & node){
  SignalOptions options(node);
  std::string documentation = options.value<std::string>("text");
  emit documentation_received(QString(documentation.c_str()));
}


void NScriptEngine::signal_completion(common::SignalArgs & node){
  SignalOptions options(node);
  QStringList add=convertsStdVectorToQStringList(options.array<std::string>("add"));
  QStringList sub=convertsStdVectorToQStringList(options.array<std::string>("sub"));
  emit completion_list_received(add,sub);
}

void NScriptEngine::signal_debug_trace(common::SignalArgs & node){
  SignalOptions options(node);
  emit debug_trace_received(options.value<int>("fragment"),options.value<int>("line"));
}

//////////////////////////////////////////////////////////////////////////////

void NScriptEngine::execute_line( const QString & line , int fragment_number){
  const common::URI script_engine_path("//Tools/Python/ScriptEngine", common::URI::Scheme::CPATH);
  SignalOptions options;
  QString repl=QString(line);
  repl.replace(QString("\t"),QString(";"));
  repl.replace(QString("\n"),QString("?"));
  options.add_option("script", repl.toStdString());
  options.add_option("fragment",fragment_number);
  SignalFrame frame = options.create_frame("execute_script", uri(), script_engine_path);
  NetworkQueue::global()->send( frame, NetworkQueue::IMMEDIATE );
}

//////////////////////////////////////////////////////////////////////////////

void NScriptEngine::emit_debug_command(debug_command command, int fragment, int line){
  const common::URI script_engine_path("//Tools/Python/ScriptEngine", common::URI::Scheme::CPATH);
  SignalOptions options;
  options.add_option("command", static_cast<int>(command));
  if (command == TOGGLE_BREAK_POINT){
    options.add_option("fragment", fragment);
    options.add_option("line", line);
  }
  SignalFrame frame = options.create_frame("change_debug_state", uri(), script_engine_path);
  NetworkQueue::global()->send( frame, NetworkQueue::IMMEDIATE );
}

///////////////////////////////////////////////////////////////////////////

void NScriptEngine::get_completion_list(){
  const common::URI script_engine_path("//Tools/Python/ScriptEngine", common::URI::Scheme::CPATH);
  SignalOptions options;
  SignalFrame frame = options.create_frame("get_completion", uri(), script_engine_path);
  NetworkQueue::global()->send( frame, NetworkQueue::IMMEDIATE );
}

///////////////////////////////////////////////////////////////////////////

void NScriptEngine::request_documentation(QString &doc){
  const common::URI script_engine_path("//Tools/Python/ScriptEngine", common::URI::Scheme::CPATH);
  SignalOptions options;
  options.add_option("expression", doc.toStdString());
  SignalFrame frame = options.create_frame("get_documentation", uri(), script_engine_path);
  NetworkQueue::global()->send( frame, NetworkQueue::IMMEDIATE );
}

///////////////////////////////////////////////////////////////////////////

QStringList NScriptEngine::convertsStdVectorToQStringList(std::vector<std::string> vector){
  std::vector<std::string>::const_iterator itt=vector.begin();
  QStringList list;
  for (;itt!=vector.end();itt++){
    list.push_back(QString(itt->c_str()));
  }
  return list;
}

void NScriptEngine::append_command_to_python_console(std::string & command){
  ui::graphics::PythonConsole::main_console->execute_code(QString(command.c_str()),true);
}

} // Core
} // ui
} // cf3

/////////////////////////////////////////////////////////////////////////////
