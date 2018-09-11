if( CF3_HAVE_PYTHON )

    list( APPEND coolfluid_python_files
      BoostPython.hpp
      ComponentFilterPython.hpp
      ComponentFilterPython.cpp
      ComponentWrapper.hpp
      ComponentWrapper.cpp
      CoreWrapper.hpp
      CoreWrapper.cpp
      TableWrapper.hpp
      TableWrapper.cpp
      ListWrapper.hpp
      ListWrapper.cpp
      LibPython.hpp
      LibPython.cpp
      Math.hpp
      Math.cpp
      MatrixWrappers.hpp
      MatrixWrappers.cpp
      PythonAny.hpp
      PythonAny.cpp
      PythonModule.hpp
      PythonModule.cpp
      ScriptEngine.hpp
      ScriptEngine.cpp
      URIWrapper.hpp
      URIWrapper.cpp
      Utility.hpp
      NetworkXPython.hpp
      NetworkXPython.cpp
      TestAllOptions.hpp
      TestAllOptions.cpp
      TestSignals.hpp
      TestSignals.cpp
    )

    coolfluid3_add_library( TARGET  coolfluid_python
                            KERNEL
                            PYTHON_MODULE
                            SOURCES  ${coolfluid_python_files}
                            INCLUDES
                                ${PYTHON_INCLUDE_DIRS}
                            LIBS coolfluid_common coolfluid_mesh
                                ${Boost_PYTHON_LIBRARY}
                                ${PYTHON_LIBRARIES} )

    # Takes a lot of RAM to compile:
    set_source_files_properties(MatrixWrappers.cpp PROPERTIES COMPILE_FLAGS "-g0")

    configure_file( coolfluid.py      ${CF3_DSO_DIR} COPYONLY )
    configure_file( networkxpython.py ${CF3_DSO_DIR} COPYONLY )
    configure_file( check.py          ${CF3_DSO_DIR} COPYONLY )

endif()
