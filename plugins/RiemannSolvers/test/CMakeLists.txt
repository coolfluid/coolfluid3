
coolfluid_add_test( UTEST   utest-riemannsolver
                    CPP     utest-riemannsolver.cpp
                    PLUGINS Physics
                    LIBS    coolfluid_riemannsolvers coolfluid_physics_navierstokes coolfluid_physics_scalar coolfluid_physics_lineuler )
