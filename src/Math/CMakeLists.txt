coolfluid_find_orphan_files()

list( APPEND coolfluid_math_files
 LibMath.cpp
 LibMath.hpp
 BoostMath.hpp
 Defs.hpp
 FindMinimum.hpp
 MathConsts.hpp
 MathFunctions.hpp
 MatrixTypes.hpp
 MathChecks.hpp
 MathExceptions.hpp
 MathExceptions.cpp
 VectorialFunction.hpp
 VectorialFunction.cpp
)

list( APPEND coolfluid_math_cflibs coolfluid_fparser coolfluid_common )

set( coolfluid_math_kernellib TRUE )

coolfluid_add_library( coolfluid_math )

