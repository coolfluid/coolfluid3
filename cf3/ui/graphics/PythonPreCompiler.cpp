// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/python.hpp>

#include "ui/graphics/PythonPreCompiler.hpp"

#include "ui/uicommon/ComponentNames.hpp"

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace graphics {

using namespace core;
using namespace common;

int PythonPreCompiler::python_init=0;

////////////////////////////////////////////////////////////////////////////

PythonPreCompiler::PythonPreCompiler()
  : CNode(CLIENT_PYTHON_PRE_COMPILER , "PythonPreCompiler", CNode::DEBUG_NODE)
{
  if (python_init++ == 0)
    Py_Initialize();
}

PythonPreCompiler::~PythonPreCompiler(){
  if (--python_init == 0)
    Py_Finalize();
}

const QString PythonPreCompiler::try_compile(const QString & line) const {
  boost::python::handle<> src=boost::python::handle<>(boost::python::allow_null(Py_CompileString(line.toStdString().c_str(), "test", Py_single_input)));
  if (NULL != src.get())
    return "";
  PyObject *exc,*val,*trb,*obj;
  char* error;
  PyErr_Fetch(&exc, &val, &trb);
  if (NULL != val){
    if (PyArg_ParseTuple (val, "sO", &error, &obj)){
      return error;
    }else{
      const char* val_str=PyString_AsString(PyObject_Str(val));
      return val_str;
    }
  }
  PyErr_Clear();
}


} // Graphics
} // ui
} // cf3
