// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "python/ScriptEngine.hpp"

#include <coolfluid-paths.hpp>

#include "common/BoostFilesystem.hpp"
#include "common/Builder.hpp"
#include "common/LibCommon.hpp"
#include "common/Log.hpp"
#include "common/OptionT.hpp"
#include "common/OSystem.hpp"
#include "common/Signal.hpp"

#include "common/OSystem.hpp"
#include "common/OSystemLayer.hpp"

#include "common/XML/Protocol.hpp"
#include "common/XML/SignalOptions.hpp"

#include "ui/uicommon/ComponentNames.hpp"

#include <boost/thread/thread.hpp>
#include <boost/thread/locks.hpp>
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
std::queue< std::pair<boost::python::handle<>,int> > python_code_queue;
boost::mutex* python_code_queue_mutex;
boost::condition_variable* python_code_queue_condition;
boost::mutex* python_break_mutex;
boost::mutex* python_scope_mutex;
boost::condition_variable* python_break_condition;
boost::thread* python_thread;
boost::thread* python_stream_statement_thread;
int current_execution_fragment;
bool python_should_wake_up;
bool python_should_break;
bool python_should_run;

////////////////////////////////////////////////////////////////////////////////////////////

int python_trace(PyObject *obj, PyFrameObject *frame, int what, PyObject *arg){
  if ( what == PyTrace_LINE ){
    boost::python::handle<> frag_number(boost::python::allow_null(PyObject_Str(frame->f_code->co_filename)));
    if (frag_number.get() != NULL){
      int code_fragment=atoi(PyString_AsString(frag_number.get()));
      if (code_fragment > 0){
        return script_engine->new_line_reached(code_fragment,frame->f_lineno);
      }
    }
  }
  return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////

void python_execute_function(){
  boost::unique_lock<boost::mutex> locker(*python_code_queue_mutex,boost::defer_lock_t());
  locker.lock();
  boost::posix_time::millisec wait_init(500);//to wait that the component tree is created
  boost::this_thread::sleep(wait_init);
  while(python_should_run){
    if (python_code_queue.size()){
      std::pair<boost::python::handle<>,int> python_current_fragment=python_code_queue.front();
      python_code_queue.pop();
      locker.unlock();
      current_execution_fragment=python_current_fragment.second;
      try{
        {
          boost::lock_guard<boost::mutex> lock(*python_scope_mutex);
          boost::python::handle<> dum(boost::python::allow_null(PyEval_EvalCode((PyCodeObject *)python_current_fragment.first.get(), global_scope.get(), local_scope.get())));
        }
        if (python_current_fragment.second){//we don't check errors on internal request (fragment code = 0)
          PyObject *exc,*val,*trb,*obj;
          char* error;
          PyErr_Fetch(&exc, &val, &trb);
          if (NULL != val){
            if (PyArg_ParseTuple (val, "sO", &error, &obj)){
              CFinfo << ":" << python_current_fragment.second << ":" << error << CFendl;
            }else{
              const char* val_str=PyString_AsString(PyObject_Str(val));
              CFinfo << ":" << python_current_fragment.second << ":" << val_str << CFendl;
            }
          }
        }
      } catch(...) {
        CFinfo << "Error while executing python code." << CFendl;
      }
      PyErr_Clear();
      script_engine->check_python_change();
      locker.lock();
    }else{
      script_engine->no_more_instruction();
      python_code_queue_condition->wait(locker);
    }
  }
  locker.unlock();
}

////////////////////////////////////////////////////////////////////////////////////////////

ScriptEngine::ScriptEngine ( const std::string& name ) : Component ( name )
{
  if (python_close++ == 0){//near singleton implementation
    fragment_generator=1;
    stoped=false;
    python_should_wake_up=false;
    python_should_break=false;
    python_should_run=true;
    script_engine=this;
    python_code_queue_mutex = new boost::mutex();
    python_code_queue_condition = new boost::condition_variable();
    python_break_mutex = new boost::mutex();
    python_break_condition = new boost::condition_variable();
    python_scope_mutex = new boost::mutex();
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
    python_stream_statement_thread=new boost::thread(python_execute_function);
    execute_script("from coolfluid import *\n"
                   "class SimpleStringStack(object):\n"
                   "\tdef __init__(self):\n"
                   "\t\tself.data = ''\n"
                   "\tdef write(self, ndata):\n"
                   "\t\tself.data = self.data + ndata\n"
                   "stdoutStack = SimpleStringStack()\n"
                   "sys.stdout = stdoutStack\n"
                   "stderrStack = SimpleStringStack()\n"
                   "sys.stderr = stderrStack\n"
                   "Root=Core.root()");
    regist_signal( "execute_script" )
        .connect( boost::bind( &ScriptEngine::signal_execute_script, this, _1 ) )
        .description("Execute a python script, passed as string")
        .pretty_name("Execute Script")
        .signature( boost::bind( &ScriptEngine::signature_execute_script, this, _1 ) );
    regist_signal( "get_documentation" )
        .connect( boost::bind( &ScriptEngine::signal_get_documentation, this, _1 ) )
        .description("Give the documentation of a python expression")
        .pretty_name("Get documentation")
        .hidden(true);
    regist_signal( "change_debug_state" )
        .connect( boost::bind( &ScriptEngine::signal_change_debug_state, this, _1 ) )
        .description("Select the current debug state")
        .pretty_name("Change debug state")
        .hidden(true);
    regist_signal( "get_completion" )
        .connect( boost::bind( &ScriptEngine::signal_completion, this, _1 ) )
        .description("Retrieve the completion list")
        .pretty_name("Completion list")
        .hidden(true);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

ScriptEngine::~ScriptEngine()
{
  if (--python_close == 0){
    python_should_run=false;
    {
      boost::lock_guard<boost::mutex> lock(*python_code_queue_mutex);
      // the thread is probably waiting for the condition variable at this point
      python_code_queue_condition->notify_one();
      // to be sure that the thread ends well
    }
    Py_Finalize();
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

int ScriptEngine::execute_script(std::string script,int code_fragment){
  boost::lock_guard<boost::mutex> lock(*python_code_queue_mutex);
  // we compile the received fragment
  std::stringstream ss;
  if (code_fragment > 0)
    code_fragment=fragment_generator++;
  ss << code_fragment;
  boost::python::handle<> src;
  if (code_fragment == -1){//entire script 'flag'
    src=boost::python::handle<>(boost::python::allow_null(Py_CompileString(script.c_str(), ss.str().c_str(), Py_file_input)));
    if (NULL != src.get()){
      boost::lock_guard<boost::mutex> lock(*python_scope_mutex);
      boost::python::handle<> dum(boost::python::allow_null(PyEval_EvalCode((PyCodeObject *)src.get(), global_scope.get(), local_scope.get())));
    }
  }else{//single input compilation
    src=boost::python::handle<>(boost::python::allow_null(Py_CompileString(script.c_str(), ss.str().c_str(), Py_single_input)));
    if (NULL != src.get()){
        python_code_queue.push(std::pair<boost::python::handle<>,int>(src,code_fragment));
        python_code_queue_condition->notify_one();
    }
    if (code_fragment){//we don't check errors on internal request (fragment code = 0)
      PyObject *exc,*val,*trb,*obj;
      char* error;
      PyErr_Fetch(&exc, &val, &trb);
      if (NULL != val){
        if (PyArg_ParseTuple (val, "sO", &error, &obj)){
          CFinfo << ":" << code_fragment << ":" << error << CFendl;
        }else{
          const char* val_str=PyString_AsString(PyObject_Str(val));
          CFinfo << ":" << code_fragment << ":" << val_str << CFendl;
        }
      }
    }
  }
  PyErr_Clear();
  return code_fragment;
}

////////////////////////////////////////////////////////////////////////////////////////////

void ScriptEngine::check_scope_difference(PythonDictEntry &entry,std::string name,std::vector<std::string> *add,std::vector<std::string> *sub, int rec,bool child){
  static char value_limiter[105];//100 character limit (+ ...\0)
  PyObject *py_key, *py_value;
  char* key_str;
  bool fetch_values=interpreter_mode==STOP || interpreter_mode==LINE_BY_LINE_EXECUTION;
#if PY_VERSION_HEX >= 0x02050000
  ssize_t pos = 0;
#else
 int pos = 0;
#endif
  ssize_t dir_size;
  std::vector<bool> reverse_found;
  std::string c_name;
  std::string value_string;
  if (entry.name.size()){
    c_name=(name.size()>0?name+".":"")+entry.name;
  }
  if (c_name == "sys.stdin" || c_name =="sys.stdout" || c_name == "sys.stderr")
    return;
  reverse_found.resize(entry.entry_list.size(),false);
  bool loop=true;
  if (entry.py_ref != NULL){
    boost::python::handle<> dir_list;
    if (rec > 0){
      dir_list=boost::python::handle<>(boost::python::allow_null(PyObject_Dir(entry.py_ref)));
      if (dir_list.get() != NULL)
        dir_size=PyList_Size(dir_list.get());
      else
        loop=false;
    }
    while (loop) {
      boost::python::handle<> key;
      boost::python::handle<> value;

      if (rec==0){//scope fetch
        loop=PyDict_Next(entry.py_ref, &pos, &py_key, &py_value);
        key=boost::python::handle<>(boost::python::borrowed(boost::python::allow_null(py_key)));
        value=boost::python::handle<>(boost::python::borrowed(boost::python::allow_null(py_value)));
      }else{
        key=boost::python::handle<>(boost::python::borrowed(boost::python::allow_null(PyList_GetItem(dir_list.get(),pos++))));
        if (NULL != key.get())
          value=boost::python::handle<>(boost::python::allow_null(PyObject_GetAttr(entry.py_ref,key.get())));
        if (pos >= dir_size)
          loop=false;
      }

      if (NULL != key.get() && NULL != value.get()){
        key_str=PyString_AsString(key.get());
        if (key_str[0] != '_'){
          bool found=false;
          if (fetch_values)
            value_string=PyString_AsString(PyObject_Str(value.get()));
          bool callable=PyCallable_Check(value.get());
          int i=0;
          for(;i<entry.entry_list.size();i++){
            if (entry.entry_list[i].py_ref==value.get() && entry.entry_list[i].name == key_str){
              if (fetch_values && !callable){
                if (entry.entry_list[i].value==value_string){
                  found=true;
                  reverse_found[i]=true;
                  break;
                }
              }else{
                found=true;
                reverse_found[i]=true;
                break;
              }
            }
          }
          if (!found){
            PythonDictEntry n_entry;
            n_entry.py_ref=value.get();
            n_entry.name=key_str;
            if (PyCallable_Check(value.get())){
              add->push_back((c_name.size()>0&&!child?c_name+".":"")+n_entry.name+"(:");
            }else{
              if (fetch_values){
                char* value_str=PyString_AsString(PyObject_Str(value.get()));
                strncpy(value_limiter,value_str,100);
                if (strlen(value_limiter) > 99){
                  strcpy(value_limiter+99,"...");
                }
                n_entry.value=value_limiter;
              }
              add->push_back((c_name.size()>0&&!child?c_name+".":"")+n_entry.name+":"+n_entry.value+"{");
              if (rec < 2)
                check_scope_difference(n_entry,c_name,add,sub,rec+1,true);
              add->push_back("}");
            }
            entry.entry_list.push_back(n_entry);
            reverse_found.push_back(true);
          }else{
            if (!PyCallable_Check(value.get())){
              PythonDictEntry n_entry=entry.entry_list[i];
              if (rec < 2)
                check_scope_difference(n_entry,c_name,add,sub,rec+1,false);
              entry.entry_list[i]=n_entry;
            }
          }
        }
      }
    }
  }
  for (int i=0;i<reverse_found.size();i++){
    if (!reverse_found[i]){
      reverse_found[i]=reverse_found[reverse_found.size()-1];
      reverse_found.pop_back();
      sub->push_back((c_name.size()>0?c_name+".":"")+entry.entry_list[i].name);
      entry.entry_list[i]=entry.entry_list[entry.entry_list.size()-1];
      entry.entry_list.pop_back();
      i--;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

void ScriptEngine::flush_python_stdout(){
  boost::python::handle<> name(PyString_FromString("data"));
  boost::python::handle<> blank(PyString_FromString(""));
  boost::python::handle<> py_stdout(boost::python::borrowed(boost::python::allow_null(PyDict_GetItemString(local_scope.get(),"stdoutStack"))));
  if (py_stdout.get() != NULL){
    boost::python::handle<> data(boost::python::allow_null(PyObject_GetAttr(py_stdout.get(),name.get())));
    if (data.get() != NULL){
      char * data_str=PyString_AsString(data.get());
      if (strlen(data_str)>0){
        //CFinfo << "flush emit output" << CFendl;
        if (current_execution_fragment)
          CFinfo << ":" << current_execution_fragment << ":" << data_str << CFendl;
        else//must be a documentation request
          emit_documentation(data_str);
        PyObject_SetAttr(py_stdout.get(),name.get(),blank.get());
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

void ScriptEngine::check_python_change(){
  boost::lock_guard<boost::mutex> lock(*python_scope_mutex);
  flush_python_stdout();
  std::vector<std::string> add,sub;
  check_scope_difference(local_scope_entry,"",&add,&sub);
  builtin_scope_entry.py_ref=PyEval_GetBuiltins();
  check_scope_difference(builtin_scope_entry,"",&add,&sub);
  if (PyEval_GetFrame() != NULL){
    std::string target=PyString_AsString(PyEval_GetFrame()->f_code->co_name);
    if (target != "<module>"){
      local_frame_scope_entry.py_ref=PyEval_GetLocals();
      local_frame_scope_entry.name=target+"(";
      global_frame_scope_entry.py_ref=PyEval_GetGlobals();
      global_frame_scope_entry.name=target+"(";
    }
  }else{
    local_frame_scope_entry.py_ref=NULL;
    global_frame_scope_entry.py_ref=NULL;
  }
  check_scope_difference(local_frame_scope_entry,"",&add,&sub);
  check_scope_difference(global_frame_scope_entry,"",&add,&sub);
  if (add.size() || sub.size()){
    emit_completion_list(&add,&sub);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

void ScriptEngine::no_more_instruction(){
  switch (interpreter_mode){
  case STOP:
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
  int fragment=-1;
  if (options.check("fragment")){
    fragment=options.option("fragment").value<int>();
  }
  if (options.check("breakpoints")){
    std::vector<int> break_lines=options.array<int>("breakpoints");
    for (int i=0;i<break_lines.size();i++){
      break_points.push_back(std::pair<int,int>(fragment,break_lines[i]));
    }
  }
  int new_fragment=0;
  {
    boost::lock_guard<boost::mutex> lock(compile_mutex);
    new_fragment=execute_script(code,fragment);
  }
  if (new_fragment > 0 && fragment != new_fragment){
    SignalFrame reply=node.create_reply();
    SignalOptions options(reply);
    options.add("fragment", fragment);
    options.add("new_fragment", new_fragment);
    options.flush();
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

void ScriptEngine::signature_execute_script(SignalArgs& node)
{
  SignalOptions options( node );

  options.add( "script", std::string() )
      .description("Script to execute")
      .pretty_name("Script");
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
  if (interpreter_mode==NORMAL_EXECUTION)
    for (int i=0;i<break_points.size();i++)
      if (break_points[i].first == code_fragment && break_points[i].second == line_number-1)
        interpreter_mode=STOP;
  switch (interpreter_mode){
  case STOP:
  case LINE_BY_LINE_EXECUTION:
    python_should_wake_up=false;
    python_scope_mutex->unlock();
    emit_debug_trace(code_fragment,line_number);
    check_python_change();
  {
    boost::unique_lock<boost::mutex> lock(*python_break_mutex);
    while (!python_should_wake_up)
      python_break_condition->wait(lock);
  }
    if (interpreter_mode==STOP)
      interpreter_mode=NORMAL_EXECUTION;
    if (python_should_break){
      python_should_break=false;
      return -1;
    }
    python_scope_mutex->lock();
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
  std::pair<int,int> fragment;
  bool push;
  switch (commande){
  case STOP:
    if (interpreter_mode==NORMAL_EXECUTION)
      interpreter_mode=STOP;
    break;
  case NORMAL_EXECUTION:
  case LINE_BY_LINE_EXECUTION:
    interpreter_mode=(debug_command)commande;
    break;
  case BREAK:
    python_should_break=true;
  case CONTINUE:
  {
    boost::lock_guard<boost::mutex> lock(*python_break_mutex);
    python_should_wake_up=true;
  }
    python_break_condition->notify_one();
    break;
  case TOGGLE_BREAK_POINT:
    fragment=std::pair<int,int>(options.option("fragment").value<int>(),options.option("line").value<int>());
    push=true;
    for (int i=0;i<break_points.size();i++){
      if (break_points[i].first == fragment.first && break_points[i].second == fragment.second){
        break_points[i]=break_points[break_points.size()-1];
        break_points.pop_back();
        push=false;
        break;
      }
    }
    if (push)
      break_points.push_back(fragment);
  default:
    break;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

void ScriptEngine::emit_completion_list(std::vector<std::string> *add, std::vector<std::string> *sub){
  if (m_manager.get()==NULL)
    access_pe_manager();
  if (m_manager.get()!=NULL){
    SignalFrame frame("completion", uri(), CLIENT_SCRIPT_ENGINE_PATH);
    SignalOptions options(frame);
    options.add("add", *add);
    options.add("sub", *sub);
    options.flush();
    m_manager->send_to_parent(frame);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

void ScriptEngine::emit_documentation(std::string doc){
  if (m_manager.get()==NULL)
    access_pe_manager();
  if (m_manager.get()!=NULL){
    SignalFrame frame("documentation", uri(), CLIENT_SCRIPT_ENGINE_PATH);
    SignalOptions options(frame);
    options.add("text", doc);
    options.flush();
    m_manager->send_to_parent( frame );
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

void ScriptEngine::emit_debug_trace(int fragment,int line){
  if (m_manager.get()==NULL)
    access_pe_manager();
  if (m_manager.get()!=NULL){
    SignalFrame frame("debug_trace", uri(), CLIENT_SCRIPT_ENGINE_PATH);
    SignalOptions options(frame);
    options.add("fragment", fragment);
    options.add("line", line);
    options.flush();
    m_manager->send_to_parent(frame);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

void ScriptEngine::signal_completion(SignalArgs& node){
  local_scope_entry.entry_list.clear();
  local_frame_scope_entry.entry_list.clear();
  global_frame_scope_entry.entry_list.clear();
  builtin_scope_entry.entry_list.clear();
  {
    boost::lock_guard<boost::mutex> lock(*python_scope_mutex);
    std::vector<std::string> add,sub;
    sub.push_back("*");//to clear the dictionary client-side
    check_scope_difference(local_scope_entry,"",&add,&sub);
    check_scope_difference(builtin_scope_entry,"",&add,&sub);
    if (add.size() || sub.size()){
      emit_completion_list(&add,&sub);
    }
  }
}

void ScriptEngine::access_pe_manager(){
  Handle<Component> tools=Core::instance().root().get_child("Tools");
  if (tools.get()!= NULL){
    Handle<Component> pe_manager=tools->get_child("PEManager");
    if (pe_manager.get()!=NULL)
      m_manager=pe_manager->handle<PE::Manager>();
  }
}


} // python
} // cf3
