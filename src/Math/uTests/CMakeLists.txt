################################################################################

LIST ( APPEND uTestMath_libs Math )
LIST ( APPEND uTestMath_files
  uTestMath.cpp
  TestVectorOperations.cpp
  TestMatrixInverter.cpp )

CF_ADD_UNITTEST ( uTestMath )

################################################################################
