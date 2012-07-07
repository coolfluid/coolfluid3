coolfluid_add_test( UTEST  utest-python-basics
                    PYTHON utest-python-basics.py )

LIST( APPEND utest-python-scriptengine_includedirs ${PYTHON_INCLUDE_DIR} )
coolfluid_add_test( UTEST      utest-python-scriptengine
                    CPP        utest-python-scriptengine.cpp
                    ARGUMENTS  ${CMAKE_CURRENT_SOURCE_DIR}/utest-python-basics.py
                    CONDITION  CF3_ENABLE_PYTHON AND PYTHONLIBS_FOUND AND Boost_PYTHON_FOUND
                    LIBS       coolfluid_python )

coolfluid_add_test( UTEST      utest-python-runscript
                    PYTHON     utest-python-runscript.py
                    ARGUMENTS  ${CMAKE_CURRENT_SOURCE_DIR}/utest-python-basics.py)

coolfluid_add_test( UTEST  utest-python-table
                    PYTHON utest-python-table.py )

coolfluid_add_test( UTEST  utest-python-properties
                    PYTHON utest-python-properties.py )

coolfluid_add_test( UTEST  utest-python-signal-return
                    PYTHON utest-python-signal-return.py )

coolfluid_add_test( UTEST  utest-python-matrix
                    PYTHON utest-python-matrix.py )

coolfluid_add_test( UTEST  utest-python-options
                    PYTHON utest-python-options.py )

coolfluid_add_test( UTEST  utest-python-component
                    PYTHON utest-python-component.py )
