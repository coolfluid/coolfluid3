
coolfluid_add_test( UTEST utest-function-parser
                    CPP   utest-function-parser.cpp
                    LIBS  coolfluid_math )


coolfluid_add_test( UTEST utest-vector-operations
                    CPP   utest-vector-operations.cpp
                    LIBS  coolfluid_math )


coolfluid_add_test( UTEST utest-math-checks
                    CPP   utest-math-checks.cpp
                    LIBS  coolfluid_math )


coolfluid_add_test( UTEST utest-matrix-inverter
                    CPP   utest-matrix-inverter.cpp
                    LIBS  coolfluid_math )

# performance test
add_definitions( -DNDEBUG -DEIGEN_NO_DEBUG )
coolfluid_add_test( PTEST ptest-eigen-vs-matrixt
                    CPP   ptest-eigen-vs-matrixt.cpp
                    LIBS  coolfluid_math )


coolfluid_add_test( UTEST utest-math-variablesdescriptor
                    CPP   utest-math-variablesdescriptor.cpp
                    LIBS  coolfluid_math )


coolfluid_add_test( UTEST utest-math-variablemanager
                    CPP   utest-math-variablemanager.cpp
                    LIBS  coolfluid_math )

coolfluid_add_test( UTEST utest-math-integrate
                    CPP   utest-math-integrate.cpp
                    LIBS  coolfluid_math )

coolfluid_add_test( UTEST utest-math-hilbert
                    CPP   utest-math-hilbert.cpp
                    LIBS  coolfluid_math )

################################################################################


coolfluid_add_test( UTEST utest-lss-system-emptylss
                    CPP   utest-lss-system-emptylss.cpp
                    LIBS  coolfluid_math_lss coolfluid_math
                    MPI   1 )
if(CF3_HAVE_TRILINOS)
coolfluid_add_test( UTEST utest-lss-atomic-fevbr
                    CPP   utest-lss-atomic.cpp
                    LIBS  coolfluid_math_lss coolfluid_math
                    ARGUMENTS cf3.math.LSS.TrilinosFEVbrMatrix
                    MPI   2)

add_test(utest-lss-atomic-crs ${CF3_MPIRUN_PROGRAM} -np 2 utest-lss-atomic-fevbr cf3.math.LSS.TrilinosCrsMatrix)

coolfluid_add_test( UTEST utest-lss-distributed-matrix-fevbr
                    CPP   utest-lss-distributed-matrix.cpp utest-lss-test-matrix.hpp
                    LIBS  coolfluid_math_lss coolfluid_math
                    ARGUMENTS cf3.math.LSS.TrilinosFEVbrMatrix
                    MPI   4)

add_test(utest-lss-distributed-matrix-crs ${CF3_MPIRUN_PROGRAM} -np 4 utest-lss-distributed-matrix-fevbr cf3.math.LSS.TrilinosCrsMatrix)

coolfluid_add_test( UTEST utest-lss-symmetric-dirichlet-crs
                    CPP   utest-lss-symmetric-dirichlet.cpp
                    LIBS  coolfluid_math_lss coolfluid_math
                    ARGUMENTS cf3.math.LSS.TrilinosCrsMatrix
                    MPI   2)

add_test(utest-lss-symmetric-dirichlet-fevbr ${CF3_MPIRUN_PROGRAM} -np 2 utest-lss-symmetric-dirichlet-crs cf3.math.LSS.TrilinosFEVbrMatrix)

else()
coolfluid_mark_not_orphan(utest-lss-atomic.cpp utest-lss-distributed-matrix.cpp utest-lss-symmetric-dirichlet.cpp utest-lss-test-matrix.hpp)
endif()

coolfluid_add_test( UTEST utest-lss-solvelss
                    CPP   utest-lss-solvelss.cpp
                    LIBS  coolfluid_math_lss coolfluid_math
                    MPI   1 )

################################################################################

#if( CMAKE_COMPILER_IS_GNUCC )
#    set_source_files_properties( utest-eigen-vs-matrixt.cpp
#      PROPERTIES COMPILE_FLAGS "-O2 -msse2 -msse3 -mssse3 -march=core2" )
#endif()
