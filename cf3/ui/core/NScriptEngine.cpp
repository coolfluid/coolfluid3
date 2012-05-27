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
  connected=false;
  regist_signal("documentation")
      .hidden(true)
      .pretty_name("documentation signal")
      .connect(boost::bind(&NScriptEngine::signal_documentation, this, _1));
  regist_signal("completion")
      .hidden(true)
      .pretty_name("completion signal")
      .connect(boost::bind(&NScriptEngine::signal_completion, this, _1));
  regist_signal("debug_trace")
      .hidden(true)
      .pretty_name("debug trace signal")
      .connect(boost::bind(&NScriptEngine::signal_debug_trace, this, _1));
  regist_signal("execute_script")// reply signal
      .hidden(true)
      .pretty_name("execute script reply")
      .connect(boost::bind(&NScriptEngine::signal_execute_script_reply, this, _1));
}

///////////////////////////////////////////////////////////////////////////

void NScriptEngine::append_command_to_python_console(std::string & command){
  emit execute_code_request(QString(command.c_str()));
}

///////////////////////////////////////////////////////////////////////////

void NScriptEngine::append_false_command_to_python_console(std::string & command){
  emit append_false_command_request(QString(command.c_str()));
}

///////////////////////////////////////////////////////////////////////////

QString NScriptEngine::tool_tip() const {
  return this->component_type();
}

///////////////////////////////////////////////////////////////////////////

Handle< NScriptEngine > NScriptEngine::global() {
  static Handle< NScriptEngine > scr = ThreadManager::instance().tree().root_child<NScriptEngine>(CLIENT_SCRIPT_ENGINE);
  cf3_assert( is_not_null(scr.get()) );
  return scr;
}

///////////////////////////////////////////////////////////////////////////

void NScriptEngine::signal_documentation(common::SignalArgs & node){
  SignalOptions options(node);
  std::string documentation = options.value<std::string>("text");
  emit documentation_received(QString(documentation.c_str()));
}

///////////////////////////////////////////////////////////////////////////

void NScriptEngine::signal_completion(common::SignalArgs & node){
  SignalOptions options(node);
  QStringList add=std_vector_to_QStringList(options.array<std::string>("add"));
  QStringList sub=std_vector_to_QStringList(options.array<std::string>("sub"));
  emit completion_list_received(add,sub);
}

///////////////////////////////////////////////////////////////////////////

void NScriptEngine::signal_debug_trace(common::SignalArgs & node){
  SignalOptions options(node);
  emit debug_trace_received(options.value<int>("fragment"),options.value<int>("line"));
}

///////////////////////////////////////////////////////////////////////////

void NScriptEngine::signal_execute_script_reply(common::SignalArgs & node){
  SignalOptions options(node);
  emit change_fragment_request(options.value<int>("fragment"),options.value<int>("new_fragment"));
}

//////////////////////////////////////////////////////////////////////////////

void NScriptEngine::execute_line( const QString & line , int fragment_number, QVector<int> break_points){
  if (connected){
    const common::URI script_engine_path("//Tools/Python/ScriptEngine", common::URI::Scheme::CPATH);
    SignalOptions options;
    QString repl=QString(line);
    repl.replace(QString("\t"),QString(";"));
    repl.replace(QString("\n"),QString("?"));
    options.add("script", repl.toStdString());
    options.add("fragment",fragment_number);
    options.add("breakpoints",break_points.toStdVector());
    SignalFrame frame = options.create_frame("execute_script", uri(), script_engine_path);
    NetworkQueue::global()->send( frame, NetworkQueue::IMMEDIATE );
  }
}

//////////////////////////////////////////////////////////////////////////////

void NScriptEngine::emit_debug_command(debug_command command, int fragment, int line){
  if (connected){
    const common::URI script_engine_path("//Tools/Python/ScriptEngine", common::URI::Scheme::CPATH);
    SignalOptions options;
    options.add("command", static_cast<int>(command));
    if (command == TOGGLE_BREAK_POINT){
      options.add("fragment", fragment);
      options.add("line", line);
    }
    SignalFrame frame = options.create_frame("change_debug_state", uri(), script_engine_path);
    NetworkQueue::global()->send( frame, NetworkQueue::IMMEDIATE );
  }
}

///////////////////////////////////////////////////////////////////////////

void NScriptEngine::client_connected(){
  connected=true;
  const common::URI script_engine_path("//Tools/Python/ScriptEngine", common::URI::Scheme::CPATH);
  SignalOptions options;
  SignalFrame frame = options.create_frame("get_completion", uri(), script_engine_path);
  NetworkQueue::global()->send( frame, NetworkQueue::IMMEDIATE );
}

///////////////////////////////////////////////////////////////////////////

void NScriptEngine::request_documentation(QString &doc){
  if (connected){
    const common::URI script_engine_path("//Tools/Python/ScriptEngine", common::URI::Scheme::CPATH);
    SignalOptions options;
    options.add("expression", doc.toStdString());
    SignalFrame frame = options.create_frame("get_documentation", uri(), script_engine_path);
    NetworkQueue::global()->send( frame, NetworkQueue::IMMEDIATE );
  }
}

///////////////////////////////////////////////////////////////////////////

QStringList NScriptEngine::std_vector_to_QStringList(std::vector<std::string> vector){
  std::vector<std::string>::const_iterator itt=vector.begin();
  QStringList list;
  for (;itt!=vector.end();itt++){
    list.push_back(QString(itt->c_str()));
  }
  return list;
}

} // Core
} // ui
} // cf3

/////////////////////////////////////////////////////////////////////////////
