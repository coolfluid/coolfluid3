# TQ: I leave the sources ere but deactivate this physical model
#     as it follows the old API. Willem will upgrade to new one.

#########################################################################################
# utest-advectiondiffusion

coolfluid_add_test( UTEST utest-advectiondiffusion
                    CPP   utest-advectiondiffusion.cpp
                    LIBS  coolfluid_advectiondiffusion coolfluid_solver
                    CONDITION FALSE)

#########################################################################################
# utest-euler

coolfluid_add_test( UTEST utest-euler
                    CPP   utest-euler.cpp
                    LIBS  coolfluid_euler coolfluid_solver
                    CONDITION FALSE)

#########################################################################################

add_subdirectory( NavierStokes )

