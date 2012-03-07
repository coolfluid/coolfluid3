if( CF3_HAVE_PYTHON )

list( APPEND coolfluid_python_files
  BoostPython.hpp
  ComponentWrapper.hpp
  ComponentWrapper.cpp
  CoreWrapper.hpp
  CoreWrapper.cpp
  TableWrapper.hpp
  TableWrapper.cpp
  LibPython.hpp
  LibPython.cpp
  MatrixWrappers.hpp
  MatrixWrappers.cpp
  PythonModule.hpp
  PythonModule.cpp
  ScriptEngine.hpp
  ScriptEngine.cpp
  URIWrapper.hpp
  URIWrapper.cpp
  Utility.hpp
  NetworkXPython.hpp
  NetworkXPython.cpp
)

set( coolfluid_python_kernellib TRUE )
set( coolfluid_python_PYTHON_MODULE TRUE )
list(APPEND coolfluid_python_includedirs ${PYTHON_INCLUDE_DIRS})
list(APPEND coolfluid_python_libs ${Boost_PYTHON_LIBRARY} ${PYTHON_LIBRARIES})
list(APPEND coolfluid_python_cflibs coolfluid_common coolfluid_mesh )

coolfluid_add_library( coolfluid_python )

configure_file(coolfluid.py ${coolfluid_DSO_DIR} COPYONLY)
configure_file(networkxpython.py ${coolfluid_DSO_DIR} COPYONLY)

endif()
