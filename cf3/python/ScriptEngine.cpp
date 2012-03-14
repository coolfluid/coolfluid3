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

#include "ui/uicommon/ComponentNames.hpp"

#include "python/ScriptEngine.hpp"

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>
#include <boost/python/handle.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>
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
std::queue< std::pair<std::string,int> > python_code_queue;
boost::thread* python_thread;
boost::thread* python_stream_statement_thread;
boost::mutex python_code_queue_mutex;
boost::condition_variable python_code_queue_condition;
boost::mutex python_break_mutex;
boost::condition_variable python_break_condition;
bool python_should_wake_up;

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

////////////////////////////////////////////////////////////////////////////////////////////

void python_execute_function(){
  boost::posix_time::millisec wait_init(500);//to wait that the component tree is created
  //CFinfo << "stop_lock_aquired" << CFendl;
  boost::this_thread::sleep(wait_init);
  while(1){
    python_code_queue_mutex.lock();
    if (python_code_queue.size()){
      std::pair<std::string,int> python_current_fragment=python_code_queue.front();
      python_code_queue.pop();
      python_code_queue_mutex.unlock();
      try{
        std::stringstream ss;
        ss << python_current_fragment.second;
        //CFinfo << python_current_fragment.first.c_str() << CFendl;
        boost::python::handle<> src(boost::python::allow_null(Py_CompileString(python_current_fragment.first.c_str(), ss.str().c_str(), Py_single_input)));
        if (NULL != src){//code compiled, we execute it
          boost::python::handle<> dum(boost::python::allow_null(PyEval_EvalCode((PyCodeObject *)src.get(), global_scope.get(), local_scope.get())));
        }
        if (python_current_fragment.second){//we don't check errors on internal request (fragment code = 0)
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
        }
      } catch(...) {
        CFinfo << "Error while executing python code." << CFendl;
      }
      PyErr_Clear();
      script_engine->check_python_change(python_current_fragment.second);
    }else{
      python_code_queue_mutex.unlock();
      script_engine->no_more_instruction();
      boost::unique_lock<boost::mutex> lock(python_code_queue_mutex);
      python_code_queue_condition.wait(lock);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

ScriptEngine::ScriptEngine ( const std::string& name ) : Component ( name )
{
  if (python_close++ == 0){//this class is instancied two times, don't known why
    stoped=false;
    python_should_wake_up=false;
    script_engine=this;
    interpreter_mode=NORMAL_EXECUTION;
    //init python
    const boost::filesystem::path dso_dir = boost::filesystem::path(CF3_BUILD_DIR) / boost::filesystem::path("dso");
    OSystem::setenv("PYTHONPATH", dso_dir.string());
    Py_Initialize();
    local_scope = boost::python::handle<>(PyDict_New());
    global_scope = boost::python::handle<>(PyDict_New());
    PyDict_SetItemString (global_scope.get(), "__builtins__", PyEval_GetBuiltins());
    PyEval_SetTrace(python_trace,NULL);
    local_scope_entry.py_ref=local_scope.get();
    local_scope_entry.is_module=true;
    python_stream_statement_thread=new boost::thread(python_execute_function);
    execute_script("import sys\n",0);
    //execute_script("import ctypes as dl\n",0);
    execute_script("from libcoolfluid_python import *\n",0);
    execute_script("class SimpleStringStack(object):\n"
            "\tdef __init__(self):\n"
            "\t\tself.data = ''\n"
            "\tdef write(self, ndata):\n"
            "\t\tself.data = self.data + ndata\n",0);
    execute_script("stdoutStack = SimpleStringStack()\n",0);
    execute_script("sys.stdout = stdoutStack\n",0);
    execute_script("stderrStack = SimpleStringStack()\n",0);
    execute_script("sys.stderr = stderrStack\n",0);
    execute_script("Root=Core.root()",0);
    regist_signal( "execute_script" )
        .connect( boost::bind( &ScriptEngine::signal_execute_script, this, _1 ) )
        .description("Execute a python script, passed as string")
        .pretty_name("Execute Script")
        .signature( boost::bind( &ScriptEngine::signature_execute_script, this, _1 ) );
    regist_signal( "get_documentation" )
        .connect( boost::bind( &ScriptEngine::signal_get_documentation, this, _1 ) )
        .description("Give the documentation of a python expression")
        .pretty_name("Get documentation");
    signal("get_documentation")->hidden(true);
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
  {
    boost::lock_guard<boost::mutex> lock(python_code_queue_mutex);
    python_code_queue.push(std::pair<std::string,int>(script,code_fragment));
  }
  python_code_queue_condition.notify_one();
}

////////////////////////////////////////////////////////////////////////////////////////////

void ScriptEngine::check_scope_difference(PythonDictEntry &entry,std::string name,std::vector<std::string> *add,std::vector<std::string> *sub, int rec){
  if (rec > 2)//max recusivity
    return;
  PyObject *key, *value;
  char* key_str;
  Py_ssize_t pos = 0;
  std::vector<bool> revers_found;
  std::string c_name;
  if (entry.name.size()){
    c_name=(name.size()>0?name+".":"")+entry.name;
  }
  revers_found.resize(entry.entry_list.size(),false);
  if (entry.is_module){
    while (PyDict_Next(entry.py_ref, &pos, &key, &value)) {
      if (NULL != key && NULL != value){
        key_str=PyString_AsString(key);
        if (key_str[0] != '_'){
          bool found=false;
          for(int i=0;i<entry.entry_list.size();i++){
            if (value == entry.entry_list[i].py_ref){
              found=true;
              revers_found[i]=true;
            }
          }
          if (!found){
            PythonDictEntry n_entry;
            n_entry.py_ref=value;
            n_entry.name=key_str;
            n_entry.is_module=false;//!strcmp(n_entry.py_ref->ob_type->tp_name,"module");
            if (PyCallable_Check(value))
              add->push_back(n_entry.name+"(");
            else{
              add->push_back(n_entry.name+"{");
              check_scope_difference(n_entry,c_name,add,sub,rec+1);
              add->push_back("}");
            }
            entry.entry_list.push_back(n_entry);
            revers_found.push_back(true);
          }
        }
      }
    }
  }else{
    char *key_str;
    boost::python::handle<> dir_list(boost::python::allow_null(PyObject_Dir(entry.py_ref)));
    if (NULL != dir_list.get()){
      Py_ssize_t size=PyList_Size(dir_list.get());
      for (Py_ssize_t i=0;i<size;i++){
        boost::python::handle<> key(boost::python::borrowed(boost::python::allow_null(PyList_GetItem(dir_list.get(),i))));
        if (NULL != key.get()){
          boost::python::handle<> value(boost::python::allow_null(PyObject_GetAttr(entry.py_ref,key.get())));
          key_str=PyString_AsString(key.get());
          if (key_str!=NULL && strlen(key_str) && key_str[0] != '_' && value.get() != NULL){
            bool found=false;
            for(int i=0;i<entry.entry_list.size();i++){
              if (value.get() == entry.entry_list[i].py_ref){
                found=true;
                revers_found[i]=true;
              }
            }
            if (!found){
              PythonDictEntry n_entry;
              n_entry.py_ref=value.get();
              if (PyCallable_Check(value.get())){
                n_entry.name=std::string(key_str)+"(";
                add->push_back(n_entry.name);
              }else{
                n_entry.is_module=false;//!strcmp(value.get()->ob_type->tp_name,"module");
                n_entry.name=key_str;
                add->push_back(n_entry.name+"{");
                check_scope_difference(n_entry,c_name,add,sub,rec+1);
                add->push_back("}");
              }
              entry.entry_list.push_back(n_entry);
              revers_found.push_back(true);
            }
          }
        }
      }
    }
  }
  for (int i=0;i<revers_found.size();i++){
    if (!revers_found[i]){
      revers_found[i]=revers_found[revers_found.size()-1];
      revers_found.pop_back();
      sub->push_back((c_name.size()>0?c_name+".":"")+entry.entry_list[i].name);
      entry.entry_list[i]=entry.entry_list[entry.entry_list.size()-1];
      entry.entry_list.pop_back();
      i--;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

void ScriptEngine::getScopeValues(std::vector<std::string> &names,std::vector<std::string> &values){
  PyObject *key, *value;
  char* key_str;
  Py_ssize_t pos = 0;
  if (local_scope.get() != NULL){
    while (PyDict_Next(local_scope.get(), &pos, &key, &value)) {
      if (NULL != key && NULL != value){
        key_str=PyString_AsString(key);
        if (key_str[0] != '_'){
          boost::python::handle<> value_str_obj(boost::python::allow_null(PyObject_Str(value)));
          if (value_str_obj.get() != NULL){
            names.push_back(std::string(key_str));
            values.push_back(std::string(PyString_AsString(value_str_obj.get())));
          }
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

void ScriptEngine::flush_python_stdout(int code_fragment){
  boost::python::handle<> py_stdout(boost::python::borrowed(boost::python::allow_null(PyDict_GetItemString(local_scope.get(),"stdoutStack"))));
  if (py_stdout.get() != NULL){
    boost::python::handle<> data(boost::python::allow_null(PyObject_GetAttrString(py_stdout.get(),"data")));
    if (data.get() != NULL){
      char * data_str=PyString_AsString(data.get());
      if (strlen(data_str)>0){
        //CFinfo << "flush emit output" << CFendl;
        if (code_fragment)
          CFinfo << data_str << CFendl;
        else//must be a documentation request
          emit_documentation(data_str);
        //Py_XDECREF(data.get());
        PyObject_SetAttrString(py_stdout.get(),"data",PyString_FromString(""));
        //execute_script("sys.stdout.data=''",0);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

void ScriptEngine::check_python_change(int code_fragment){
  flush_python_stdout(code_fragment);
  std::vector<std::string> add,sub;
  check_scope_difference(local_scope_entry,"",&add,&sub);
  if (add.size() || sub.size()){
    emit_completion_list(&add,&sub);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

void ScriptEngine::no_more_instruction(){
  switch (interpreter_mode){
  case BREAK:
    interpreter_mode=NORMAL_EXECUTION;
  case LINE_BY_LINE_EXECUTION:
    emit_debug_trace(0,0);//end of execution trace
    break;
  default:
    break;
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

void ScriptEngine::signal_get_documentation(SignalArgs& node)
{
  SignalOptions options( node );
  std::string code=options.option("expression").value<std::string>();
  execute_script(code.append(".__doc__"),0);
  //return boost::python::object();
}

////////////////////////////////////////////////////////////////////////////////////////////

int ScriptEngine::new_line_reached(int code_fragment,int line_number){
  switch (interpreter_mode){
  case BREAK:
    interpreter_mode=NORMAL_EXECUTION;
  case LINE_BY_LINE_EXECUTION:
    python_should_wake_up=false;
    emit_debug_trace(code_fragment,line_number);
    flush_python_stdout(code_fragment);
  {
    boost::unique_lock<boost::mutex> lock(python_break_mutex);
    while (!python_should_wake_up)
      python_break_condition.wait(lock);
  }
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
  {
    boost::lock_guard<boost::mutex> lock(python_break_mutex);
    python_should_wake_up=true;
  }
    python_break_condition.notify_one();
  default:
    break;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

void ScriptEngine::emit_output(std::string output){//not used anymore
  /// @todo remove those hardcoded URIs
  m_manager=Core::instance().root().access_component("//Tools/PEManager")->handle<PE::Manager>();
  SignalFrame frame("output", uri(), CLIENT_SCRIPT_ENGINE_PATH);
  SignalOptions options(frame);
  options.add_option("text", output);
  options.flush();
  m_manager->send_to_parent( frame );
}

////////////////////////////////////////////////////////////////////////////////////////////

void ScriptEngine::emit_completion_list(std::vector<std::string> *add, std::vector<std::string> *sub){
  /// @todo remove those hardcoded URIs
  m_manager=Core::instance().root().access_component_checked("//Tools/PEManager")->handle<PE::Manager>();
  SignalFrame frame("completion", uri(), CLIENT_SCRIPT_ENGINE_PATH);
  SignalOptions options(frame);
  options.add_option("add", *add);
  options.add_option("sub", *sub);
  options.flush();
  m_manager->send_to_parent(frame);
}

////////////////////////////////////////////////////////////////////////////////////////////

void ScriptEngine::emit_documentation(std::string doc){
  /// @todo remove those hardcoded URIs
  m_manager=Core::instance().root().access_component("//Tools/PEManager")->handle<PE::Manager>();
  SignalFrame frame("documentation", uri(), CLIENT_SCRIPT_ENGINE_PATH);
  SignalOptions options(frame);
  options.add_option("text", doc);
  options.flush();
  m_manager->send_to_parent( frame );
}

////////////////////////////////////////////////////////////////////////////////////////////

void ScriptEngine::emit_debug_trace(int fragment,int line){
  /// @todo remove those hardcoded URIs
  m_manager=Core::instance().root().access_component_checked("//Tools/PEManager")->handle<PE::Manager>();
  SignalFrame frame("debug_trace", uri(), CLIENT_SCRIPT_ENGINE_PATH);
  SignalOptions options(frame);
  options.add_option("fragment", fragment);
  options.add_option("line", line);
  std::vector<std::string> keys,values;
  getScopeValues(keys,values);
  options.add_option("scope_keys",keys);
  options.add_option("scope_values",values);
  options.flush();
  m_manager->send_to_parent(frame);
}

////////////////////////////////////////////////////////////////////////////////////////////

void ScriptEngine::signal_completion(SignalArgs& node){
  local_scope_entry.entry_list.clear();
  std::vector<std::string> add,sub;
  sub.push_back("*");//to clear the dictionary client-side
  check_scope_difference(local_scope_entry,"",&add,&sub);
  if (add.size() || sub.size()){
    emit_completion_list(&add,&sub);
  }
}


} // python
} // cf3
