if( CF_ENABLE_PYTHON AND PYTHONLIBS_FOUND AND Boost_PYTHON_FOUND AND CF_ENABLE_UNIT_TESTS)

add_test("utest-python-basics" ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/utest-python-basics.py)
set_tests_properties(utest-python-basics PROPERTIES ENVIRONMENT "PYTHONPATH=${coolfluid_BINARY_DIR}/dso")

endif()
