################################################################################

list( APPEND uTestMath_cflibs Math )
list( APPEND uTestMath_files
  uTestMath.cpp
  TestFunctionParser.cpp
  TestVectorOperations.cpp
  TestMatrixInverter.cpp
)

coolfluid_add_unittest( uTestMath )

################################################################################
