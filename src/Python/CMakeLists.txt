if( CF_ENABLE_PYTHON AND PYTHONLIBS_FOUND AND Boost_PYTHON_FOUND)

list( APPEND coolfluid_python_files
  Component.hpp
  Component.cpp
  Core.hpp
  Core.cpp
  LibPython.hpp
  LibPython.cpp
  PythonModule.hpp
  PythonModule.cpp
  URI.hpp
  URI.cpp
)

set( coolfluid_python_kernellib TRUE )

list(APPEND coolfluid_python_includedirs ${PYTHON_INCLUDE_DIRS})
list(APPEND coolfluid_python_libs ${Boost_PYTHON_LIBRARY} ${PYTHON_LIBRARIES})
list(APPEND coolfluid_python_cflibs coolfluid_common )

coolfluid_add_library( coolfluid_python )

configure_file(coolfluid.py ${coolfluid_DSO_DIR} COPYONLY)

endif()
