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


#include <QStringList>

namespace cf3 {

namespace ui {
namespace core {

/////////////////////////////////////////////////////////////////////////////

NScriptEngine::NScriptEngine():CNode(CLIENT_SCRIPT_ENGINE,"NScriptEngine",CNode::LOCAL_NODE) {
    current_code_fragment=1;//0 is for hidden code
    regist_signal("output")
      .description("Output of the python console")
      .pretty_name("").connect(boost::bind(&NScriptEngine::signal_output, this, _1));
    regist_signal("completion")
      .description("Return the avalaibles keywords in Python")
      .pretty_name("").connect(boost::bind(&NScriptEngine::signal_completion, this, _1));
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

void NScriptEngine::signal_completion(common::SignalArgs & node){
    SignalOptions options(node);
    std::vector<std::string> words;
    std::vector<std::string>::const_iterator it_words;
    words=options.array<std::string>("words");
    QStringList word_list;
    for (it_words=words.begin();it_words!=words.end();++it_words){
        word_list.append(QString(it_words->c_str()));
    }
    emit completion_list_received(word_list);
}


//////////////////////////////////////////////////////////////////////////////

void NScriptEngine::execute_line ( const QString & line ){
    const common::URI script_engine_path("//Tools/Python/ScriptEngine", common::URI::Scheme::CPATH);
    SignalOptions options;
    QString repl=QString(line);
    repl.replace(QString("\t"),QString(";"));
    options.add_option("script", repl.toStdString());
    options.add_option("fragment",current_code_fragment++);
    SignalFrame frame = options.create_frame("execute_script", uri(), script_engine_path);
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

} // Core
} // ui
} // cf3

/////////////////////////////////////////////////////////////////////////////
