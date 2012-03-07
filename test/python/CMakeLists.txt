coolfluid_add_test( UTEST  utest-python-basics
                    PYTHON utest-python-basics.py )

coolfluid_add_test( UTEST      utest-python-scriptengine
                    CPP        utest-python-scriptengine.cpp
                    ARGUMENTS  ${CMAKE_CURRENT_SOURCE_DIR}/utest-python-basics.py
                    CONDITION  CF3_HAVE_PYTHON
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
