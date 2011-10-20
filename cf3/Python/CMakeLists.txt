if( CF3_ENABLE_PYTHON AND PYTHONLIBS_FOUND AND Boost_PYTHON_FOUND)

list( APPEND coolfluid_python_files
  Component.hpp
  Component.cpp
  Core.hpp
  Core.cpp
  CTable.hpp
  CTable.cpp
  LibPython.hpp
  LibPython.cpp
  MatrixTypes.hpp
  MatrixTypes.cpp
  PythonModule.hpp
  PythonModule.cpp
  ScriptEngine.hpp
  ScriptEngine.cpp
  URI.hpp
  URI.cpp
  Utility.hpp
)

set( coolfluid_python_kernellib TRUE )
set( coolfluid_python_PYTHON_MODULE TRUE )
list(APPEND coolfluid_python_includedirs ${PYTHON_INCLUDE_DIRS})
list(APPEND coolfluid_python_libs ${Boost_PYTHON_LIBRARY} ${PYTHON_LIBRARIES})
list(APPEND coolfluid_python_cflibs coolfluid_common coolfluid_mesh )

coolfluid_add_library( coolfluid_python )

configure_file(coolfluid.py ${coolfluid_DSO_DIR} COPYONLY)

endif()
