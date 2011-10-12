
coolfluid_add_python_test(NAME utest-python-basics
                          SCRIPT utest-python-basics.py
                          UTEST)

list( APPEND utest-python-scriptengine_cflibs coolfluid_python)
list( APPEND utest-python-scriptengine_files
  utest-python-scriptengine.cpp )
list( APPEND utest-python-scriptengine_args ${CMAKE_CURRENT_SOURCE_DIR}/utest-python-basics.py )
coolfluid_add_unit_test( utest-python-scriptengine )

coolfluid_add_python_test(NAME utest-python-runscript
                          SCRIPT utest-python-runscript.py
                          UTEST
                          ARGUMENTS ${CMAKE_CURRENT_SOURCE_DIR}/utest-python-basics.py)