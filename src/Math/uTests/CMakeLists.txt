################################################################################

list( APPEND utest-math_cflibs Math )
list( APPEND utest-math_files
  utest-math.cpp
  TestFunctionParser.cpp
  TestVectorOperations.cpp
  TestMatrixInverter.cpp
)

coolfluid_add_unittest( utest-math )

################################################################################

list( APPEND utest-eigen-vs-matrixt_cflibs Math )
list( APPEND utest-eigen-vs-matrixt_files
  utest-eigen-vs-matrixt.cpp
)

add_definitions( -DNDEBUG -DEIGEN_NO_DEBUG )
coolfluid_add_unittest( utest-eigen-vs-matrixt )

if( CMAKE_COMPILER_IS_GNUCC )
    set_source_files_properties( utest-eigen-vs-matrixt.cpp
      PROPERTIES COMPILE_FLAGS "-O2 -msse2 -msse3 -mssse3 -march=core2" )
endif()
