################################################################################

LIST ( APPEND uTestMath_libs Math )
LIST ( APPEND uTestMath_files
  UTestMath.cpp 
  TestMatrixInverter.cpp )

CF_ADD_UNITTEST ( uTestMath )

################################################################################
