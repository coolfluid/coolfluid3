################################################################################

list( APPEND utest-function-parser_cflibs coolfluid_math )
list( APPEND utest-function-parser_files  utest-function-parser.cpp )

coolfluid_add_unit_test( utest-function-parser )

################################################################################

list( APPEND utest-vector-operations_cflibs coolfluid_math )
list( APPEND utest-vector-operations_files  utest-vector-operations.cpp )

coolfluid_add_unit_test( utest-vector-operations )

################################################################################

list( APPEND utest-math-checks_cflibs coolfluid_math )
list( APPEND utest-math-checks_files  utest-math-checks.cpp )

coolfluid_add_unit_test( utest-math-checks )

################################################################################

list( APPEND utest-matrix-inverter_cflibs coolfluid_math )
list( APPEND utest-matrix-inverter_files  utest-matrix-inverter.cpp )

coolfluid_add_unit_test( utest-matrix-inverter )

################################################################################

list( APPEND utest-eigen-vs-matrixt_cflibs coolfluid_math )
list( APPEND utest-eigen-vs-matrixt_files  utest-eigen-vs-matrixt.cpp )

set( utest-eigen-vs-matrixt_performance_test TRUE )

add_definitions( -DNDEBUG -DEIGEN_NO_DEBUG )
coolfluid_add_unit_test( utest-eigen-vs-matrixt )

################################################################################

list( APPEND utest-math-variablesdescriptor_cflibs coolfluid_physics )
list( APPEND utest-math-variablesdescriptor_files  utest-math-variablesdescriptor.cpp )

coolfluid_add_unit_test( utest-math-variablesdescriptor )

#if( CMAKE_COMPILER_IS_GNUCC )
#    set_source_files_properties( utest-eigen-vs-matrixt.cpp
#      PROPERTIES COMPILE_FLAGS "-O2 -msse2 -msse3 -mssse3 -march=core2" )
#endif()
