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
#include <vector>
#include <string>
#include <stdio.h>

#include <Python.h>
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

////////////////////////////////////////////////////////////////////////////////////////////

int pytohn_trace(PyObject *obj, PyFrameObject *frame, int what, PyObject *arg){
    if ( what == PyTrace_LINE ){
        boost::python::handle<> frag_number(boost::python::allow_null(PyObject_Str(frame->f_code->co_filename)));
        if (frag_number.get() != NULL){
            int code_fragment=atoi(PyString_AsString(frag_number.get()));
            if (code_fragment > 0)
                CFinfo << "executing code fragment :" << code_fragment << " line :" << frame->f_lineno << CFendl;
        }
    }
    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////

ScriptEngine::ScriptEngine ( const std::string& name ) : Component ( name )
{
    python_close++;
    if(!Py_IsInitialized())
    {
        const boost::filesystem::path dso_dir = boost::filesystem::path(CF3_BUILD_DIR) / boost::filesystem::path("dso");
        OSystem::setenv("PYTHONPATH", dso_dir.string());
        Py_Initialize();
        local_scope = boost::python::handle<>(PyDict_New());
        global_scope = boost::python::handle<>(PyDict_New());
        PyDict_SetItemString (global_scope.get(), "__builtins__", PyEval_GetBuiltins());
        PyEval_SetTrace(pytohn_trace,NULL);
        execute_script("import sys\n",0);
        execute_script("import ctypes as dl\n",0);
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
        flush_python_stdout();
        /*std::vector<std::string>glb;
        read_dictionary(local_scope.get(),&glb);
        for (int i=0;i<glb.size();i++)
            CFinfo << glb[i] << CFendl;*/
/*                       "root = Core.root()\n"
                       "model = root.create_component('model', 'cf3.solver.Model')\n"
                       "domain = model.create_domain()\n"
                       "solver = model.create_solver('cf3.solver.SimpleSolver')\n"
                       "phys_model = model.create_physics('cf3.physics.DynamicModel')\n");*/
        //m_manager=Core::instance().root().access_component_checked("//Tools/PEManager")->handle<PE::Manager>();
    }
    regist_signal( "execute_script" )
            .connect( boost::bind( &ScriptEngine::signal_execute_script, this, _1 ) )
            .description("Execute a python script, passed as string")
            .pretty_name("Execute Script")
            .signature( boost::bind( &ScriptEngine::signature_execute_script, this, _1 ) );
    regist_signal( "get_completion" )
            .connect( boost::bind( &ScriptEngine::signal_completion, this, _1 ) )
            .description("Retrieve the completion list")
            .pretty_name("Completion list");
    signal("get_completion")->hidden(true);
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
    try{
        std::stringstream ss;
        ss << code_fragment;
        boost::python::handle<> src(boost::python::allow_null(Py_CompileString(script.c_str(), ss.str().c_str(), Py_single_input)));
        if (NULL != src){//code compiled, we execute it
            boost::python::handle<> dum(boost::python::allow_null(PyEval_EvalCode((PyCodeObject *)src.get(), global_scope.get(), local_scope.get())));
        }
        PyObject *exc,*val,*trb,*obj;
        char* error;
        PyErr_Fetch(&exc, &val, &trb);
        if (NULL != exc && NULL != val){
            if (PyArg_ParseTuple (val, "sO", &error, &obj)){
                emit_output(std::string(error));
            }else{
                emit_output(std::string("Expression not found in the scope."));
            }
        }
    } catch(...) {
        CFinfo << "Error while executing Python" << CFendl;
    }
    PyErr_Clear();
    //old code
    /*std::istringstream str(script);
    std::string line;
    while (std::getline(str,line)){
        execute_line(line);
    }*/
}

////////////////////////////////////////////////////////////////////////////////////////////

void ScriptEngine::execute_line(std::string line){
    //old code
    /*bool exec_code=true,need_next_exec=false;
    if (current_instruction.size() > 0){
        if (line[0]=='\t'){
            current_instruction.append(1,'\n').append(line);
            exec_code=false;
        }else{
            need_next_exec=true;
        }
    }else{
        current_instruction=line;
    }
    try{
        std::stringstream ss;
        ss << current_code_fragment;
        boost::python::handle<> src(boost::python::allow_null(Py_CompileString(current_instruction.c_str(), ss.str().c_str(), Py_single_input)));
        CFinfo << "Py compile : " << current_instruction << CFendl;
        if (NULL != src && exec_code){//code compiled, we execute it
            boost::python::handle<> dum(boost::python::allow_null(PyEval_EvalCode((PyCodeObject *)src.get(), global_scope.get(), local_scope.get())));
            current_code_fragment++;
            current_instruction.clear();
        }
        PyObject *exc,*val,*trb,*obj;
        char* error;
        PyErr_Fetch(&exc, &val, &trb);
        if (NULL != exc && NULL != val){
            if (PyArg_ParseTuple (val, "sO", &error, &obj)){
                if (!strcmp (error, "unexpected EOF while parsing") || !strcmp(error,"expected an indented block")){
                    //raised when a multi-line command is not terminated, must wait the next lines
                    //CFinfo << "Python need next line" << CFendl;
                    if(need_next_exec)
                        current_instruction.clear();// to prevent infinite loop
                }else{
                    //CFinfo << "Python error : " << error << CFendl;
                    emit_output(std::string(error));
                    current_instruction.clear();
                }
            }else{
                //CFinfo << "Python unknown error" << CFendl;
                emit_output(std::string("Expression not found in the scope."));
                current_instruction.clear();
            }
        }
    } catch(...) {
        CFinfo << "Error while executing Python" << CFendl;
        current_instruction.clear();
    }
    PyErr_Clear();
    if (need_next_exec)
        execute_line(line);*/
}

bool ScriptEngine::check_scope_difference(PyObject* dict){
    PyObject *key, *value;
    char* key_str;
    Py_ssize_t pos = 0;
    while (PyDict_Next(dict, &pos, &key, &value)) {
        if (NULL != key && NULL != value){
            key_str=PyString_AsString(key);
            if (key_str[0] != '_'){
                bool found=false;
                for(int i=0;i<scope_diff.size();i++){
                    if (value == scope_diff[i]){
                        found=true;
                    }
                }
                if (!found)
                    return true;
            }
        }
    }
    return false;
}


////////////////////////////////////////////////////////////////////////////////////////////

void ScriptEngine::read_dictionary(PyObject* dict,std::vector<std::string> *word_list){
    PyObject *key, *value;
    char* key_str;
    Py_ssize_t pos = 0;
    scope_diff.clear();
    while (PyDict_Next(dict, &pos, &key, &value)) {
        if (NULL != key && NULL != value){
            key_str=PyString_AsString(key);
            if (key_str[0] != '_'){
                scope_diff.push_back(value);
                word_list->push_back(std::string(key_str));
            }
            if (strcmp(key_str,"__builtins__"))
                read_objects(value,std::string(key_str)+".",word_list);
            else//builtins members are avalaible without writing the __builtins__
                read_objects(value,"",word_list);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////

void ScriptEngine::read_objects(PyObject* obj,std::string obj_name,std::vector<std::string> *word_list){
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
                    if (PyCallable_Check(value.get()))
                        word_list->push_back(obj_name+key_str+"(");
                    else
                        word_list->push_back(obj_name+key_str);
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
                if (out.size()>0){
                    //CFinfo << "flush emit output" << CFendl;
                    emit_output(out);
                    execute_line("sys.stdout.data=''");
                }
                //execute_line("sys.stdout.data=''");
                //PyObject_SetAttrString(py_stdout,"data",PyString_FromString(""));
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////

void ScriptEngine::signal_execute_script(SignalArgs& node)
{
    SignalOptions options( node );
    std::string code=options.option("script").value<std::string>();
    std::replace(code.begin(),code.end(),';','\t');
    execute_script(code,options.option("fragment").value<int>());
    flush_python_stdout();
    if (check_scope_difference(local_scope.get())){
        emit_completion_list();
    }
    //return boost::python::object();
}

////////////////////////////////////////////////////////////////////////////////////////////

void ScriptEngine::signature_execute_script(SignalArgs& node)
{
    SignalOptions options( node );

    options.add_option( "script", std::string() )
            .description("Script to execute")
            .pretty_name("Script");
    options.add_option( "fragment", std::string() )
            .description("Code fragment number")
            .pretty_name("Code fragment");
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

    std::vector<std::string> word_list;
    read_dictionary(local_scope.get(),&word_list);
    /// @todo remove those hardcoded URIs
    m_manager=Core::instance().root().access_component_checked("//Tools/PEManager")->handle<PE::Manager>();
    SignalFrame frame("completion", uri(), "cpath:/UI/ScriptEngine");
    SignalOptions options(frame);
    options.add_option("words", word_list);
    options.flush();
    m_manager->send_to_parent( frame );
}


////////////////////////////////////////////////////////////////////////////////////////////

void ScriptEngine::signal_completion(SignalArgs& node){
    emit_completion_list();
}


} // python
} // cf3
