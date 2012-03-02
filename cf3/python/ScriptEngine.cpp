// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "python/BoostPython.hpp"

#include <coolfluid-paths.hpp>

#include "common/BoostFilesystem.hpp"
#include "common/Builder.hpp"
#include "common/LibCommon.hpp"
#include "common/Log.hpp"
#include "common/OptionT.hpp"
#include "common/OSystem.hpp"
#include "common/Signal.hpp"

#include "common/XML/Protocol.hpp"
#include "common/XML/SignalOptions.hpp"

#include "python/ScriptEngine.hpp"

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>
#include <boost/python/handle.hpp>
#include <vector>
#include <string>
#include <queue>
#include <stdio.h>
#include <cStringIO.h>
#include <compile.h>
#include <eval.h>


namespace cf3 {
namespace python {

using namespace common;
using namespace common::XML;

////////////////////////////////////////////////////////////////////////////////////////////

ComponentBuilder < ScriptEngine, Component, LibPython > ScriptEngine_Builder;


int ScriptEngine::python_close=0;
ScriptEngine *script_engine=NULL;
boost::python::handle<> global_scope, local_scope;
boost::thread python_thread(python_execute_function);
boost::thread python_stop_thread(python_stop_function);
boost::mutex python_mutex;
std::queue< std::pair<std::string,int> > python_code_queue;
std::pair<std::string,int> python_current_fragment;
bool python_executing_fragment;
bool python_should_wake_up;
bool python_should_be_stopped;
boost::posix_time::millisec wait_a_little(50);

////////////////////////////////////////////////////////////////////////////////////////////

int python_trace(PyObject *obj, PyFrameObject *frame, int what, PyObject *arg){
    if ( what == PyTrace_LINE ){
        boost::python::handle<> frag_number(boost::python::allow_null(PyObject_Str(frame->f_code->co_filename)));
        if (frag_number.get() != NULL){
            int code_fragment=atoi(PyString_AsString(frag_number.get()));
            if (code_fragment > 0){
                script_engine->new_line_reached(code_fragment,frame->f_lineno);
            }
        }
    }
    return 0;
}

void python_execute_function(){
    const boost::filesystem::path dso_dir = boost::filesystem::path(CF3_BUILD_DIR) / boost::filesystem::path("dso");
    OSystem::setenv("PYTHONPATH", dso_dir.string());
    Py_Initialize();
    local_scope = boost::python::handle<>(PyDict_New());
    global_scope = boost::python::handle<>(PyDict_New());
    PyDict_SetItemString (global_scope.get(), "__builtins__", PyEval_GetBuiltins());
    PyEval_SetTrace(python_trace,NULL);
    while(1){
        //CFinfo << "exec_lock" << CFendl;
        python_mutex.lock();//the stop function should have the lock so make this thread waiting at this point
        //CFinfo << "exec_lock_aquired" << CFendl;
        if (python_executing_fragment){
            try{
                std::stringstream ss;
                ss << python_current_fragment.second;
                //CFinfo << script << CFendl;
                boost::python::handle<> src(boost::python::allow_null(Py_CompileString(python_current_fragment.first.c_str(), ss.str().c_str(), Py_single_input)));
               if (NULL != src){//code compiled, we execute it
                    boost::python::handle<> dum(boost::python::allow_null(PyEval_EvalCode((PyCodeObject *)src.get(), global_scope.get(), local_scope.get())));
                }
                PyObject *exc,*val,*trb,*obj;
                char* error;
                PyErr_Fetch(&exc, &val, &trb);
                if (NULL != exc && NULL != val){
                    if (PyArg_ParseTuple (val, "sO", &error, &obj)){
                        CFinfo << error << CFendl;
                        //ScriptEngine::emit_output(std::string(error));
                    }else{
                        CFinfo << "Expression not found in the scope." << CFendl;
                        //ScriptEngine::emit_output(std::string("Expression not found in the scope."));
                    }
                }
            } catch(...) {
                CFinfo << "Error while executing python code." << CFendl;
            }
            PyErr_Clear();
            python_executing_fragment=false;
        }
        //CFinfo << "exec_unlock" << CFendl;
        python_mutex.unlock();
        boost::this_thread::sleep(wait_a_little);
    }
}
//weird will not be used I think
void python_stop_function(){
    //CFinfo << "stop_lock" << CFendl;
    python_mutex.lock();
    //CFinfo << "stop_lock_aquired" << CFendl;
    boost::this_thread::sleep(wait_a_little);
    while(1){
        //CFinfo << python_executing_fragment << "," << python_code_queue.size() << CFendl;
        if (!python_executing_fragment && python_code_queue.size()){
            python_current_fragment=python_code_queue.front();
            //CFinfo << "exec string:\n" << python_current_fragment.first << CFendl;
            python_code_queue.pop();
            python_executing_fragment=true;
            //CFinfo << "stop_unlock" << CFendl;
            python_mutex.unlock();
            boost::this_thread::sleep(wait_a_little);
            //CFinfo << "stop_lock" << CFendl;
            python_mutex.lock();
            script_engine->check_python_change();
            //CFinfo << "stop_lock_aquired" << CFendl;
        }else if (python_should_wake_up){
            python_should_wake_up=false;
            //CFinfo << "stop_unlock" << CFendl;
            python_mutex.unlock();
            //CFinfo << "stop_lock" << CFendl;
            boost::this_thread::sleep(wait_a_little);
            python_mutex.lock();
            script_engine->check_python_change();
            //CFinfo << "stop_lock_aquired" << CFendl;
        }else
            boost::this_thread::sleep(wait_a_little);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////

ScriptEngine::ScriptEngine ( const std::string& name ) : Component ( name )
{
    if (python_close++ == 0){//this class is instancied two times, don't known why
        stoped=false;
        python_should_wake_up=false;
        python_executing_fragment=false;
        python_should_be_stopped=false;
        script_engine=this;
        interpreter_mode=NORMAL_EXECUTION;
        execute_script("import sys\n",0);
        execute_script("import ctypes as dl\n",0);
        execute_script("from libcoolfluid_python import *\n",0);
        //execute_script("import wingdbstub\n",0);
        execute_script("class SimpleStringStack(object):\n"
                       "\tdef __init__(self):\n"
                       "\t\tself.data = ''\n"
                       "\tdef write(self, ndata):\n"
                       "\t\tself.data = self.data + ndata\n",0);
        execute_script("stdoutStack = SimpleStringStack()\n",0);
        execute_script("sys.stdout = stdoutStack\n",0);
        execute_script("stderrStack = SimpleStringStack()\n",0);
        execute_script("sys.stderr = stderrStack\n",0);
        //interpreter_mode=LINE_BY_LINE_EXECUTION;
        //execute_script("if ('lol').isalpha():\tprint 'alpha'\nelse:\n\tprint 'not alpha'\n",1);
        //flush_python_stdout();
        regist_signal( "execute_script" )
                .connect( boost::bind( &ScriptEngine::signal_execute_script, this, _1 ) )
                .description("Execute a python script, passed as string")
                .pretty_name("Execute Script")
                .signature( boost::bind( &ScriptEngine::signature_execute_script, this, _1 ) );
        regist_signal( "change_debug_state" )
                .connect( boost::bind( &ScriptEngine::signal_change_debug_state, this, _1 ) )
                .description("Select the current debug state")
                .pretty_name("Change debug state");
        signal("change_debug_state")->hidden(true);
        regist_signal( "get_completion" )
                .connect( boost::bind( &ScriptEngine::signal_completion, this, _1 ) )
                .description("Retrieve the completion list")
                .pretty_name("Completion list");
        signal("get_completion")->hidden(true);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////

ScriptEngine::~ScriptEngine()
{
    if (--python_close == 0){
        Py_Finalize();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////

void ScriptEngine::execute_script(std::string script,int code_fragment){
    python_code_queue.push(std::pair<std::string,int>(script,code_fragment));
}

bool ScriptEngine::check_scope_difference(PyObject* dict,std::vector<PyObject*>* diff){
    PyObject *key, *value;
    char* key_str;
    Py_ssize_t pos = 0;
    bool need_update=false;
    if (dict != NULL && diff != NULL){
        while (PyDict_Next(dict, &pos, &key, &value)) {
            if (NULL != key && NULL != value){
                key_str=PyString_AsString(key);
                if (key_str[0] != '_'){
                    bool found=false;
                    for(int i=0;i<diff->size();i++){
                        if (value == diff->at(i)){
                            found=true;
                        }
                    }
                    if (!found){
                        need_update=true;
                        std::string str(key_str);
                        diff->push_back(value);
                        completion_word_list.push_back(str);
                        if (strcmp(key_str,"__builtins__"))
                            read_objects(value,str+".");
                        else//builtins members are avalaible without writing the __builtins__
                            read_objects(value,"");
                    }
                }
            }
        }
    }
    return need_update;
}


////////////////////////////////////////////////////////////////////////////////////////////

void ScriptEngine::read_objects(PyObject* obj,std::string obj_name){
    PyObject *key;
    char* key_str;
    boost::python::handle<> dir_list(boost::python::allow_null(PyObject_Dir(obj)));
    if (NULL != dir_list.get()){
        Py_ssize_t size=PyList_Size(dir_list.get());
        for (Py_ssize_t i=0;i<size;i++){
            key=PyList_GetItem(dir_list.get(),i);
            if (NULL != key){
                boost::python::handle<> value(boost::python::allow_null(PyObject_GetAttr(obj,key)));
                key_str=PyString_AsString(key);
                if (strlen(key_str) && key_str[0] != '_' && value.get() != NULL){
                    //word_list->push_back(obj_name+key_str+PyEval_GetFuncDesc(value.get()));
                    if (PyCallable_Check(value.get()))
                        completion_word_list.push_back(obj_name+key_str+PyEval_GetFuncDesc(value.get()));
                    else
                        completion_word_list.push_back(obj_name+key_str);
                }
            }
        }
    }
}


void ScriptEngine::flush_python_stdout(){
    boost::python::handle<> py_stdout(boost::python::borrowed(boost::python::allow_null(PyDict_GetItemString(local_scope.get(),"stdoutStack"))));
    if (py_stdout.get() != NULL){
        boost::python::handle<> data(boost::python::allow_null(PyObject_GetAttrString(py_stdout.get(),"data")));
        if (data.get() != NULL){
            char * data_str=PyString_AsString(data.get());
            if (data_str != NULL){
                std::string out(data_str);
                if (strlen(data_str)>0){
                    //CFinfo << "flush emit output" << CFendl;
                    CFinfo << data_str << CFendl;
                    PyObject_SetAttrString(py_stdout.get(),"data",PyString_FromString(""));

                    //execute_script("sys.stdout.data=''",0);
                }
                //execute_line("sys.stdout.data=''");
                //PyObject_SetAttrString(py_stdout,"data",PyString_FromString(""));
            }
        }
    }
}

void ScriptEngine::check_python_change(){
    if (script_engine != NULL){
        flush_python_stdout();
        if (check_scope_difference(local_scope.get(),&local_scope_diff) || check_scope_difference(global_scope.get(),&global_scope_diff)){
            emit_completion_list();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////

void ScriptEngine::signal_execute_script(SignalArgs& node)
{
    SignalOptions options( node );
    std::string code=options.option("script").value<std::string>();
    std::replace(code.begin(),code.end(),';','\t');
    std::replace(code.begin(),code.end(),'?','\n');
    execute_script(code,options.option("fragment").value<int>());
    //return boost::python::object();
}

////////////////////////////////////////////////////////////////////////////////////////////

void ScriptEngine::signature_execute_script(SignalArgs& node)
{
    SignalOptions options( node );

    options.add_option( "script", std::string() )
            .description("Script to execute")
            .pretty_name("Script");
    options.add_option( "fragment", int() )
            .description("Code fragment number")
            .pretty_name("Code fragment");
}

////////////////////////////////////////////////////////////////////////////////////////////

int ScriptEngine::new_line_reached(int code_fragment,int line_number){
    //CFinfo << "new_line_reached:" << code_fragment << "," << line_number << CFendl;
    switch (interpreter_mode){
    case BREAK:
        interpreter_mode=NORMAL_EXECUTION;
    case LINE_BY_LINE_EXECUTION:
        //PyEval_ReInitThreads();
        //interpreter_state=PyEval_SaveThread();
        //we are in the interpreter thread at this point so unlocking the mutex will let the stop function go
        emit_debug_trace(code_fragment,line_number);
        python_mutex.unlock();
        boost::this_thread::sleep(wait_a_little);
        //check_python_change();
        python_mutex.lock();
        //wait for the script engine to unlock
        //CFinfo << "exec_lock_aquired" << CFendl;
        break;
    default:
        break;
    }
    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////

void ScriptEngine::signal_change_debug_state(SignalArgs& node){
    SignalOptions options( node );
    int commande=options.option("command").value<int>();
    switch (commande){
    case BREAK:
        if (interpreter_mode==NORMAL_EXECUTION)
            interpreter_mode=BREAK;
        break;
    case NORMAL_EXECUTION:
    case LINE_BY_LINE_EXECUTION:
        interpreter_mode=(debug_command)commande;
        break;
    case CONTINUE:
        python_should_wake_up=true;//
        //PyThreadState_Swap(&interpreter_thread_state);
        //PyEval_RestoreThread(&interpreter_thread_state);
        /*if (interpreter_thread_state.inte!=NULL){
            state=interpreter_thread_state;
            interpreter_thread_state=NULL;
            PyEval_RestoreThread(state);
            //execute_frame(state);
            PyEval_SaveThread();
            PyEval_RestoreThread(interpreter_state);

            //PyEval_SetTrace(python_trace,NULL);
            //PyEval_AcquireThread();
        }*/
    default:
        break;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////

void ScriptEngine::emit_output(std::string output){

    /// @todo remove those hardcoded URIs
    //CFinfo << "py:" << output << CFendl;
    m_manager=Core::instance().root().access_component_checked("//Tools/PEManager")->handle<PE::Manager>();
    SignalFrame frame("output", uri(), "cpath:/UI/ScriptEngine");
    SignalOptions options(frame);
    options.add_option("text", output);
    options.flush();
    m_manager->send_to_parent( frame );
}

////////////////////////////////////////////////////////////////////////////////////////////

void ScriptEngine::emit_completion_list(){
    /// @todo remove those hardcoded URIs
    m_manager=Core::instance().root().access_component_checked("//Tools/PEManager")->handle<PE::Manager>();
    SignalFrame frame("completion", uri(), "cpath:/UI/ScriptEngine");
    SignalOptions options(frame);
    options.add_option("words", completion_word_list);
    options.flush();
    m_manager->send_to_parent(frame);
}


void ScriptEngine::emit_debug_trace(int fragment,int line){
    /// @todo remove those hardcoded URIs
    m_manager=Core::instance().root().access_component_checked("//Tools/PEManager")->handle<PE::Manager>();
    SignalFrame frame("debug_trace", uri(), "cpath:/UI/ScriptEngine");
    SignalOptions options(frame);
    options.add_option("fragment", fragment);
    options.add_option("line", line);
    options.flush();
    m_manager->send_to_parent(frame);
}

////////////////////////////////////////////////////////////////////////////////////////////

void ScriptEngine::signal_completion(SignalArgs& node){
    emit_completion_list();
}


} // python
} // cf3
