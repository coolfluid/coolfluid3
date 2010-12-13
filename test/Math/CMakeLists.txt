################################################################################

list( APPEND utest-function-parser_cflibs coolfluid_math )
list( APPEND utest-function-parser_files  utest-function-parser.cpp )

coolfluid_add_unittest( utest-function-parser )

################################################################################

list( APPEND utest-vector-operations_cflibs coolfluid_math )
list( APPEND utest-vector-operations_files  utest-vector-operations.cpp )

coolfluid_add_unittest( utest-vector-operations )

################################################################################

list( APPEND utest-matrix-inverter_cflibs coolfluid_math )
list( APPEND utest-matrix-inverter_files  utest-matrix-inverter.cpp )

coolfluid_add_unittest( utest-matrix-inverter )

################################################################################

list( APPEND utest-eigen-vs-matrixt_cflibs coolfluid_math )
list( APPEND utest-eigen-vs-matrixt_files  utest-eigen-vs-matrixt.cpp )

add_definitions( -DNDEBUG -DEIGEN_NO_DEBUG )
coolfluid_add_unittest( utest-eigen-vs-matrixt )


#if( CMAKE_COMPILER_IS_GNUCC )
#    set_source_files_properties( utest-eigen-vs-matrixt.cpp
#      PROPERTIES COMPILE_FLAGS "-O2 -msse2 -msse3 -mssse3 -march=core2" )
#endif()
