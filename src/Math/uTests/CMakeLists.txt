################################################################################

LIST ( APPEND uTestMath_cflibs Math )
LIST ( APPEND uTestMath_files
  uTestMath.cpp
  TestVectorOperations.cpp
  TestMatrixInverter.cpp )

CF_ADD_UNITTEST ( uTestMath )

################################################################################
