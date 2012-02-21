
coolfluid_add_test( UTEST utest-solver-physics-static
                    CPP   utest-solver-physics-static.cpp
                    LIBS  coolfluid_solver )

coolfluid_add_test( UTEST utest-solver-physics-static2dynamic
                    CPP   utest-solver-physics-static2dynamic.cpp
                    LIBS  coolfluid_solver )

coolfluid_add_test( UTEST utest-solver-model
                    PYTHON utest-solver-model.py )

########################################################################
# action tests
add_subdirectory( actions )
