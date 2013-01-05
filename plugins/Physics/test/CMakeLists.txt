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

coolfluid_add_test( UTEST utest-physics-euler
                    CPP   utest-physics-euler.cpp
                    LIBS  coolfluid_physics_euler )

#########################################################################################

coolfluid_add_test( UTEST utest-physics-lineuler
                    CPP   utest-physics-lineuler.cpp
                    LIBS  coolfluid_physics_lineuler )

#########################################################################################

add_subdirectory( NavierStokes )

